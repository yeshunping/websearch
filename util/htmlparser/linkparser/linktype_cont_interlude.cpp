/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_cont_interlude.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_cont_interlude.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 
 **/

#include "easou_link.h"
#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "PageType.h"
#include "log.h"

#include <ctype.h>

#define CHECK_NORMAL	0
#define CHECK_STRICT	1
#define MAX_GROUP_COUNT 200		//内容组的最多个数
#define DEFAULT_MAX_VNODE_COUNT	10000	//默认的每个数最多的结点个数
#define MAX_END_TAG_CODE	10000	//网页结点的最大tag_code
#define DEFAULT_VNODE_SET_SIZE 5	//每个内容组默认的最大结点个数
/**
 * @brief short description 内容组
 */
typedef struct _text_group_t
{
	int route; //内容组的路径深度
	int pure_text_len; //该内容组的文字长度
	html_vnode_t **vnode_set; //该内容组中的结点集合
	int vnode_count; //该内容组中结点个数
	int vnode_set_size; //该内容组中结点集合的最大容量
} text_group_t;

/**
 * @brief short description 链接穿插信息结构
 */
typedef struct _link_interlude_info_t
{
	int right_in_line; //链接与右结点是否在同一行
	int left_in_line; //链接与左结点是否在同一行
	int right_in_range; //链接与右结点之间的结点个数是否在一定阈值内
	int left_in_range; //链接与左内容结点之间的结点个数是否在一定阈值内
	int right_is_mark_character; //链接与右内容结点之间相邻的不是有意义字符
	int left_is_mark_character; //链接与左内容结点之间相邻的不是有意义字符
	int right_distance; //链接与右内容节点之间的坐标距离是否在一定阈值内
	int left_distance; //链接与左内容结点之间的坐标距离是否在一定阈值内
	int right_has_td; //链接与右内容节点之间是否有td标签
	int left_has_td; //链接与做内容结点之间是否有td标签
	int total_wx; //链接和左右内容节点的宽度总和
} link_interlude_info_t;

/**
 * @brief short description 保存全局信息
 */
typedef struct _tree_visit_t
{
	int route; //当前路径
	text_group_t text_group_set[MAX_GROUP_COUNT]; //内容组集合
	int text_group_count; //内容组的个数
	int re_end_tag_code; //网页页脚开始的结点序号
	cont_interlude_res_t *resource; //外部传入的预处理资源
} tree_visit_t;

static const char *END_MARK[] =
{ //网页页脚开始标志
		"powered by", "power by", "copyright" };

//创建包， NULL -> 创建失败， NOT NULL -》 成功
cont_interlude_res_t *cont_interlude_res_create()
{
	const char *where = "cont_interlude_res_create()";
	cont_interlude_res_t *pres = NULL;

	pres = (cont_interlude_res_t*) calloc(1, sizeof(cont_interlude_res_t));
	if (pres == NULL)
	{
		Error("calloc cont_interlude_res_t error. %s", where);
		cont_interlude_res_del(pres);
		return NULL;
	}
	pres->vnode_set = (html_vnode_t **) calloc(DEFAULT_MAX_VNODE_COUNT, sizeof(html_vnode_t *));
	if (pres->vnode_set == NULL)
	{
		Error("calloc pres->vnode_set error. %s", where);
		cont_interlude_res_del(pres);
		return NULL;
	}
	pres->vnode_set_size = DEFAULT_MAX_VNODE_COUNT;
	pres->vnode_set_used = 0;
	return pres;
}

//refresh 资源
void cont_interlude_res_reset(cont_interlude_res_t *pres)
{
	pres->vnode_set_used = 0;
}

//删除资源
void cont_interlude_res_del(cont_interlude_res_t *pres)
{
	if (pres)
	{
		if (pres->vnode_set)
		{
			free(pres->vnode_set);
			pres->vnode_set = NULL;
		}
		free(pres);
		pres = NULL;
	}
	return;
}

//从数组中找寻特征字符串
static int strstr_array(char *text, const char *strings[], const int count)
{
	for (int i = 0; i < count; i++)
	{
		if (strstr(text, strings[i]) != NULL)
			return 1;
	}
	return 0;
}

/**
 * @brief 根据路径深度，获得相应的内容组
 * @param [in] route   : int	路径深度
 * @param [in] ptgroup   : text_group_t*	内容组集合
 * @param [in] text_group_count   : int	内容组个数
 * @return  text_group_t* 
 * @retval   	NOT NULL	路径对应的内容组
 * @retval	NULL		当前路径对应空的内容组
 **/
static text_group_t *get_text_group(int route, text_group_t *ptgroup, int text_group_count)
{
	text_group_t *ret_group = NULL;
	for (int i = 0; i < text_group_count; i++)
	{
		if (route == ptgroup[i].route)
		{
			ret_group = ptgroup + i;
			break;
		}
	}
	return ret_group;
}

/**
 * @brief 前序遍历树信息
 * @param [in] vnode   : html_vnode_t*	树节点
 * @param [in/out] result   : void*	全局数据结构
 * @return  int 
 * @retval   	VISIT_SKIP_CHILD	不访问子节点
 * @retval	VISIT_NORMAL		访问结点结束
 * @retval	VISIT_ERROR		访问结点出错
 * @retval	VISIT_FINISH		访问树结束
 **/
static int start_visit_for_tree(html_vnode_t *vnode, void *result)
{
	tree_visit_t *pdata = (tree_visit_t *) result;
	cont_interlude_res_t *pres = pdata->resource;

	html_tag_t *ptag = &vnode->hpNode->html_tag;

	if (ptag->tag_type == TAG_SCRIPT || ptag->tag_type == TAG_HEAD || ptag->tag_type == TAG_A)
	{
		return VISIT_SKIP_CHILD;
	}

	//更新路径
	pdata->route += ptag->tag_type;

	if (ptag->tag_type != TAG_PURETEXT)
		return VISIT_NORMAL;

	int text_size = vnode->textSize > 0 ? vnode->textSize : 0;

	if (text_size <= 1)
		return VISIT_NORMAL;

	text_group_t *ptgroup = NULL;
	ptgroup = get_text_group(pdata->route, pdata->text_group_set, pdata->text_group_count);
	if (ptgroup == NULL)
	{
		if (pdata->text_group_count == MAX_GROUP_COUNT)
			return VISIT_FINISH;

		ptgroup = &pdata->text_group_set[pdata->text_group_count++];
		ptgroup->pure_text_len = 0;
		ptgroup->route = pdata->route;
		ptgroup->vnode_count = 0;

		ptgroup->vnode_set_size = DEFAULT_VNODE_SET_SIZE;
		if (ptgroup->vnode_set_size + pres->vnode_set_used > pres->vnode_set_size)
		{
			return VISIT_FINISH;
		}
		ptgroup->vnode_set = pres->vnode_set + pres->vnode_set_used;
		pres->vnode_set_used += ptgroup->vnode_set_size;
	}

	if (ptgroup->vnode_count == ptgroup->vnode_set_size) //realloc vnode set from resources
	{
		ptgroup->vnode_set_size *= 2;
		if (ptgroup->vnode_set_size + pres->vnode_set_used > pres->vnode_set_size)
		{
			return VISIT_FINISH;
		}
		html_vnode_t **prev_vnode_set = ptgroup->vnode_set;
		ptgroup->vnode_set = pres->vnode_set + pres->vnode_set_used;
		for (int i = 0; i < ptgroup->vnode_count; i++)
		{
			ptgroup->vnode_set[i] = prev_vnode_set[i];
		}
		pres->vnode_set_used += ptgroup->vnode_set_size;
	}
	assert(ptgroup->vnode_count < ptgroup->vnode_set_size);

	ptgroup->vnode_set[ptgroup->vnode_count++] = vnode;
	ptgroup->pure_text_len += text_size;

	if (text_size < 150 && strstr_array(ptag->text, END_MARK, sizeof(END_MARK) / sizeof(char *)))
	{
		pdata->re_end_tag_code = ptag->tag_code;
	}
	return VISIT_NORMAL;
}

//后序遍历树节点
static int finish_visit_for_tree(html_vnode_t *vnode, void *result)
{
	tree_visit_t *pdata = (tree_visit_t*) result;
	html_tag_t *ptag = &vnode->hpNode->html_tag;

	pdata->route -= ptag->tag_type;

	return VISIT_NORMAL;
}

/**
 * @brief 从树上获得全局统计信息
 * @param [in] root   : html_vnode_t*	root结点
 * @param [in/out] result   : void*	cont_interlude全局结构
 * @return  int 
 * @retval   	VISIT_ERROR	访问出错
 * @retval	VISIT_NORMAL	访问正常结束
 **/
static int get_feature4ci_from_tree(html_vnode_t *root, void *result)
{
	int ret;
	ret = html_vnode_visit(root, start_visit_for_tree, finish_visit_for_tree, result);

	if (ret == VISIT_ERROR)
		return VISIT_ERROR;
	return VISIT_FINISH;
}

/**
 * @brief 获得文章正文内容层次
 * @param [in/out] pdata   : tree_visit_t*	全局数据
 * @return  text_group_t* 
 * @retval   	NOT NULL	获得内容组成功
 * @retval	NULL		获取失败
 **/
static text_group_t *get_content_group(tree_visit_t *pdata)
{
	text_group_t *ptgroup = NULL;
	text_group_t *ptgroup_best = NULL;
	for (int i = 0; i < pdata->text_group_count; i++)
	{
		ptgroup = &pdata->text_group_set[i];
		if (ptgroup_best == NULL)
		{
			ptgroup_best = ptgroup;
			continue;
		}
		if (ptgroup->pure_text_len > ptgroup_best->pure_text_len)
		{
			ptgroup_best = ptgroup;
		}
	}
	if (ptgroup_best != NULL && ptgroup_best->pure_text_len < 80)
	{
		return NULL;
	}
	return ptgroup_best;
}

/**
 * @brief 判断一个链接是否适合作为内容穿插链接
 *
 * @param [in] pvlink   : vlink_t*	链接
 * @return  int 
 * @retval   	0	不适合作为内容穿插链接
 * @retval	1	适合作为内容穿插链接
 **/
static int is_cont_interlude_url(vlink_t *pvlink)
{
	if (!pvlink->inner.is_goodlink)
		return 0;

	if (is_url_has_protocol_head(pvlink->text) || strstr(pvlink->text, "www") != NULL)
		return 0;

	if (strlen(pvlink->text) >= 40)
		return 0;

	return 1;
}

/**
 * @brief 判断链接和结点是否在同一行
 *
 * @param [in] pvlink   : vlink_t*	链接
 * @param [in] pvnode   : html_vnode_t*	结点
 * @param [in] is_right_node : int	该节点是否在链接的右边
 * @return  int 
 * @retval   	0	不再同一行
 * @retval	1	在同一行
 **/
static int is_in_line(vlink_t *pvlink, html_vnode_t *pvnode, int is_right_node)
{
	if (pvnode->ypos == pvlink->inner.ypos)
		return 1;

	if (is_right_node)
	{
		return 0;
	}

	int limit = pvnode->hx;
	if (limit > 30)
		limit = 30;

	if (abs(pvnode->ypos - pvlink->inner.ypos) <= limit && (IS_LINE_BREAK(pvnode->property) || pvnode->hx > 15))
	{
		return 1;
	}
	return 0;
}

/**
 * @brief 判断俩节点之间是否有td结点间隔
 *
 * @param [in] pvnode_front   : html_vnode_t*	前节点
 * @param [in] pvnode_end   : html_vnode_t*		后结点
 * @return  int 
 * @retval   	0	没有td
 * @retval	1	有td间隔
 **/
static int has_td_between(html_vnode_t *pvnode_front, html_vnode_t *pvnode_end)
{
	if (pvnode_front->nextNode == NULL && pvnode_front->upperNode != NULL
			&& pvnode_front->upperNode->hpNode->html_tag.tag_type == TAG_TD)
		return 1;
	if (pvnode_end->prevNode == NULL && pvnode_end->upperNode != NULL
			&& pvnode_end->upperNode->hpNode->html_tag.tag_type == TAG_TD)
		return 1;

	return 0;
}

/**
 * @brief 判断左右结点是否在坐标位置上接近
 *
 * @param [in] left_node   : html_vnode_t*	左结点
 * @param [in] right_node   : html_vnode_t*	右结点
 * @return  int 
 * @retval   	0	不接近
 * @retval	1	接近
 **/
static int is_in_distance(html_vnode_t *left_node, html_vnode_t *right_node)
{
	if (right_node->xpos > left_node->xpos)
	{
		if (right_node->xpos - left_node->xpos - left_node->wx < 4)
			return 1;
	}
	else
	{
		if (right_node->wx > 400 || (right_node->hx >= 30 && right_node->wx > 150))
			return 1;
	}
	return 0;
}

//1 : is omit, 0 : not omit
static int is_tag_code_distance_omit_node(html_vnode_t* vnode)
{
	html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
	if (tag_type == TAG_FONT || tag_type == TAG_U || tag_type == TAG_H1 || tag_type == TAG_H2 || tag_type == TAG_H3
			|| tag_type == TAG_H4 || tag_type == TAG_H5 || tag_type == TAG_H6 || tag_type == TAG_B)
	{
		return 1;
	}
	return 0;
}

/**
 * @brief 得到两个节点之间间隔的结点数量
 *
 * @param [in] pvnode_front   : html_vnode_t*	前节点
 * @param [in] pvnode_end   : html_vnode_t*		后节点
 * @return  int 
 * @retval   	>=0	间隔结点数量
 **/
static int get_tag_code_distance(html_vnode_t *pvnode_front, html_vnode_t *pvnode_end)
{
	int init = get_vnode_tag_code(pvnode_end) - get_vnode_tag_code(pvnode_front);
	while (pvnode_front != NULL && pvnode_front->nextNode == NULL
			&& is_tag_code_distance_omit_node(pvnode_front->upperNode))
	{
		pvnode_front = pvnode_front->upperNode;
		init--;
	}
	while (pvnode_end != NULL && pvnode_end->prevNode == NULL && is_tag_code_distance_omit_node(pvnode_end->upperNode))
	{
		pvnode_end = pvnode_end->upperNode;
		init--;
	}
	return init;
}

/**
 * @brief 检查字符串后端是否为中文字或者是字母和数字, 为了效率，并不准确
 *
 * @param [in/out] text   : const char*
 * @return  int 
 **/
static int last_is_chword_or_alnum(const char *text)
{
	int len = strlen(text);
	if (len == 0)
		return 0;

	if (len >= 2)
	{
		if (IS_GBK(text + len - 2)) //最后一个为中文字符
		{
			if (IS_GB_CODE(text + len - 2))
				return 1;

			if (len >= 3 && IS_GBK(text + len - 3))
			{
				if (isalnum(*(text + len - 1)))
					return 1;
				return 0;
			}
		}
		else if (isalnum(*(text + len - 1)))
		{
			return 1;
		}
	}
	else if (isalnum(*text))
	{
		return 1;
	}
	return 0;
}

/**
 * @brief 获得链接在内容中的穿插信息
 *
 * @param [in] pvlink   : vlink_t*	当前链接
 * @param [in] left_node   : html_vnode_t*	链接的左边内容结点
 * @param [in] right_node   : html_vnode_t*	链接的右边内容结点
 * @param [in/out] pitl_info   : link_interlude_info_t*	穿插信息
 * @return  void 
 **/
static void get_link_interlude_info(vlink_t *pvlink, html_vnode_t *left_node, html_vnode_t *right_node,
		link_interlude_info_t *pitl_info)
{
	memset(pitl_info, 0, sizeof(link_interlude_info_t));
	pitl_info->right_is_mark_character = 1;
	pitl_info->left_is_mark_character = 1;

	if (right_node)
	{
		if (is_in_line(pvlink, right_node, 1))
			pitl_info->right_in_line = 1;
		if (get_tag_code_distance(pvlink->inner.vnode, right_node) < 4)
			pitl_info->right_in_range = 1;

		char *text = right_node->hpNode->html_tag.text;
		if (text != NULL)
		{
			if (isalnum(*text) || IS_GB_CODE(text))
				pitl_info->right_is_mark_character = 0;
		}
		if (is_in_distance(pvlink->inner.vnode, right_node))
			pitl_info->right_distance = 1;
		if (has_td_between(pvlink->inner.vnode, right_node))
			pitl_info->right_has_td = 1;
	}
	if (left_node)
	{

		if (is_in_distance(left_node, pvlink->inner.vnode))
			pitl_info->left_distance = 1;
		if (has_td_between(left_node, pvlink->inner.vnode))
			pitl_info->left_has_td = 1;

		char *text = left_node->hpNode->html_tag.text;
		if (text && last_is_chword_or_alnum(text))
		{
			pitl_info->left_is_mark_character = 0;
		}

		if (get_tag_code_distance(left_node, pvlink->inner.vnode) < 4)
			pitl_info->left_in_range = 1;

		if (is_in_line(pvlink, left_node, 0))
			pitl_info->left_in_line = 1;
	}
	pitl_info->total_wx = pvlink->inner.width;

	if (pitl_info->right_in_line)
	{
		pitl_info->total_wx += right_node->wx;
	}
	if (pitl_info->left_in_line)
	{
		pitl_info->total_wx += left_node->wx;
	}
	if (!pitl_info->right_has_td && pitl_info->right_in_range && pitl_info->right_distance)
		pitl_info->right_in_line = 1;
	if (!pitl_info->left_has_td && pitl_info->left_in_range && pitl_info->left_distance)
		pitl_info->left_in_line = 1;
	return;
}

/**
 * @brief 检查该链接是否是穿插链接
 *
 * @param [in] pitl_info   : link_interlude_info_t* 链接的穿插信息
 * @param [in] pfgroup   : text_group_t*	该内容的结点集合
 * @param [in] flag	 : 控制粒度
 * @return  int 
 * @retval   	1	是穿插链接
 * @retval	0	不是穿插链接
 **/
static int check_cont_interlude_inner(link_interlude_info_t *pitl_info, text_group_t *pfgroup, int flag)
{
	int total_wx_limit = 0;
	if (pfgroup->pure_text_len > 2500)
		total_wx_limit = 0;
	else if (pfgroup->pure_text_len > 1500)
		total_wx_limit = 400;
	else if (pfgroup->pure_text_len > 750)
		total_wx_limit = 600;
	else
		total_wx_limit = 800;

	if (pitl_info->right_in_line && pitl_info->left_in_line)
	{
		if (pitl_info->right_in_range || pitl_info->left_in_range)
		{
			if (!pitl_info->right_in_range)
			{
				if (pitl_info->left_distance && !pitl_info->left_has_td)
				{
					if (pitl_info->left_is_mark_character == 0 || pitl_info->total_wx > total_wx_limit)
						return 1;
					return 0;
				}
				else
					return 0;
			}
			else if (!pitl_info->left_in_range)
			{
				if (pitl_info->right_distance && !pitl_info->right_has_td)
				{
					if (pitl_info->right_is_mark_character == 0 || pitl_info->total_wx > total_wx_limit)
						return 1;
					return 0;
				}
				else
					return 0;
			}
			else if (!pitl_info->right_has_td || !pitl_info->left_has_td)
			{
				if (flag == CHECK_STRICT)
				{
					if (pitl_info->left_is_mark_character == 0 || pitl_info->right_is_mark_character == 0)
						return 1;
					return 0;
				}
				else
					return 1;
			}
			else
				return 0;
		}
		else
			return 0;
	}
	else if (pitl_info->right_in_line)
	{
		if (pitl_info->right_in_range && !pitl_info->right_is_mark_character && pitl_info->total_wx > total_wx_limit
				&& pitl_info->right_distance && !pitl_info->right_has_td)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if (pitl_info->left_in_line)
	{
		if (pitl_info->left_in_range && !pitl_info->left_is_mark_character && pitl_info->total_wx > total_wx_limit
				&& pitl_info->left_distance && !pitl_info->left_has_td)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

/**
 * @brief 检查该链接是否为穿插链接
 *
 * @param [in] pvlink   : vlink_t*	待检查的链接
 * @param [in] pfgroup   : text_group_t*	内容结点集合
 * @param [in] footer_tag_code   : int	网页页脚开始结点序号
 * @param [in] height   : int	网页的高度
 * @param [in] flag 	: int	控制检测粒度
 * @return  int 
 * @retval   	0	不是内容穿插链接
 * @retavl	1	是内容穿插链接
 **/
static int check_cont_interlude(vlink_t *pvlink, text_group_t *pfgroup, int footer_tag_code, int height, int flag)
{
	int tag_code = pvlink->inner.node->html_tag.tag_code;

	if (tag_code >= footer_tag_code || tag_code < get_vnode_tag_code(pfgroup->vnode_set[0]))
		return 0;

	int i = 0;
	for (i = 0; i < pfgroup->vnode_count - 1; i++)
	{
		if (tag_code > get_vnode_tag_code(pfgroup->vnode_set[i])
				&& tag_code < get_vnode_tag_code(pfgroup->vnode_set[i + 1]))
			break;
	}

	if (i > pfgroup->vnode_count - 1)
	{
		return 0;
	}

	if (height > 500 && height - pvlink->inner.ypos < 15 * 15)
	{
		return 0;
	}

	//获取链接的穿插信息
	html_vnode_t *left_node = NULL;
	html_vnode_t *right_node = NULL;
	link_interlude_info_t link_ci_info;
	if (i >= 0)
	{
		left_node = pfgroup->vnode_set[i];
	}
	if (i + 1 < pfgroup->vnode_count)
	{
		right_node = pfgroup->vnode_set[i + 1];
	}
	get_link_interlude_info(pvlink, left_node, right_node, &link_ci_info);

	return check_cont_interlude_inner(&link_ci_info, pfgroup, flag);
}

/**
 * @brief 标记链接类型
 *
 * @param [in/out] vlink   : vlink_t*	链接集合
 * @param [in] link_count   : int	链接个数
 * @param [in] root   : html_vnode_t*	root结点
 * @param [in] footer_tag_code   : int	网页页脚开始结点序号
 * @param [in] text_group_set   : text_group_t*	网页的内容组
 * @param [in] text_group_count   : int	内容组的个数
 * @param [in] ptgroup_main   : text_group_t*	主正文内容组
 * @return  void 
 **/
static void mark_cont_interlude_type(vlink_t *vlink, int link_count, html_vnode_t *root, int footer_tag_code,
		text_group_t *text_group_set, int text_group_count, text_group_t *ptgroup_main)
{
	//标记链接
	for (int i = 0; i < link_count; i++)
	{
		if (!is_cont_interlude_url(&vlink[i]) || (vlink[i].linkFunc & VLINK_QUOTATION))
			continue;

		if (check_cont_interlude(&vlink[i], ptgroup_main, footer_tag_code, root->hx, CHECK_NORMAL))
		{
			vlink[i].linkFunc |= VLINK_CONT_INTERLUDE;
			continue;
		}

		for (int j = 0; j < text_group_count; j++)
		{
			text_group_t *bakgroup = &text_group_set[j];
			if (bakgroup->pure_text_len > 220 && bakgroup != ptgroup_main)
			{
				if (check_cont_interlude(&vlink[i], bakgroup, footer_tag_code, root->hx, CHECK_STRICT))
				{
					vlink[i].linkFunc |= VLINK_CONT_INTERLUDE;
					break;
				}

			}
		}
	}
	//左右传递链接标记
	//注，此处由于 vlink[i-1].linkFunc & VLINK_CONT_INTERLUDE ==1 语句 优先级错误，未能执行的策略，下次升级效果
	//可以重新评估该策略
	/*
	 for (int i = 0; i < link_count; i++)
	 {
	 if (vlink[i].linkFunc & VLINK_CONT_INTERLUDE ||
	 !is_cont_interlude_url(&vlink[i]))
	 continue;

	 if (i > 0 && vlink[i-1].linkFunc & VLINK_CONT_INTERLUDE == 1 &&
	 vlink[i].inner.node->html_tag.tag_code - vlink[i-1].inner.node->html_tag.tag_code <= 4)
	 {
	 vlink[i].linkFunc |= VLINK_CONT_INTERLUDE;
	 continue;
	 }

	 if (i < link_count-1 && vlink[i+1].linkFunc & VLINK_CONT_INTERLUDE == 1 &&
	 vlink[i+1].inner.node->html_tag.tag_code - vlink[i].inner.node->html_tag.tag_code <= 4)
	 {
	 vlink[i].linkFunc |= VLINK_CONT_INTERLUDE;
	 continue;
	 }
	 }*/

	//传递相同链接标记
	for (int i = 0; i < link_count; i++)
	{
		if (vlink[i].inner.is_goodlink == 0)
			continue;

		for (int j = i + 1; j < link_count; j++)
		{
			if (vlink[j].inner.is_goodlink == 0)
				continue;

			if ((vlink[i].linkFunc & VLINK_CONT_INTERLUDE) && (vlink[j].linkFunc & VLINK_CONT_INTERLUDE))
				continue;

			if (!(vlink[i].linkFunc & VLINK_CONT_INTERLUDE) && !(vlink[j].linkFunc & VLINK_CONT_INTERLUDE))
				continue;

			if (strcmp(vlink[i].url, vlink[j].url) == 0)
			{
				vlink[i].linkFunc |= VLINK_CONT_INTERLUDE;
				vlink[j].linkFunc |= VLINK_CONT_INTERLUDE;
			}
		}
	}
}

/**
 * @brief 标记内容穿插链接
 * @param [in/out] pvarg   : vhp_args_t*	传入参数
 * @param [in/out] pvres   : vhp_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @retval			<0		函数出错
 **/
int mark_linktype_cont_interlude(lt_args_t *pvarg, lt_res_t *pvres)
{
	if (PT_IS_HUB(pvarg->pagetype) && !PT_IS_BLOG(pvarg->pagetype))
		return 0;
	cont_interlude_res_t *pres_ci = pvres->res_cont_interlude;
	vlink_t *vlink = pvarg->vlink;
	int link_count = pvarg->link_count;
	tree_visit_t cont_itl_data;

	cont_itl_data.route = 0;
	cont_itl_data.text_group_count = 0;
	cont_itl_data.re_end_tag_code = 0;
	cont_itl_data.resource = pres_ci;
	cont_interlude_res_reset(pres_ci);

	//遍历htmltree获得内容分层信息
	get_feature4ci_from_tree(pvarg->root, &cont_itl_data);

	//获取主内容段
	text_group_t *ptgroup_main = NULL;
	ptgroup_main = get_content_group(&cont_itl_data);
	if (ptgroup_main == NULL)
	{
		return 0;
	}

	//获得页脚开始结点的序号
	int footer_tag_code = MAX_END_TAG_CODE;
	for (html_area_t *subArea = pvarg->atree->root->subArea; subArea; subArea = subArea->nextArea)
	{
		if (subArea->abspos_mark == PAGE_FOOTER)
			footer_tag_code = subArea->begin->hpNode->html_tag.tag_code;
	}
	if (footer_tag_code == MAX_END_TAG_CODE && cont_itl_data.re_end_tag_code != 0)
	{
		footer_tag_code = cont_itl_data.re_end_tag_code;
	}

	mark_cont_interlude_type(vlink, link_count, pvarg->root, footer_tag_code, cont_itl_data.text_group_set,
			cont_itl_data.text_group_count, ptgroup_main);

	for (int i = 0; i < link_count; i++)
	{
		if (vlink[i].linkFunc & VLINK_COPYRIGHT)
		{
			vlink[i].linkFunc = vlink[i].linkFunc & (~VLINK_CONT_INTERLUDE);
		}
	}
	return 0;
}
