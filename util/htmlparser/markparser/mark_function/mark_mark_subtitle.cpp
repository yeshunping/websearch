/**
 * easou_mark_subtitle.cpp
 * Description: 标记文章的副标题
 *  Created on: 2011-11-24
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "easou_string.h"
#include "easou_html_attr.h"
#include "easou_mark_markinfo.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "debuginfo.h"

#define TEXT_BUF_LEN    1024
#define MAX_NODE        600
#define ID_CLASS_LEN    50
#define TEXT_MAX_LEN    256

#define IS_SUBTITLE     2
#define MAYBE_SUBTITLE  1
#define NOT_SUBTITLE    0

const int prime[] =
{ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
		317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677,
		683, 691, 701, 709, 719, 727, 733, 739, 743 };
const int max_prime_num = sizeof(prime) / sizeof(prime[0]);

typedef struct subtit_collector_t
{
	char tag_title[MAX_TITLE_LEN];

	bool is_subtitle;
	int width;
	int height;
	int count;
	html_area_t *cur_area;
	html_vnode_t *vnode[MAX_NODE];
	int vnode_table_first[MAX_NODE];
	int vnode_is_title[MAX_NODE];
	int vnode_depth[MAX_NODE];
	int vnode_route[MAX_NODE];
	char vnode_id_class_val[MAX_NODE][ID_CLASS_LEN];
	int vnode_has_title[MAX_NODE];
	char vnode_text[MAX_NODE][TEXT_MAX_LEN];
	int vnode_text_len[MAX_NODE];
	int vnode_has_time[MAX_NODE];
	html_area_t *vnode_area[MAX_NODE];

	int vnode_pre_str_type[MAX_NODE];
	int vnode_pre_str_os[MAX_NODE];
	int vnode_ch_str_num[MAX_NODE];
	int vnode_text_hash_value[MAX_NODE];
	int vnode_id[MAX_NODE];
} subtit_collector_t;

static bool is_unproper_tag(html_tag_type_t tag_type)
{
	if (tag_type == TAG_CODE)
		return true;

	return false;
}

/**
 * @brief 块是否包含codenode；true：包含
 */
static bool is_in_unproper_tag(html_area_t *area)
{
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid)
		{
			if (is_unproper_tag(vnode->hpNode->html_tag.tag_type))
				return true;
		}

		if (vnode == area->end)
			break;
	}

	int stat = 0;
	html_vnode_t *up = area->begin->upperNode;
	int level_wx = area->area_info.width;
	int level_hx = area->area_info.height;

	while (up)
	{
		if (is_unproper_tag(up->hpNode->html_tag.tag_type))
			return true;
		if (up->wx > level_wx || up->hx > level_hx)
		{
			stat++;
			if (stat == 2)
				break;
			level_wx = up->wx;
			level_hx = up->hx;
		}
		up = up->upperNode;
	}

	return false;
}

static bool is_table_col(html_area_t *area)
{
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid && (vnode->hpNode->html_tag.tag_type == TAG_TD || vnode->hpNode->html_tag.tag_type == TAG_TH))
		{
			return true;
		}
	}

	return false;
}

/**
 * @brief 是否是表行；true：该块是表行
 */
static bool is_table_row(html_area_t *area)
{
	if (area->area_info.height >= 60)
		return false;

	html_area_t *last = NULL;

	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (subarea->isValid && subarea->baseinfo->text_info.con_size > 0)
		{
			if (last != NULL)
			{
				if (subarea->area_info.ypos == last->area_info.ypos && is_table_col(subarea))
				{
					return true;
				}
				else
					return false;
			}
			last = subarea;
		}
	}

	return false;
}

/**
 * @brief
 * @param [in/out] area   : html_area_t*
 * @return  bool
 **/
static bool is_to_skip_subtree(html_area_t *area)
{
	if (!area->isValid)
	{
		return true;
	}
	int use_con_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	if (use_con_size < 4)
	{
		marktreeprintfs(MARK_SUBTITLE, "the useful text size <4,so skip the area(id=%d) at %s(%x)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (area->depth == 1 || (area->depth == 2 && area->area_info.height >= 200))
	{
		if (area->pos_mark == RELA_LEFT || area->pos_mark == RELA_RIGHT)
		{
			marktreeprintfs(MARK_SUBTITLE, "the area(id=%d) is at rela_left or at rela_right,so skip it at %s(%x)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
	}
	if (area->area_info.width <= 250 && area->area_info.height >= area->area_info.width)
	{
		//竖条
		marktreeprintfs(MARK_SUBTITLE, "its height(%d)>=its width(%d),so skip the area(id=%d) at %s(%x)-%s\r\n", area->area_info.height, area->area_info.width, area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (is_in_func_area(area, AREA_FUNC_MYPOS) || is_in_func_area(area, AREA_FUNC_NAV) || is_in_func_area(area, AREA_FUNC_RELATE_LINK) || is_in_func_area(area, AREA_FUNC_FRIEND) || is_in_func_area(area, AREA_FUNC_ARTICLE_SIGN) || is_in_srctype_area(area, AREA_SRCTYPE_OUT)
			|| is_in_srctype_area(area, AREA_SRCTYPE_INTERACTION) || (is_in_srctype_area(area, AREA_SRCTYPE_PIC) && area->baseinfo->pic_info.pic_num >= 4))
	{
		marktreeprintfs(MARK_SUBTITLE, "the area(id=%d) is in func or src area,upper_func=%x,upper_src=%x,so skip it at %s(%d)-%s\r\n", area->no, area->upper_func_mark._mark_bits, area->upper_srctype_mark._mark_bits, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (is_in_unproper_tag(area))
	{
		marktreeprintfs(MARK_SUBTITLE, "the area(id=%d) contains code tag,so skip it at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (is_table_row(area))
	{
		marktreeprintfs(MARK_SUBTITLE, "the area(id=%d) is one row of table,so skip it at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	return false;
}

static bool has_time_str(const char *tit)
{
	for (const char *p = tit; *p; p += GET_CHR_LEN(p))
	{
		if (is_time_str(p))
			return true;
	}

	return false;
}

/**
 * @brief tag_tit是否含有candi_tit;true:含有
 */
static bool is_partof_tag_title(const char *tag_tit, const char *candi_tit)
{
	const char *ptit = candi_tit;
	if (strlen(candi_tit) >= 20)
		ptit += 6;

	/**
	 * 越过前缀,增加容错性
	 */
	if (strstr(tag_tit, ptit) != NULL)
		return true;

	const char *p = strstr(candi_tit, "标题");
	if (p != NULL)
	{
		p += strlen("标题");
		if (strncmp(p, ":", strlen(":")) == 0 || strncmp(p, "：", strlen("：")) == 0)
		{
			return true;
		}
	}

	return false;
}

/**
 * @brief 该节点的父节点的id、class属性值是否含有title；true：含有
 */
static int upperNode_has_title(html_vnode_t *vnode, char *str, int str_len)
{
	str[0] = '\0';

	if (vnode == NULL)
		return 0;

	const char title[] = "title";

	if (vnode->upperNode)
	{
		const char *id_class_val = get_attribute_value((html_tag_t *) &vnode->upperNode->hpNode->html_tag, ATTR_ID);

		if (id_class_val == NULL)
			id_class_val = get_attribute_value((html_tag_t *) &vnode->upperNode->hpNode->html_tag, ATTR_CLASS);

		if (id_class_val != NULL && easou_strcasestr(id_class_val, title) != NULL)
		{
			strncpy(str, id_class_val, str_len - 1);
			str[str_len - 1] = '\0';
			return 1;
		}
	}
	return 0;
}

/**
 * @brief 计算该vnode节点到根的深度
 *
 * @param [in/out] vnode   : html_vnode_t*
 * @param [in/out] depth   : int*
 * @param [in/out] route   : int*
 **/
static int cal_vnode_depth(html_vnode_t * vnode, int *depth, int *route)
{
	*depth = 0;
	*route = 0;

	html_vnode_t * node = vnode;

	while (node)
	{
		*route += node->hpNode->html_tag.tag_type * prime[*depth];
		*depth = *depth + 1;

		if (*depth >= max_prime_num)
			break;

		node = node->upperNode;
	}
	return 1;
}

static int visit_for_subtitle_2(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
	{
		return VISIT_SKIP_CHILD;
	}
	subtit_collector_t *collect = (subtit_collector_t *) data;
	if (vnode->hpNode->html_tag.tag_type != TAG_PURETEXT)
	{
		//marktreeprintfs(MARK_SUBTITLE,"the node is not puretext at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return VISIT_NORMAL;
	}
	if (collect->count >= MAX_NODE)
	{
		marktreeprintfs(MARK_SUBTITLE, "get node number =600,so finish at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return VISIT_FINISH;
	}
	collect->vnode[collect->count] = vnode;
	collect->vnode_area[collect->count] = collect->cur_area;
	collect->count++;
	return VISIT_SKIP_CHILD;
}

/**
 * @brief
 * @param [in/out] area   : html_area_t*
 * @param [in/out] data   : void*
 **/
static int visit_for_subtitle(html_area_t *area, void *data)
{
	assert(area!=NULL);
	if (MARK_SUBTITLE == g_EASOU_DEBUG)
	{
		printlines("mark subtitle");
		printNode(area->begin->hpNode);
	}
	if (area->isValid == false)
	{
		marktreeprintfs(MARK_SUBTITLE, "the area(id=%d) is not valid,so it is not subtitle at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	if (!IS_LEAF_AREA(area->areaattr))
	{
		marktreeprintfs(MARK_SUBTITLE, "it is not leaf area(id=%d) ,it is not subtitle  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	subtit_collector_t *collect = (subtit_collector_t *) data;
	if (is_to_skip_subtree(area))
	{
		marktreeprintfs(MARK_SUBTITLE, "the area(id=%d) is to skip,so skip it at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo;
	if (paoi->text_info.con_num != 1)
	{
		marktreeprintfs(MARK_SUBTITLE, "the number of text (%d)!=1,so the area(id=%d) is not subtitle at %s(%d)-%s\r\n", paoi->text_info.con_num, area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	collect->cur_area = area;
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		html_vnode_visit(vnode, visit_for_subtitle_2, NULL, collect);
		if (vnode == area->end)
		{
			break;
		}
	}
	return AREA_VISIT_SKIP;
}

/**
 * @brief
 *
 * @param [in/out] collect   : subtit_collector_t*
 * @return  void
 **/
static void subtit_collector_clean(subtit_collector_t * collect)
{
	collect->tag_title[0] = 0;
	collect->is_subtitle = 0;
	collect->count = 0;
}

void clean_route(subtit_collector_t *collect, int route)
{
	for (int i = 0; i < collect->count; i++)
		if (route == collect->vnode_route[i])
		{
			collect->vnode_is_title[i] = MAYBE_SUBTITLE;
		}
}

/**
 * @brief 祖先是否为表
 */
static int is_table_first(html_vnode_t *vnode)
{
	if (vnode == NULL)
		return 0;

	html_vnode_t * node = vnode;
	while (node)
	{
		if (node->hpNode->html_tag.tag_type == TAG_TABLE)
			return 1;

		if (node->prevNode)
			return 0;

		node = node->upperNode;
	}
	return 0;
}

static int text_is_samepat(char *text1, int len1, char *text2, int len2)
{

	int lcs = 0;
	int i = 0;
	int lcs_left = 0;
	int lcs_right = 0;
	int maxl = 0;
	for (i = 0; i < len1 && i < len2; i++)
	{
		if (text1[i] == text2[i])
			lcs_left++;
		if (text1[len1 - 1 - i] == text2[len2 - 1 - i])
			lcs_right++;
	}
	lcs = (lcs_left > lcs_right ? lcs_left : lcs_right);
	maxl = (len1 > len2 ? len1 : len2);

	if (5 * (maxl - lcs) < maxl)
		return 1;

	return 0;

}

void fill_vnode_info(subtit_collector_t *collect)
{
	int i;
	for (i = 0; i < collect->count; i++)
	{
		collect->vnode_is_title[i] = IS_SUBTITLE;
		html_vnode_t *vnode = collect->vnode[i];
		font_t *font = &vnode->font;
		cal_vnode_depth(collect->vnode[i], &collect->vnode_depth[i], &collect->vnode_route[i]);
		int has_title = upperNode_has_title(vnode, collect->vnode_id_class_val[i], ID_CLASS_LEN);
		collect->vnode_has_title[i] = has_title;
		collect->vnode_table_first[i] = is_table_first(vnode);
		collect->vnode_text[i][0] = 0;
		copy_html_text(collect->vnode_text[i], 0, sizeof(collect->vnode_text[i]) - 1, vnode->hpNode->html_tag.text);
		str_trim_side_space(collect->vnode_text[i]);
		trans2bj_lower(collect->vnode_text[i], collect->vnode_text[i]);
		collect->vnode_text_len[i] = strlen(collect->vnode_text[i]);
		collect->vnode_has_time[i] = -1;
		if (has_title || font->is_bold || font->is_strong || font->align == VHP_TEXT_ALIGN_CENTER || font->header_size)
		{
			collect->vnode_is_title[i] = IS_SUBTITLE;
		}
		else
		{
			marktreeprintfs(MARK_SUBTITLE, "the node(%s) is not title,not bold,not strong,not center,not head_size,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
			collect->vnode_is_title[i] = NOT_SUBTITLE;
		}
	}
}
static const char *COLON_STR[] =
{ ":", "：" };
inline bool text_has_colon(char *text)
{
	int n = sizeof(COLON_STR) / sizeof(char *);
	for (int i = 0; i < n; i++)
	{
		if (strstr(text, COLON_STR[i]) != NULL)
			return true;
	}
	return false;
}
void removal_key_value(subtit_collector_t *collect)
{
	int i = 0, j = 0;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] != IS_SUBTITLE)
		{
			continue;
		}
		for (j = i + 1; j < collect->count; j++)
		{
			html_vnode_t *vnode_i = collect->vnode[i];
			html_vnode_t *vnode_j = collect->vnode[j];
			font_t *font_i = &vnode_i->font;
			font_t *font_j = &vnode_j->font;

			if (vnode_i->xpos == vnode_j->xpos && font_i->align == font_j->align && font_i->size == font_j->size && font_i->line_height == font_j->line_height && collect->vnode_route[i] == collect->vnode_route[j])
			{
				break;
			}
		}
		if (j < collect->count)
		{
			if (text_has_colon(collect->vnode_text[i]) && text_has_colon(collect->vnode_text[j]) && collect->vnode[j]->ypos - collect->vnode[i]->ypos < 60 && collect->vnode_text_len[i] < 20 && collect->vnode_text_len[j] < 20)
			{
				collect->vnode_is_title[i] = MAYBE_SUBTITLE;
				collect->vnode_is_title[j] = MAYBE_SUBTITLE;
				marktreeprintfs(MARK_SUBTITLE, "the node(%s) contains ':', and is the same path with another,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
				marktreeprintfs(MARK_SUBTITLE, "the node(%s)  contains ':', and is the same path with another,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[j], __FILE__, __LINE__, __FUNCTION__);
			}
		}
	}
}

void removal_bad_pos(subtit_collector_t *collect)
{
	int i, j;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] != IS_SUBTITLE)
		{
			continue;
		}
		for (j = 0; j < collect->count; j++)
		{
			if (j != i && collect->vnode_is_title[j] == IS_SUBTITLE)
			{
				html_vnode_t *vnode_i = collect->vnode[i];
				html_vnode_t *vnode_j = collect->vnode[j];
				font_t *font_i = &vnode_i->font;
				font_t *font_j = &vnode_j->font;
				if (vnode_i->xpos == vnode_j->xpos && font_i->size == font_j->size && font_i->line_height == font_j->line_height && font_i->bgcolor == font_j->bgcolor && font_i->color == font_j->color && font_i->align == font_j->align && collect->vnode_route[i] == collect->vnode_route[j])
					break;
			}
		}
		if (j == collect->count && collect->vnode_table_first[i] == false)
		{
			marktreeprintfs(MARK_SUBTITLE, "the node(%s) is not in table node,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
			collect->vnode_is_title[i] = MAYBE_SUBTITLE;
		}
		else
		{
			int first = 0;
			int end = collect->count - 1;
			while (collect->vnode_is_title[first] < IS_SUBTITLE || collect->vnode_route[first] != collect->vnode_route[i])
				first++;

			while (collect->vnode_is_title[end] < IS_SUBTITLE || collect->vnode_route[end] != collect->vnode_route[i])
				end--;

			html_vnode_t *vnode_lca_first = collect->vnode[first];
			html_vnode_t *vnode_lca_end = collect->vnode[end];

			while (1)
			{
				if (vnode_lca_first == NULL || vnode_lca_end == NULL)
					break;

				if (vnode_lca_first == vnode_lca_end)
				{
					if (10 * vnode_lca_first->xpos > 7 * collect->width || 10 * (vnode_lca_first->xpos + vnode_lca_first->wx) < 3 * collect->width || 10 * vnode_lca_first->ypos > 7 * collect->height)
					{
						clean_route(collect, collect->vnode_route[i]);
						marktreeprintfs(MARK_SUBTITLE, "the node(%s)  is the same path with another, and not center,delete the node with route=%d,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], collect->vnode_route[i], __FILE__, __LINE__, __FUNCTION__);
					}
					break;
				}
				vnode_lca_first = vnode_lca_first->upperNode;
				vnode_lca_end = vnode_lca_end->upperNode;
			}
		}
	}
}

void removal_same_text(subtit_collector_t *collect)
{
	int i, j;
	for (i = 0; i < collect->count; i++)
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			for (j = i + 1; j < collect->count; j++)
				if (collect->vnode_is_title[j] == IS_SUBTITLE && collect->vnode_route[i] == collect->vnode_route[j] && text_is_samepat(collect->vnode_text[i], collect->vnode_text_len[i], collect->vnode_text[j], collect->vnode_text_len[j]) == 1)
				{
					collect->vnode_is_title[i] = MAYBE_SUBTITLE;
					collect->vnode_is_title[j] = MAYBE_SUBTITLE;
					marktreeprintfs(MARK_SUBTITLE, "the node(%s)  is the same text pattern with another,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
					marktreeprintfs(MARK_SUBTITLE, "the node(%s)  is the same text pattern with another,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[j], __FILE__, __LINE__, __FUNCTION__);
				}
		}
}

void removal_longorshort_text(subtit_collector_t *collect)
{
	int i, j;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			int text_len = collect->vnode_text_len[i];
			if (text_len < 4 || text_len > 80)
			{
				for (j = 0; j < collect->count; j++)
				{
					if (collect->vnode_route[j] == collect->vnode_route[i])
					{
						collect->vnode_is_title[j] = MAYBE_SUBTITLE;
						marktreeprintfs(MARK_SUBTITLE, "the node(%s)  is after the long or short node ,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[j], __FILE__, __LINE__, __FUNCTION__);
					}
				}
			}
		}
	}
}

void removal_has_time(subtit_collector_t *collect)
{
	int i;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			if (20 * collect->vnode[i]->xpos > 13 * collect->width)
			{
				collect->vnode_is_title[i] = MAYBE_SUBTITLE;
				marktreeprintfs(MARK_SUBTITLE, "the node(%s)  xpos > 0.65* the page width,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
			}
			if (has_time_str(collect->vnode_text[i]))
			{
				collect->vnode_is_title[i] = MAYBE_SUBTITLE;
				marktreeprintfs(MARK_SUBTITLE, "the node(%s)  contain time,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
			}
			if (i + 1 < collect->count && (collect->vnode_is_title[i + 1] == NOT_SUBTITLE || has_time_str(collect->vnode_text[i + 1])) && collect->vnode[i]->ypos == collect->vnode[i + 1]->ypos)
			{
				collect->vnode_is_title[i] = MAYBE_SUBTITLE;
				marktreeprintfs(MARK_SUBTITLE, " the next node of the node(%s)  is not subtitle or contain time,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
			}
			if (i - 1 >= 0 && collect->vnode_text_len[i - 1] > 2 && collect->vnode[i]->ypos == collect->vnode[i - 1]->ypos)
			{
				collect->vnode_is_title[i] = MAYBE_SUBTITLE;
				marktreeprintfs(MARK_SUBTITLE, "the pre node length of the node(%s)  >2,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
			}
		}
	}
}

void reser_single(subtit_collector_t *collect)
{
	int i = 0;
	int j = -1;
	for (; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			if (j == -1)
				j = i;
			else
				j = -2;
		}
	}
	if (j >= 0)
	{
		collect->vnode_is_title[j] = MAYBE_SUBTITLE;
		marktreeprintfs(MARK_SUBTITLE, "the node(%s)  is only one subtitle,so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], __FILE__, __LINE__, __FUNCTION__);
	}
}

void reser_single_depth(subtit_collector_t *collect)
{
	const int MAX_DEPTH = 1000;
	int i = 0;
	int min_depth = MAX_DEPTH;
	int second_min = MAX_DEPTH;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE && collect->vnode_depth[i] < min_depth)
		{
			second_min = min_depth;
			min_depth = collect->vnode_depth[i];
		}
	}
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] < IS_SUBTITLE && collect->vnode_depth[i] > second_min)
		{
			collect->vnode_is_title[i] = MAYBE_SUBTITLE;
			marktreeprintfs(MARK_SUBTITLE, "the depth of the node(%s)  > second_min(%d),so it not subtitle at %s(%d)-%s\r\n", collect->vnode_text[i], second_min, __FILE__, __LINE__, __FUNCTION__);
		}
	}
}

void removal_too_more_count(subtit_collector_t *collect)
{
	const int MAX_NUM = 10000;
	int i = 0;
	int j = 0;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			int min = MAX_NUM;
			int max = 0;
			int count = 0;
			int height = 0;
			int dis = 0;

			for (j = 0; j < collect->count; j++)
			{
				html_vnode_t *vnode_i = collect->vnode[i];
				html_vnode_t *vnode_j = collect->vnode[j];

				font_t *font_i = &vnode_i->font;
				font_t *font_j = &vnode_j->font;

				if (vnode_i->xpos == vnode_j->xpos && font_i->align == font_j->align && font_i->size == font_j->size && font_i->line_height == font_j->line_height && collect->vnode_route[i] == collect->vnode_route[j])
				{
					if (collect->vnode_is_title[j] == IS_SUBTITLE)
					{
						count++;

						if (j < min)
							min = j;

						if (j > max)
							max = j;

						if (height > 0 && collect->vnode[j]->ypos != height && collect->vnode[j]->ypos != height + font_j->line_height)
							dis = 1;

						height = font_j->line_height + collect->vnode[j]->ypos;
					}
				}
			}
			if (count > 1 && max - min + 1 == count && dis == 0)
			{
				clean_route(collect, collect->vnode_route[i]);
				marktreeprintfs(MARK_SUBTITLE, " delete the node with route=%d at %s(%d)-%s\r\n", collect->vnode_route[i], __FILE__, __LINE__, __FUNCTION__);
			}
		}
	}
}

void removal_diff(subtit_collector_t *collect)
{
	int i = 0;
	int j = 0;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			int same = 0;
			int diff = 0;
			for (j = 0; j < collect->count; j++)
			{
				html_vnode_t *vnode_i = collect->vnode[i];
				html_vnode_t *vnode_j = collect->vnode[j];

				font_t *font_i = &vnode_i->font;
				font_t *font_j = &vnode_j->font;

				if (vnode_i->xpos == vnode_j->xpos && font_i->align == font_j->align && font_i->size == font_j->size && font_i->line_height == font_j->line_height && strcmp(collect->vnode_id_class_val[i], collect->vnode_id_class_val[j]) == 0 && collect->vnode_route[i] == collect->vnode_route[j])
				{
					if (collect->vnode_is_title[j] == IS_SUBTITLE)
						same++;
					else
						diff++;
				}
			}
			if (diff > same)
			{
				clean_route(collect, collect->vnode_route[i]);
				marktreeprintfs(MARK_SUBTITLE, "delete the node(%s)with route=%d,these nodes is same in xpos,font_size,font_height,id_class_val at %s(%d)-%s\r\n", collect->vnode_text[i], collect->vnode_route[i], __FILE__, __LINE__, __FUNCTION__);
			}
		}
	}
}

void removal_like_title(subtit_collector_t *collect)
{
	int i = 0;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			if (collect->vnode_text_len[i] >= 10 && is_partof_tag_title(collect->tag_title, collect->vnode_text[i]))
			{
				collect->vnode_is_title[i] = MAYBE_SUBTITLE;
			}
		}
	}
}

void removal_maxormin_hxspan(subtit_collector_t *collect)
{
	int i = 0;
	int j = -1;
	int min_hxspan = 10000;
	int max_hxspan = 0;
	int last_ypos = -1;
	int last_height = -1;
	int repeat_num = 0;
	int first_ypos = -1;
	int end_ypos = -1;

	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] < IS_SUBTITLE)
			continue;

		html_vnode_t *vnode_i = collect->vnode[i];
		if (last_ypos == -1)
		{
			repeat_num++;
			last_ypos = vnode_i->ypos;
			first_ypos = vnode_i->ypos;
			continue;
		}

		int hspan = vnode_i->ypos - last_ypos;

		if (hspan < min_hxspan)
			min_hxspan = hspan;

		if (hspan > max_hxspan)
			max_hxspan = hspan;

		last_ypos = vnode_i->ypos;
		last_height = vnode_i->hx;
		j = i;
		end_ypos = vnode_i->ypos;
		repeat_num++;
	}

	if (repeat_num < 2 || repeat_num > 16 || (max_hxspan < 80 && min_hxspan < 50))
	{
		if (j >= 0)
		{
			clean_route(collect, collect->vnode_route[j]);
			marktreeprintfs(MARK_SUBTITLE, "delete the node(%s)with route=%d,repeat_num(%d)<2||repeat_num>16 || (max_hxspan(%d)<80&&min_hxspan(%d)<50)  at %s(%d)-%s\r\n", collect->vnode_text[i], collect->vnode_route[i], repeat_num, max_hxspan, min_hxspan, __FILE__, __LINE__, __FUNCTION__);
		}
	}
}

void removal_bbs_time(subtit_collector_t *collect)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int time_count = 0;
	int title_count = 0;
	int route = 0;
	int size = -1;
	for (i = 0; i < collect->count; i++)
	{
		if (collect->vnode_is_title[i] == IS_SUBTITLE)
		{
			route = collect->vnode_route[i];

			k = i;
			title_count++;

			html_vnode_t *vnode_i = collect->vnode[i];
			font_t *font_i = &vnode_i->font;

			size = font_i->line_height;
			for (j = i + 1; j < collect->count; j++)
			{
				html_vnode_t *vnode_j = collect->vnode[j];
				font_t *font_j = &vnode_j->font;

				if (collect->vnode_route[j] == collect->vnode_route[i] && vnode_i->xpos == vnode_j->xpos && font_i->size == font_j->size && font_i->line_height == font_j->line_height && font_i->bgcolor == font_j->bgcolor && font_i->color == font_j->color && font_i->align == font_j->align)
					break;
				if (collect->vnode_has_time[j] == -1)
					collect->vnode_has_time[j] = has_time_str(collect->vnode_text[j]);

				if (collect->vnode_has_time[j])
				{
					time_count++;
					break;
				}
			}
		}
	}
	if (time_count == title_count && time_count > 0 && size < 25)
	{
		clean_route(collect, route);
		marktreeprintfs(MARK_SUBTITLE, "delete the node with route=%d  for bbs_time at %s(%d)-%s\r\n", route, __FILE__, __LINE__, __FUNCTION__);
	}
}

const int PRE_STR_LEN = 4;
const char g_ch_str[] = "一二三四五六七八九十";
const char g_pre_str[][PRE_STR_LEN] =
{ "第", "(", "[", "〔", "〈", "【" };
const int g_pre_str_len = sizeof(g_pre_str) / sizeof(g_pre_str[0]);
/*      第一    一,     一.     (一)    1.      [一]    〔九〕〈五〉【一】三:   */
const int BIG_PRIME = 991;

void add_cont_title_by_ch(subtit_collector_t *collect)
{
	const int SEQ_CH_MAX = 10;
	int i = 0;
	int j = 0;
	int k = 0;

	int offset = -1;
	int seq_ch[SEQ_CH_MAX] =
	{ 0 };
	int max_pre_len = 0;

	int hash_count[BIG_PRIME] =
	{ 0 };
	int hash_index[MAX_NODE] =
	{ 0 };

	int max_hash_id = 0;
	int text_hash[BIG_PRIME] =
	{ 0 };

	int ch_str_num = sizeof(g_ch_str) / sizeof(g_ch_str[0]) / 2;

	for (i = 0; i < collect->count; i++)
	{
		collect->vnode_pre_str_type[i] = -1;
		collect->vnode_pre_str_os[i] = -1;
		collect->vnode_ch_str_num[i] = -1;
		collect->vnode_id[i] = -1;
		collect->vnode_text_hash_value[i] = 0;

		if (collect->vnode_text_len[i] > 128 || collect->vnode_text_len[i] < 6)
			continue;

		int pre_os = 0;
		for (j = 0; j < g_pre_str_len; j++)
		{
			for (k = 0; g_pre_str[j][k]; k++)
				if (g_pre_str[j][k] != collect->vnode_text[i][k])
					break;

			if (g_pre_str[j][k] == '\0')
			{
				pre_os = k;
				break;
			}
		}
		for (k = 0; k < ch_str_num; k++)
		{
			if (memcmp(collect->vnode_text[i] + pre_os, g_ch_str + k + k, 2) == 0)
			{

				unsigned int id_1 = (unsigned char) collect->vnode_text[i][pre_os + 2];
				unsigned int id_2 = (unsigned char) collect->vnode_text[i][pre_os + 3];
				unsigned int id = id_1;

				if (id_1 > 128)
					id = id_1 * 128 + id_2;
				id %= BIG_PRIME;

				hash_count[id]++;
				hash_index[i] = id;

				int total_value = 0;
				for (char *pos = collect->vnode_text[i] + pre_os + 2; *pos != '\0'; pos++)
					total_value += (unsigned char) (*pos);

				text_hash[total_value % BIG_PRIME]++;

				collect->vnode_pre_str_type[i] = j;
				collect->vnode_pre_str_os[i] = pre_os;
				collect->vnode_ch_str_num[i] = k;
				collect->vnode_id[i] = id;
				collect->vnode_text_hash_value[i] = total_value % BIG_PRIME;
				break;
			}
		}
	}

	int os = 0;
	for (i = 0; i < collect->count; i++)
	{
		if (hash_count[hash_index[i]] > 2)
		{
			max_hash_id = hash_index[i];
			max_pre_len = collect->vnode_pre_str_type[i];
			os = collect->vnode_ch_str_num[i];

			break;
		}
	}
	if (i == collect->count)
		return;
	offset = 0;
	int succ0 = 0;
	for (i = 0; i < collect->count && offset < SEQ_CH_MAX; i++)
	{
		if (collect->vnode_pre_str_type[i] != max_pre_len || collect->vnode_id[i] != max_hash_id)
			continue;

		if (text_hash[collect->vnode_text_hash_value[i]] > 2)
			continue;

		if (collect->vnode_ch_str_num[i] == offset)
		{
			seq_ch[offset] = i + 1;
			if (offset > 0 && seq_ch[offset] - seq_ch[offset - 1] > 1)
				succ0 = 1;
			offset++;
		}
	}
	if (seq_ch[1] > 0 && succ0 == 1)
	{
		for (i = 0; i < SEQ_CH_MAX && i < offset; i++)
		{
			if (seq_ch[i] > 0)
			{

				collect->vnode_is_title[seq_ch[i] - 1] = IS_SUBTITLE;
				marktreeprintfs(MARK_SUBTITLE, " the node(%s) is subtitle ,is marked in  add_cont_title_by_ch()  at %s(%d)-%s\r\n", collect->vnode_text[seq_ch[i]-1], __FILE__, __LINE__, __FUNCTION__);
			}
		}
	}
}

void add_cont_title_by_digit(subtit_collector_t *collect)
{
	const int SEQ_CH_MAX = 100;
	int i = 0;
	int j = 0;

	int pos_num[SEQ_CH_MAX] =
	{ 0 };

	for (i = 0; i < collect->count; i++)
	{
		int num = 0;
		for (j = 0; collect->vnode_text[i][j] != '.'; j++)
			if (isdigit(collect->vnode_text[i][j]))
				num = num * 10 + collect->vnode_text[i][j] - '0';
			else
			{
				num = 0;
				break;
			}
		if (num > 0 && num < SEQ_CH_MAX)
		{
			pos_num[num] = i + 1;
		}
	}

	int succ = 0;
	for (i = 2; i < SEQ_CH_MAX; i++)
	{
		int dis = pos_num[i] - pos_num[i - 1];
		if ((pos_num[i] > 0 && dis < 0) || pos_num[i - 1] == 0)
		{
			succ = 0;
			break;
		}
		if (dis > 1)
			succ = 1;
	}
	if (succ == 1)
	{
		for (i = 2; i < SEQ_CH_MAX && pos_num[i - 1] > 0 && pos_num[i] > pos_num[i - 1]; i++)
		{
			collect->vnode_is_title[pos_num[i - 1] - 1] = IS_SUBTITLE;
			marktreeprintfs(MARK_SUBTITLE, " the node(%s) is subtitle ,is marked in  add_cont_title_by_digtil()  at %s(%d)-%s\r\n", collect->vnode_text[pos_num[i-1]-1], __FILE__, __LINE__, __FUNCTION__);
		}
	}
}

bool mark_func_subtitle(area_tree_t *atree)
{
	mark_area_info_t * mark_info = atree->mark_info;
	subtit_collector_t collect;
	subtit_collector_clean(&collect);
	trans2bj_lower(mark_info->tag_title, collect.tag_title);

	/**会找到一批候选，主要都是找pretext节点*/
	areatree_visit(atree, (FUNC_START_T) visit_for_subtitle, NULL, &collect);

	collect.width = atree->root->area_info.width;
	collect.height = atree->root->area_info.height;

	fill_vnode_info(&collect);
	removal_key_value(&collect);
	removal_bad_pos(&collect);
	removal_same_text(&collect);
	removal_longorshort_text(&collect);
	removal_has_time(&collect);
	reser_single(&collect);
	reser_single_depth(&collect);
	removal_too_more_count(&collect);
	removal_diff(&collect);
	removal_maxormin_hxspan(&collect);
	removal_bbs_time(&collect);
	add_cont_title_by_ch(&collect);
	add_cont_title_by_digit(&collect);

	int i = 0;
	for (i = 0; i < collect.count; i++)
	{
		if (collect.vnode_is_title[i] == IS_SUBTITLE)
		{
			tag_area_func(collect.vnode_area[i], AREA_FUNC_SUBTITLE);
		}
	}
	return true;
}

