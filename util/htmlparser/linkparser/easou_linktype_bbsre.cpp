/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_bbsre.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_bbsre.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 	标记bbs相关链接
 **/

#include "PageType.h"
#include "bbsparser.h"
#include "easou_html_attr.h"

#include "easou_link.h"
#include "easou_link_mark.h"
#include "easou_link_timematch.h"
#include "easou_link_area.h"
#include "easou_link_common.h"
#include "easou_link_tree.h"

#include <ctype.h>

/**
 * @brief 是否是签名档分割结点
 */
static bool is_sig_vnode(html_vnode_t * vnode)
{
	html_tag_t *tag = &vnode->hpNode->html_tag;
	if (tag->tag_type == TAG_IMG)
	{
		const char *src = get_attribute_value(tag, ATTR_SRC);
		if (src)
		{
			if (strstr(src, "sigline.gif") != NULL)
				return true;
			if (strstr(src, "qianming.gif") != NULL)
				return true;
			if (strstr(src, "sign.gif") != NULL)
				return true;
		}
	}
	if (tag->tag_type == TAG_PURETEXT)
	{
		if ((strstr(tag->text, "______________") != NULL || strstr(tag->text, "-------------") != NULL)
				&& get_valid_text_len(vnode) == 0 && vnode->nextNode
				&& vnode->nextNode->hpNode->html_tag.tag_type == TAG_BR)
			return true;
		if ((strstr(tag->text, "-----------------") != NULL || strstr(tag->text, "______________") != NULL)
				&& vnode->upperNode->hpNode->html_tag.tag_type == TAG_P)
			return true;
	}
	if (tag->tag_type == TAG_DIV)
	{
		const char *id = get_attribute_value(tag, ATTR_ID);
		const char *clas = get_attribute_value(tag, ATTR_CLASS);
		if (id && strstr(id, "signature") != NULL)
			return true;
		if (clas && strstr(clas, "signature") != NULL)
			return true;
	}
	return false;
}

/**
 * @brief 找到签名档分割结点
 */
static html_vnode_t * find_sig_vnode(html_vnode_t *start_vnode)
{
	html_vnode_t *iter_vnode = start_vnode;
	if (is_sig_vnode(iter_vnode))
		return iter_vnode;

	for (html_vnode_t *child = iter_vnode->firstChild; child != NULL; child = child->nextNode)
	{
		html_vnode_t *sig_vnode = find_sig_vnode(child);
		if (sig_vnode)
			return sig_vnode;
	}
	return NULL;
}

/**
 * @brief 标记bbs相关链接
 */
static int mark_bbs_link(vlink_t *pvlink, int start_num, int link_count, int flag, int min_tag_code, int max_tag_code,
		int post_tag_code, lt_res_t *pvres)
{
	vlink_t *sig_cont = NULL;
	int count = 0;

	for (int i = start_num; i < link_count; i++)
	{
		if (pvlink[i].tag_code > max_tag_code)
		{
			if (post_tag_code == -1 || pvlink[i].tag_code > post_tag_code)
			{
				if (count < 2 && sig_cont)
				{
					sig_cont->linkFunc &= ~VLINK_BBSSIG;
					sig_cont->linkFunc |= flag;
				}
				return i;
			}
			else
			{
				if (pvlink[i].inner.is_goodlink && !(pvlink[i].linkFunc & VLINK_COPYRIGHT))
					pvlink[i].linkFunc |= VLINK_BBSSIG;
				continue;
			}
		}

		if (pvlink[i].inner.is_goodlink && pvlink[i].tag_code > min_tag_code && !(pvlink[i].linkFunc & VLINK_COPYRIGHT))
		{
			int slide = 1;
			for (int j = pvlink[i].tag_code; j <= max_tag_code; j++)
			{
				if (j < pvres->tag_code_size && pvres->tag_code_len[j] > 0)
				{
					slide = 0;
					break;
				}
			}
			if (slide)
			{
				pvlink[i].linkFunc |= VLINK_BBSSIG;
				sig_cont = pvlink + i;
				count++;
			}
			else
				pvlink[i].linkFunc |= flag;
		}
	}
	//如果签名档里边链接过少，则标记为正常的bbs回复或者bbs主贴链接
	if (count < 2 && sig_cont)
	{
		sig_cont->linkFunc &= ~VLINK_BBSSIG;
		sig_cont->linkFunc |= flag;
	}
	return link_count;
}

/**
 * @brief 标记bbs链接类型
 */
static int tag_bbs_re_node(int post_index, html_vnode_t *start_vnode, vlink_t *pvlink, int start_num, int link_count,
		html_vnode_t *top_vnode, lt_res_t *pvres)
{
	int flag = 0;
	if (post_index == 1)
		flag = VLINK_BBSCONT;
	else
		flag = VLINK_BBSRE;

	//正文开始
	int min_tag_code = get_sub_tree_min_tag_code(start_vnode);
	//正文结束
	int max_tag_code = get_sub_tree_max_tag_code(start_vnode);

	//帖子结束
	int post_tag_code = -1;
	if (start_vnode->upperNode != NULL)
	{
		if (start_vnode->upperNode->upperNode != NULL)
			post_tag_code = get_sub_tree_max_tag_code(start_vnode->upperNode->upperNode);
		else
			post_tag_code = get_sub_tree_max_tag_code(start_vnode->upperNode);
	}

	if (post_tag_code - max_tag_code > max_tag_code - min_tag_code && post_tag_code - max_tag_code > 10)
		post_tag_code = -1;

	//优先选范围大的
	if (top_vnode)
	{
		html_vnode_t *vnode1 = start_vnode;
		while (vnode1->upperNode != NULL && vnode1->upperNode != top_vnode)
			vnode1 = vnode1->upperNode;
		if (vnode1->upperNode == top_vnode)
		{
			post_tag_code = get_sub_tree_max_tag_code(vnode1);
		}
	}

	//签名档跟正文混了
	html_vnode_t *sig = find_sig_vnode(start_vnode);
	if (sig)
	{
		post_tag_code = max_tag_code;
		max_tag_code = get_vnode_tag_code(sig);
	}

	//标记链接
	return mark_bbs_link(pvlink, start_num, link_count, flag, min_tag_code, max_tag_code, post_tag_code, pvres);
}

/**
 * @brief 获得bbs回复区域的父节点
 */
static html_vnode_t *get_top_vnode(bbs_post_list_t *post_list)
{
	if (post_list->post_count < 2)
	{
		return NULL;
	}
	html_vnode_t *first = post_list->post[0].maincont->maincont_vnodes->set[0];
	html_vnode_t *second = post_list->post[1].maincont->maincont_vnodes->set[0];

	while (first->upperNode != NULL && second->upperNode != NULL && first->upperNode != second->upperNode)
	{
		first = first->upperNode;
		second = second->upperNode;
	}
	if (first->upperNode == second->upperNode)
	{
		return first->upperNode;
	}
	return NULL;
}

/**
 * @brief 获得某个分块的时间个数
 */
static int get_time_count(html_area_t *parea, lt_area_info_t *parea_info)
{
	if (parea == NULL || parea->no >= MAX_AREA_NUM)
		return 0;

	area_node_info *pcur_node = &parea_info->area_nodes[parea->no];

	if (pcur_node->time_cnt == 0)
	{
		pcur_node->time_cnt = cacu_max_depth_time_count(pcur_node);
	}

	return pcur_node->time_cnt;
}

/**
 * @brief short description 遍历树的临时结构
 */
typedef struct
{
	html_area_t **area; //输出符合要求的分块
	lt_area_info_t *parea_info; //分块统计信息
} area_visit_t;

/**
 * @brief 找到时间个数最多的分块
 */
static int find_max_time_area(html_area_t *area, void *gb)
{
	area_visit_t * pav = (area_visit_t *) gb;
	html_area_t **parea = pav->area;
	if (!area->isValid || area->depth > 2)
		return AREA_VISIT_SKIP;
	if (get_time_count(area, pav->parea_info) > get_time_count(*parea, pav->parea_info)
			&& get_time_count(area, pav->parea_info) > 0)
	{
		*parea = area;
	}
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 找到时间个数与父分块一样多的子分块
 */
html_area_t *find_max_time_sub_area(html_area_t *area, lt_area_info_t *parea_info)
{
	for (html_area_t *subArea = area->subArea; subArea; subArea = subArea->nextArea)
	{
		if (get_time_count(area, parea_info) == get_time_count(subArea, parea_info))
		{
			return find_max_time_sub_area(subArea, parea_info);
		}
	}
	return area;
}

/**
 * @brief 检查链接是否与max_tag_code结点相邻,即没有其它有效的文字信息
 */
static bool is_slide(vlink_t *plink, int max_tag_code, lt_res_t *pvres)
{
	for (int i = plink->tag_code + 1; i <= max_tag_code; i++)
	{
		if (i < pvres->tag_code_size && pvres->tag_code_len[i] > 0)
		{
			return false;
		}
	}
	return true;
}

/**
 * @brief bbs parser 解析是否成功
 */
static bool bbs_parser_success(int ret, bbs_post_list_t *post_list)
{
	if (ret == BBS_EXTRACT_FAILED || ret == BBS_EXTRACT_EMPTY || ret == BBS_EXTRACT_BY_TEMPLATE_DISMATCH)
	{
		return false;
	}
	else
	{
		for (int i = 0; i < post_list->spost_count; i++)
		{
			if (post_list->special_post[i].maincont->maincont_str[0] != '\0')
			{
				return true;
			}
		}
		for (int i = 0; i < post_list->post_count; i++)
		{
			if (post_list->post[i].maincont->maincont_str[0] != '\0')
			{
				return true;
			}
		}
	}
	return false;
}

/**
 * @brief 通过时间分段分块来标记bbs链接类型，准确率低
 */
static void mark_bbsre_by_time_split(lt_args_t *pvarg, lt_res_t *pvres)
{
	get_area_info(pvarg->atree, pvres->area_info, pvarg->url, pvres->ptime_match);
	html_area_t *area = NULL;

	area_visit_t av;
	av.area = &area;
	av.parea_info = pvres->area_info;
	areatree_visit(pvarg->atree, (FUNC_START_T) find_max_time_area, NULL, &av);

	if (get_time_count(area, pvres->area_info) == 1 && area->depth > 1 && area->parentArea)
	{
		area = area->parentArea;
		if (area->depth > 1 && area->parentArea)
			area = area->parentArea;
	}
	else if (area)
	{
		area = find_max_time_sub_area(area, pvres->area_info);
	}

	if (get_time_count(area, pvres->area_info) == 0)
		return;

	int cur = 0;
	int flag = 0;
	for (html_area_t *subArea = area->subArea; subArea; subArea = subArea->nextArea)
	{
		if (get_time_count(subArea, pvres->area_info) == 0)
			continue;

		if (cur == 0)
			flag = VLINK_BBSCONT;
		else
			flag = VLINK_BBSRE;

		vlink_t *sig_cont = NULL;
		int count = 0;
		int max_tag_code = get_sub_tree_max_tag_code(subArea->begin);
		html_vnode_t *sig = find_sig_vnode(subArea->begin);

		for (int i = 0; i < pvarg->link_count; i++)
		{
			if (pvarg->vlink[i].inner.vnode == NULL)
				continue;
			if (pvarg->vlink[i].inner.is_goodlink && !(pvarg->vlink[i].linkFunc & VLINK_COPYRIGHT))
			{
				if (in_one_area(&pvarg->vlink[i].inner, subArea))
				{
					if (sig && pvarg->vlink[i].tag_code > get_vnode_tag_code(sig))
					{
						pvarg->vlink[i].linkFunc |= VLINK_BBSSIG;
					}
					else if (is_slide(pvarg->vlink + i, max_tag_code, pvres))
					{
						pvarg->vlink[i].linkFunc |= VLINK_BBSSIG;
						sig_cont = pvarg->vlink + i;
						count++;
					}
					else
					{
						pvarg->vlink[i].linkFunc |= flag;
					}
				}
			}
		}
		//通过is_slide方式标记的签名链接，如果个数少于2，则不标记，因为通常不准
		if (count < 2 && sig_cont)
		{
			sig_cont->linkFunc |= flag;
			sig_cont->linkFunc &= ~VLINK_BBSSIG;
		}

		cur++;
	}

	//与友情链接互斥
	for (int i = 0; i < pvarg->link_count; i++)
	{
		if (pvarg->vlink[i].inner.is_goodlink && (pvarg->vlink[i].linkFunc & VLINK_FRIEND))
		{
			pvarg->vlink[i].linkFunc &= ~VLINK_BBSRE;
			pvarg->vlink[i].linkFunc &= ~VLINK_BBSSIG;
			pvarg->vlink[i].linkFunc &= ~VLINK_BBSCONT;
		}
	}
}

/**
 * @brief 通过bbsparser的解析结果进行标注
 */
static void mark_bbsre_by_bbsparser(lt_args_t *pvarg, lt_res_t *pvres)
{
	bbs_post_list_t *post_list = pvres->post_list;
	html_vnode_t *maincont_vnode = NULL;
	vlink_t *pvlink = pvarg->vlink;
	int link_num = 0;
	int link_count = pvarg->link_count;
	int post_index = 0;
	html_vnode_t *top_vnode = get_top_vnode(post_list);

	for (int i = 0; i < post_list->spost_count; i++)
	{
		if (link_num == link_count)
		{
			break;
		}
		post_index++;
		maincont_vnode = post_list->special_post[i].maincont->maincont_vnodes->set[0];
		link_num = tag_bbs_re_node(post_index, maincont_vnode, pvlink, link_num, link_count, top_vnode, pvres);
	}

	for (int i = 0; i < post_list->post_count; i++)
	{
		if (link_num == link_count)
		{
			break;
		}
		post_index++;
		maincont_vnode = post_list->post[i].maincont->maincont_vnodes->set[0];
		link_num = tag_bbs_re_node(post_index, maincont_vnode, pvlink, link_num, link_count, top_vnode, pvres);
	}

	//解析的回复数很少，有可能与底层的友情链接混淆，进行互斥处理
	if (post_index < 3)
	{
		for (int i = 0; i < pvarg->link_count; i++)
		{
			if (pvarg->vlink[i].inner.vnode == NULL)
				continue;
			if (pvarg->vlink[i].inner.is_goodlink && (pvarg->vlink[i].linkFunc & VLINK_FRIEND))
			{
				pvarg->vlink[i].linkFunc &= ~VLINK_BBSRE;
				pvarg->vlink[i].linkFunc &= ~VLINK_BBSSIG;
				pvarg->vlink[i].linkFunc &= ~VLINK_BBSCONT;
			}
		}
	}
}

/**
 * @brief 标记bbsre链接
 * @param [in/out] pvarg   : vhp_args_t*		传入的参数包
 * @param [in/out] pvres   : vhp_res_t*		传入的资源
 * @return  int 
 * @retval   	0		函数正常返回
 * @retval			-1	函数运行出错
 **/
int mark_linktype_bbsre(lt_args_t *pvarg, lt_res_t *pvres)
{
	if (!PT_IS_FORUM(pvarg->pagetype))
	{
		return 0;
	}

	get_tag_code_text_len(pvarg->root, pvres->tag_code_len, pvres->tag_code_size);
	int ret = bbs_extract_all_by_vtree(pvarg->url, pvarg->vtree, pvres->post_list, NULL, true);
	if (bbs_parser_success(ret, pvres->post_list))
	{
		mark_bbsre_by_bbsparser(pvarg, pvres);
	}
	else
	{
		mark_bbsre_by_time_split(pvarg, pvres);
	}
	return 0;
}
