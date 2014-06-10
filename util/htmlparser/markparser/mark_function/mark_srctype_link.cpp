/**
 * mark_srctype_link.cpp
 * Description: 标记链接资源类型
 *  Created on: 2011-11-22
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "id_map.h"
#include "easou_html_attr.h"
#include "easou_mark_inner.h"
#include "easou_mark_srctype.h"
#include "easou_mark_com.h"
#include "debuginfo.h"

/**
 * @brief 当前程序内部对vnode的标记结构.
 */
typedef struct
{
	unsigned int need_skip :1;
} vnode_mark_t;

/**
 * @brief 是否跳过重复节点rpt_vnode；true:跳过
 */
static bool is_skip_repeat_vnode(html_vnode_t *rpt_vnode, id_spec_map_t *id_smap)
{
	vnode_mark_t *vm = (vnode_mark_t *) id_spec_map_get_node(id_smap, rpt_vnode->id);
	return vm && vm->need_skip;
}

/**
 * @brief 设置跳过rpt_vnode节点
 */
static void set_repeat_vnode_need_skip(html_vnode_t *rpt_vnode, id_spec_map_t *id_smap)
{
	vnode_mark_t *vm = (vnode_mark_t *) id_spec_map_add_node(id_smap, rpt_vnode->id);
	if (vm)
	{
		vm->need_skip = 1;
	}
}

typedef struct
{
	int same_url_num; /**< 相同URL的数量 */
	int same_anchor_num; /**< 相同的anchor的数量 */
	int align_link_num; /**< 重复子结构中,与主链接对齐的链接数量 */

	int link_repeat_num; /**< 有链接的重复子结构的数量 */
	int repeat_cnt; /**< 重复子节点重复次数 */

	int tot_sep_num; /**< 重复子节点间的间隔节点数量 */
	int sep_mean; /**< 平均间隔数量 */
	int sep_var; /**< 间隔数量的均方差 */
	int max_sep_cont_size; /**< 最大间隔文本长度 */

	int tot_other_rep_num; /**< 重复子节点间间隔的其他重复结构数量 */
	int other_rep_mean; /**< 平均间隔的其他重复节点数量 */
	int other_req_var; /**< 间隔的其他重复节点方差 */

	int align_link_size; /**< 对齐的链接总长度 */
	int max_align_link_size; /**< 对齐链接的最大长度 */

	int yspan; /**< 重复区域在竖直方向上的跨度 */
	int upper_yspan; /**< 父节点在竖起方向上的跨度 */

	html_vnode_t *curr_max_link_vnode_inline; /**< 同一行最大的链接节点 */

	int left_line_cont_size; /**< 主链接左边的内容长度 */
	int right_line_cont_size; /**< 主链接右边的内容长度 */
	int cont_num_inline; /**< 同一行的文本节点数量 */

	int link_size_inline; /**< 同一行的链接长度 */
	int link_num_inline; /**< 同一行的链接数量 */

	int cont_size_before_link; /**< 当前分块第一个链接之前的文本长度 */
	int repeat_cont_size_before_link; /**< 重复结构中链接之前的文本长度 */
	int cont_size_before_repeat; /**< 重复区域之前的文本长度 */
	int cont_size_after_repeat; /**< 重复区域之后的文本长度 */

	int link_num; /**< 当前分块链接数量 */
	int anchor_size; /**< 当前分块的链接长度 */
	int cont_size; /**< 当前分块文本内容长度 */
	int pure_text_size; /**< 当前分块纯文本长度 */
	int link_area; /**< 当前分块链接区域面积 */
	int tot_area; /**< 当前分块有效内容的总面积 */
	int other_area; /**< 当前分块除文本和链接之外的面积 */
	int inter_area; /**< 当前分块交互区域的面积 */

	unsigned int is_in_left_side :1; /**< 主链接是否位于重复子结构左边框 */
	unsigned int is_multi_line :1; /**< 重复子结构是否有多行 */
	unsigned int is_img_link :1; /**< 主链接是否图片 */
	unsigned int is_area_inline :1; /**< 当前分块是否处于一行 */
	unsigned int is_repeat_updown :1; /**< 重复兄弟是否竖直排列 */
	unsigned int is_repeat_inline :1; /**< 重复兄弟是否水平排列 */
} repeat_link_info_t;

/**
 * @brief 计算节点纵向跨度（高度），保存在r_link_info->upper_yspan
 */
static void get_vnode_span(repeat_link_info_t *r_link_info, html_vnode_t *vnode)
{
	int top_ypos = 10000;
	int bot_ypos = -10000;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid || child->subtree_textSize <= 0)
		{
			continue;
		}
		if (child->ypos < top_ypos)
		{
			top_ypos = child->ypos;
		}
		if (child->ypos + child->hx > bot_ypos)
		{
			bot_ypos = child->ypos + child->hx;
		}
	}

	r_link_info->upper_yspan = bot_ypos - top_ypos;
}

/**
 * @brief 判断两个节点是否在一行；true：在一行
 */
inline bool is_in_one_line(html_vnode_t *vnode_a, html_vnode_t *vnode_b)
{
	if (vnode_a->ypos + vnode_a->hx <= vnode_b->ypos)
	{
		return false;
	}
	if (vnode_b->ypos + vnode_b->hx <= vnode_a->ypos)
	{
		return false;
	}
	return true;
}

/**
 * @brief 计算link_vode节点在repeat_vnode所在块中同行的文本节点数量，并统计文本数量信息
 */
static void get_same_line_text_info(repeat_link_info_t *r_link_info, html_vnode_t *link_vnode, html_vnode_t *repeat_vnode)
{
	html_area_t *hp_area = repeat_vnode->hp_area;
	vnode_list_t *vlist_begin = hp_area->baseinfo->text_info.cont_vnode_list_begin;
	vnode_list_t *vlist_end = hp_area->baseinfo->text_info.cont_vnode_list_end;
	for (vnode_list_t *vlist = vlist_begin; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		if (!is_in_one_line(vnode, link_vnode))
		{
			r_link_info->is_multi_line = 1;
			goto _continue_;
		}
		if (vnode->subtree_textSize > 0)
		{
			r_link_info->cont_num_inline++;
		}
		if (vnode->xpos < link_vnode->xpos)
		{
			r_link_info->left_line_cont_size += vnode->subtree_textSize;
		}
		else if (vnode->xpos >= link_vnode->xpos + link_vnode->wx)
		{
			r_link_info->right_line_cont_size += vnode->subtree_textSize;
		}
		_continue_: if (vlist == vlist_end)
		{
			break;
		}
	}
}

/**
 * @brief 计算link_vnode节点在repeat_vnode节点所在块中的链接节点，同行节点数量统计文本数量
 */
static void get_same_line_link_info(repeat_link_info_t *r_link_info, html_vnode_t *link_vnode, html_vnode_t *repeat_vnode)
{
	html_area_t *hp_area = repeat_vnode->hp_area;
	vnode_list_t *vlist_begin = hp_area->baseinfo->link_info.url_vnode_list_begin;
	vnode_list_t *vlist_end = hp_area->baseinfo->link_info.url_vnode_list_end;
	int max_anchor_size_inline = 0;
	for (vnode_list_t *vlist = vlist_begin; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		if (vnode->ypos != link_vnode->ypos)
		{
			goto _continue_;
		}
		r_link_info->link_num_inline++;
		r_link_info->link_size_inline += vnode->subtree_textSize;
		if (vnode->subtree_textSize > max_anchor_size_inline)
		{
			max_anchor_size_inline = vnode->subtree_textSize;
			r_link_info->curr_max_link_vnode_inline = vnode;
		}
		_continue_: if (vlist == vlist_end)
		{
			break;
		}
	}
}

/**
 * @brief 统计同一行文本节点和锚节点信息
 */
static void get_same_line_info(repeat_link_info_t *r_link_info, html_vnode_t *link_vnode, html_vnode_t *repeat_vnode)
{
	get_same_line_text_info(r_link_info, link_vnode, repeat_vnode);
	get_same_line_link_info(r_link_info, link_vnode, repeat_vnode);
}

/**
 * @brief 提前包含vnode节点最小块的文本内容
 */
inline void get_vnode_anchor(char *buffer, int size, html_vnode_t *vnode)
{
	extract_area_content(buffer, size, vnode->hp_area);
}

static int start_travel_common_tree(html_vnode_t *vnode, html_vnode_t *align_vnode, void *data)
{
	html_vnode_t **link_vnode = (html_vnode_t **) data;
	if (vnode == *link_vnode)
	{
		*link_vnode = align_vnode;
		return VISIT_FINISH;
	}

	return VISIT_NORMAL;
}

static html_vnode_t *get_align_link_vnode(html_vnode_t *last_repeat_vnode, html_vnode_t *repeat_vnode, html_vnode_t *link_vnode, html_vtree_t *vtree)
{
	html_vnode_t *align_lvnode = link_vnode;
	travel_max_common_tree(last_repeat_vnode, repeat_vnode, vtree, vtree, start_travel_common_tree, NULL, &align_lvnode);

	if (align_lvnode == link_vnode || !align_lvnode->inLink)
	{
		return NULL;
	}
	return align_lvnode;
}

static bool is_infomative_url(const char *url)
{
	if (NULL == url || strncasecmp(url, "javascript:", strlen("javascript:")) == 0)
	{
		return false;
	}
	return true;
}

static void get_align_link_info(repeat_link_info_t *r_link_info, html_vnode_t *last_repeat_vnode, html_vnode_t *link_vnode, html_vnode_t *repeat_vnode, const html_vtree_t *vtree)
{
	const char * link_href = get_attribute_value(&link_vnode->hpNode->html_tag, ATTR_HREF);
	bool is_url_infomative = is_infomative_url(link_href);

	static const int BUF_SIZE = 256;
	char anchor[BUF_SIZE];
	anchor[0] = 0;

	get_vnode_anchor(anchor, BUF_SIZE, link_vnode);

	html_vnode_t *align_link_vnode = get_align_link_vnode(last_repeat_vnode, repeat_vnode, link_vnode, (html_vtree_t *) vtree);

	if (align_link_vnode)
	{
		html_vnode_t *lvnode = align_link_vnode;
		r_link_info->align_link_num++;
		r_link_info->align_link_size += lvnode->subtree_textSize;
		if (lvnode->subtree_textSize > r_link_info->max_align_link_size)
		{
			r_link_info->max_align_link_size = lvnode->subtree_textSize;
		}
		get_same_line_info(r_link_info, lvnode, repeat_vnode);
		if (link_href && is_url_infomative)
		{
			const char *l_href = get_attribute_value(&lvnode->hpNode->html_tag, ATTR_HREF);
			if (l_href && strcmp(l_href, link_href) == 0)
			{
				r_link_info->same_url_num++;
			}
		}
		char tmp_anchor[BUF_SIZE];
		tmp_anchor[0] = 0;
		get_vnode_anchor(tmp_anchor, BUF_SIZE, lvnode);
		if (tmp_anchor[0] != '\0' && strcmp(tmp_anchor, anchor) == 0)
		{
			r_link_info->same_anchor_num++;
		}
	}
}

static int get_cont_size_befor_first_link(html_vnode_t *vnode)
{
	html_area_t *hp_area = vnode->hp_area;
	vnode_list_t *vlist_begin = hp_area->baseinfo->text_info.cont_vnode_list_begin;
	vnode_list_t *vlist_end = hp_area->baseinfo->text_info.cont_vnode_list_end;
	int size = 0;
	for (vnode_list_t *vlist = vlist_begin; vlist; vlist = vlist->next)
	{
		if (vlist->vnode->inLink)
		{
			break;
		}
		size += vlist->vnode->subtree_textSize;
		if (vlist == vlist_end)
		{
			break;
		}
	}
	return size;
}

static int get_cont_size_befor_repeat(html_vnode_t *vnode, vstruct_info_t *sp_node)
{
	int cont_size = 0;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
		{
			continue;
		}
		if (child->struct_info->similar_sign == sp_node->similar_sign)
		{
			break;
		}
		cont_size += child->subtree_textSize;
	}
	return cont_size;
}

static int get_repeat_cnt(html_vnode_t *vnode)
{
	int prev_cnt = 0;
	html_vnode_t *iterv = vnode->struct_info->prev_similar_vnode;
	while (iterv)
	{
		prev_cnt++;
		iterv = iterv->struct_info->prev_similar_vnode;
	}
	return vnode->struct_info->repeat_num - prev_cnt;
}

static void adjust_repeat_link_info(repeat_link_info_t *r_link_info)
{
	if (r_link_info->is_img_link && r_link_info->align_link_size == 0)
	{
		r_link_info->align_link_size = 8 * r_link_info->align_link_num;
		r_link_info->anchor_size += r_link_info->align_link_size;
		r_link_info->link_size_inline += r_link_info->align_link_size;
	}
}

inline int get_link_num(html_area_t *area)
{
	return area->baseinfo->link_info.num + area->baseinfo->link_info.other_num;
}

static void get_info_from_first_main_link(repeat_link_info_t *r_link_info, html_vnode_t *link_vnode, html_vnode_t *repeat_vnode)
{
	if (link_vnode->hp_area->baseinfo->pic_info.pic_area > 0)
	{
		r_link_info->is_img_link = 1;
	}

	r_link_info->cont_size_before_repeat = get_cont_size_befor_repeat(repeat_vnode->upperNode, repeat_vnode->struct_info);
	r_link_info->cont_size_before_link = get_cont_size_befor_first_link(repeat_vnode->upperNode);
	r_link_info->repeat_cont_size_before_link += get_cont_size_befor_first_link(repeat_vnode);

	r_link_info->curr_max_link_vnode_inline = link_vnode;
	get_same_line_info(r_link_info, link_vnode, repeat_vnode);
	if (r_link_info->curr_max_link_vnode_inline)
	{
		r_link_info->align_link_size = r_link_info->curr_max_link_vnode_inline->subtree_textSize;
		r_link_info->max_align_link_size = r_link_info->curr_max_link_vnode_inline->subtree_textSize;
	}
	r_link_info->align_link_num = 1;
	r_link_info->same_anchor_num = 1;
	r_link_info->same_url_num = 1;

	r_link_info->repeat_cnt = get_repeat_cnt(repeat_vnode);
	r_link_info->link_repeat_num = 1;
}

inline bool is_trivial_vnode(html_vnode_t *vnode)
{
	if (!vnode->isValid || vnode->wx <= 0 || vnode->hx <= 0)
	{
		return true;
	}
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		if (is_space_text(vnode->hpNode->html_tag.text))
		{
			return true;
		}
	}
	return false;
}

typedef struct
{
	int single_num;
	int tot_num;
	int tot_square;
} mean_var_stat_t;

static void statistic_for_non_repeat_sibling(mean_var_stat_t *sep_stat, mean_var_stat_t *other_rep_stat, repeat_link_info_t *r_link_info, html_vnode_t *sibl, vstruct_info_t *repeat_sp_node)
{
	if (IS_REPEAT_STRUCT_CHILD(sibl->property) && sibl->struct_info->repeat_num >= repeat_sp_node->repeat_num - 1)
	{
		if (get_link_num(sibl->hp_area) == 0)
		{
			other_rep_stat->single_num++;
		}
	}
	else
	{
		sep_stat->single_num++;
		if (sibl->subtree_textSize > r_link_info->max_sep_cont_size)
		{
			r_link_info->max_sep_cont_size = sibl->subtree_textSize;
		}
	}
}

inline void mean_var_stat_self_update(mean_var_stat_t *stat)
{
	stat->tot_num += stat->single_num;
	stat->tot_square += stat->single_num * stat->single_num;
	stat->single_num = 0;
}

typedef struct
{
	int top_ypos;
	int bot_ypos;
} rect_span_t;

inline void rect_span_update(rect_span_t *rect_span, html_vnode_t *vnode)
{
	if (vnode->ypos < rect_span->top_ypos)
	{
		rect_span->top_ypos = vnode->ypos;
	}
	if (vnode->ypos + vnode->hx > rect_span->bot_ypos)
	{
		rect_span->bot_ypos = vnode->ypos + vnode->hx;
	}
}

static void get_repeat_link_info(repeat_link_info_t *r_link_info, html_vnode_t *repeat_vnode, html_vnode_t *link_vnode, const html_vtree_t *vtree)
{
	vstruct_info_t *repeat_sp_node = repeat_vnode->struct_info;
	mean_var_stat_t sep_stat =
	{ 0, 0, 0 };
	mean_var_stat_t other_rep_stat =
	{ 0, 0, 0 };

	rect_span_t rect_span;
	rect_span.top_ypos = repeat_vnode->ypos;
	rect_span.bot_ypos = repeat_vnode->ypos + repeat_vnode->hx;

	get_info_from_first_main_link(r_link_info, link_vnode, repeat_vnode);

	html_vnode_t *last_max_link_inline = r_link_info->curr_max_link_vnode_inline;
	html_vnode_t *last_repeat_vnode = repeat_vnode;

	for (html_vnode_t *sibl = repeat_vnode->nextNode; sibl; sibl = sibl->nextNode)
	{
		if (is_trivial_vnode(sibl))
		{
			continue;
		}
		int sibl_link_num = get_link_num(sibl->hp_area);
		if (sibl->struct_info->similar_sign != repeat_sp_node->similar_sign || sibl_link_num == 0)
		{
			statistic_for_non_repeat_sibling(&sep_stat, &other_rep_stat, r_link_info, sibl, repeat_sp_node);
		}
		else
		{
			r_link_info->repeat_cont_size_before_link += get_cont_size_befor_first_link(sibl);
			get_align_link_info(r_link_info, repeat_vnode, link_vnode, sibl, vtree);
			r_link_info->link_repeat_num++;
			if (sibl->ypos > last_repeat_vnode->ypos)
			{
				r_link_info->is_repeat_updown = 1;
			}
			else
			{
				r_link_info->is_repeat_inline = 1;
			}
			mean_var_stat_self_update(&sep_stat);
			mean_var_stat_self_update(&other_rep_stat);
			rect_span_update(&rect_span, sibl);
			last_repeat_vnode = sibl;
			last_max_link_inline = r_link_info->curr_max_link_vnode_inline;
		}
	}
	int sep_cnt = r_link_info->link_repeat_num - 1;
	if (sep_cnt <= 0)
	{
		sep_cnt = 1;
	}

	r_link_info->tot_sep_num = sep_stat.tot_num;
	r_link_info->sep_mean = sep_stat.tot_num / sep_cnt;
	r_link_info->sep_var = sep_stat.tot_square / sep_cnt - r_link_info->sep_mean * r_link_info->sep_mean;
	r_link_info->tot_other_rep_num = other_rep_stat.tot_num;
	r_link_info->other_rep_mean = other_rep_stat.tot_num / sep_cnt;
	r_link_info->other_req_var = other_rep_stat.tot_square / sep_cnt - r_link_info->other_rep_mean * r_link_info->other_rep_mean;

	int next_i = 0;
	bool after_repeat = false;
	for (html_vnode_t *sibl = last_repeat_vnode->nextNode; sibl; sibl = sibl->nextNode)
	{
		if (!sibl->isValid || sibl->wx <= 0 || sibl->hx <= 0)
		{
			continue;
		}
		if (IS_REPEAT_STRUCT_CHILD(sibl->property) && sibl->struct_info->repeat_num >= repeat_sp_node->repeat_num - 1)
		{
			continue;
		}
		if (next_i++ >= r_link_info->sep_mean)
		{
			after_repeat = true;
		}
		if (!after_repeat && sibl->hpNode->html_tag.tag_type != TAG_PURETEXT)
		{
			rect_span_update(&rect_span, sibl);
		}
		else
		{
			r_link_info->cont_size_after_repeat += sibl->subtree_textSize;
		}
	}

	r_link_info->yspan = rect_span.bot_ypos - rect_span.top_ypos;
	get_vnode_span(r_link_info, repeat_vnode->upperNode);

	adjust_repeat_link_info(r_link_info);
}

typedef struct
{
	bool lower_found_link_area; /**< 向上遍历的过程中是否已标记了索引块 */
	int repeat_num; /**< 向上遍历中,上一次标记的索引块的重复子结构数量 */
	int similar_value; /**< 向上遍历中,上一次标记的索引块的重复子结构的相似度 */
} link_area_info_t;

static bool is_link_area_by_quick_judge_2(repeat_link_info_t *r_link_info)
{
	if (r_link_info->is_area_inline && r_link_info->other_area * 2 < r_link_info->tot_area)
	{
		if (r_link_info->align_link_num >= 2)
		{
			return true;
		}
	}
	if (r_link_info->link_area * 10 >= r_link_info->tot_area * 8 && r_link_info->other_area <= 20000 && r_link_info->align_link_num >= 3 && r_link_info->align_link_num * 3 >= r_link_info->link_num)
	{
		return true;
	}
	if (r_link_info->other_area <= 0 && r_link_info->align_link_num > 0 && r_link_info->pure_text_size * r_link_info->align_link_num <= r_link_info->align_link_size && r_link_info->pure_text_size <= 48)
	{
		return true;
	}
	return false;
}

typedef struct
{
	html_vnode_t *main_link_vnode;
	bool is_link_area;
} main_link_vnode_info_t;

typedef struct
{
	html_vnode_t *link_vnode;
	int align_link_size;
	int align_num;
} align_link_element_t;

static const int CANDI_LINK_NUM_LIMIT = 16;

typedef struct
{
	align_link_element_t align_link_ele[CANDI_LINK_NUM_LIMIT];
	int num;
} align_link_info_t;

static void add_align_link(align_link_info_t *ali_info, html_vnode_t *link_vnode)
{
	if (ali_info->num < CANDI_LINK_NUM_LIMIT)
	{
		ali_info->align_link_ele[ali_info->num].link_vnode = link_vnode;
		ali_info->align_link_ele[ali_info->num].align_link_size = link_vnode->subtree_textSize;
		ali_info->align_link_ele[ali_info->num].align_num = 1;
		ali_info->num++;
	}
}

static void update_align_link(align_link_info_t *ali_info, html_vnode_t *align_vnode, int index)
{
	ali_info->align_link_ele[index].align_link_size += align_vnode->subtree_textSize;
	ali_info->align_link_ele[index].align_num++;
}

static int travel_common_tree_for_align_link_info(html_vnode_t *vnode, html_vnode_t *align_vnode, void *data)
{
	align_link_info_t *ali_info = (align_link_info_t *) data;

	for (int i = 0; i < ali_info->num; i++)
	{
		if (vnode == ali_info->align_link_ele[i].link_vnode)
		{
			update_align_link(ali_info, align_vnode, i);
			break;
		}
	}
	return VISIT_NORMAL;
}

static void align_and_get_link_info(html_vnode_t *last_repeat_vnode, html_vnode_t *repeat_vnode, align_link_info_t *ali_info, html_vtree_t *vtree)
{
	travel_max_common_tree(last_repeat_vnode, repeat_vnode, vtree, vtree, travel_common_tree_for_align_link_info, NULL, ali_info);
}

static void get_ali_link_size(align_link_info_t *ali_info, html_vnode_t *vnode, html_vtree_t *vtree)
{
	html_vnode_t *rpt_vnode = vnode;
	while (1)
	{
		rpt_vnode = rpt_vnode->struct_info->next_similar_vnode;
		if (rpt_vnode == NULL)
		{
			break;
		}
		align_and_get_link_info(vnode, rpt_vnode, ali_info, vtree);
	}
}

static main_link_vnode_info_t get_main_link_vnode(html_vnode_t * link_vnode, html_vnode_t *rpt_vnode, html_vtree_t *vtree, repeat_link_info_t *r_link_info, id_spec_map_t *id_smap)
{
	main_link_vnode_info_t mlv_info;
	mlv_info.main_link_vnode = link_vnode;
	mlv_info.is_link_area = false;
	html_area_t *hp_area = rpt_vnode->hp_area;

	if (hp_area->baseinfo->link_info.num + hp_area->baseinfo->link_info.other_num == 1)
	{
		return mlv_info;
	}

	int ypos_limit = rpt_vnode->ypos + rpt_vnode->hx / 2;
	bool has_meet_long_link = false;
	vnode_list_t *vlist_begin = hp_area->baseinfo->link_info.url_vnode_list_begin;
	vnode_list_t *vlist_end = hp_area->baseinfo->link_info.url_vnode_list_end;
	html_vnode_t *first_link_vnode = NULL;
	html_vnode_t *begin_longest_vnode = NULL;
	int max_link_size = 0;
	int align_link_num = 0;

	align_link_info_t align_link_info;
	align_link_info.num = 0;

	for (vnode_list_t *vlist = vlist_begin; vlist; vlist = vlist->next)
	{
		if (vlist->vnode->subtree_textSize > 0 && first_link_vnode == NULL)
		{
			first_link_vnode = vlist->vnode;
		}
		if (first_link_vnode && vlist->vnode->ypos > first_link_vnode->ypos + first_link_vnode->hx && vlist->vnode->ypos >= ypos_limit)
		{
			break;
		}
		add_align_link(&align_link_info, vlist->vnode);
		if (vlist == vlist_end)
		{
			break;
		}
	}

	if (align_link_info.num <= 1)
	{
		return mlv_info;
	}

	get_ali_link_size(&align_link_info, rpt_vnode, vtree);

	for (int i = 0; i < align_link_info.num; i++)
	{
		html_vnode_t *vnode = align_link_info.align_link_ele[i].link_vnode;
		if (first_link_vnode && vnode->ypos > first_link_vnode->ypos + first_link_vnode->hx && has_meet_long_link)
		{
			break;
		}
		int ali_num = align_link_info.align_link_ele[i].align_num;
		int align_link_size = align_link_info.align_link_ele[i].align_link_size;
		if (align_link_size > max_link_size)
		{
			max_link_size = align_link_size;
			begin_longest_vnode = vnode;
			align_link_num = ali_num;
		}
		if (align_link_size >= 20 * ali_num && align_link_size > 0)
		{
			has_meet_long_link = true;
		}
	}

	if (align_link_num > 0)
	{
		r_link_info->align_link_size = max_link_size;
		r_link_info->align_link_num = align_link_num;
		mlv_info.is_link_area = is_link_area_by_quick_judge_2(r_link_info);
	}

	if (begin_longest_vnode)
	{
		set_repeat_vnode_need_skip(rpt_vnode, id_smap);
		mlv_info.main_link_vnode = begin_longest_vnode;
	}

	return mlv_info;
}

static bool is_repeat_struct(repeat_link_info_t *r_link_info)
{
	if (r_link_info->is_repeat_updown && !r_link_info->is_repeat_inline)
	{
		if (r_link_info->yspan * 5 < r_link_info->upper_yspan * 3 && r_link_info->yspan >= 100)
		{
			return false;
		}
	}
	if (r_link_info->tot_sep_num < 2 * (r_link_info->repeat_cnt - 1) && r_link_info->sep_var <= r_link_info->sep_mean)
	{
	}
	else
	{
		return false;
	}
	if (r_link_info->other_req_var >= 3)
	{
		return false;
	}
	if (r_link_info->align_link_num <= 3 && r_link_info->max_sep_cont_size >= r_link_info->cont_size / 2 && r_link_info->max_sep_cont_size > 0)
	{
		return false;
	}
	if (!r_link_info->is_area_inline)
	{
		if (r_link_info->cont_size_before_repeat >= 10 && r_link_info->cont_size_before_repeat >= 2 * r_link_info->align_link_size)
		{
			return false;
		}
		if (r_link_info->cont_size_after_repeat >= 10 && r_link_info->cont_size_after_repeat >= 2 * r_link_info->align_link_size)
		{
			return false;
		}
		if (r_link_info->cont_size_before_repeat >= 300 && r_link_info->cont_size_before_repeat >= r_link_info->align_link_size)
		{
			return false;
		}
		if (r_link_info->cont_size_after_repeat >= 300 && r_link_info->cont_size_after_repeat >= r_link_info->align_link_size)
		{
			return false;
		}
	}
	return true;
}

static bool is_link_centered(repeat_link_info_t *r_link_info)
{
	int con_size_inline = r_link_info->left_line_cont_size + r_link_info->right_line_cont_size;

	/**
	 * 要召回的索引块大致分为几种:
	 *  1，对于每个重复子结构来说链接所占区域较大，且重复子结构之间没有间隔.
	 *  2，链接在重复子结构中所占区域不大，但在每个重复子结构中，链接都占据中心地位，
	 *     其他内容是对链接的描述或补充信息.
	 *  3, 链接在重复子结构中，但对链接的描述或补充信息在重复子结构之外，即处于重复子节点的间隔
	 *     节点中.
	 */
	if (r_link_info->align_link_num < 2)
	{
		return false;
	}
	if (r_link_info->align_link_size <= 12 * r_link_info->align_link_num && r_link_info->align_link_size * 3 < r_link_info->anchor_size)
	{
	}
	else
	{
		if (r_link_info->tot_sep_num * 2 <= r_link_info->align_link_num)
		{
			if (r_link_info->anchor_size * 3 >= r_link_info->cont_size)
			{
				return true;
			}
			if (r_link_info->link_area * 3 >= r_link_info->tot_area)
			{
				return true;
			}
		}
	}
	if (r_link_info->link_repeat_num * 2 < r_link_info->repeat_cnt - 1)
	{
		return false;
	}
	if (r_link_info->align_link_num * 2 < (r_link_info->link_repeat_num - 1))
	{
		return false;
	}
	if (!r_link_info->is_area_inline && r_link_info->same_anchor_num >= r_link_info->align_link_num && r_link_info->same_anchor_num >= 3)
	{
		return false;
	}
	if (!r_link_info->is_area_inline && r_link_info->same_url_num >= 3)
	{
		return false;
	}
	if (r_link_info->inter_area * 2 > r_link_info->tot_area)
	{
		return false;
	}
	if (r_link_info->repeat_cont_size_before_link >= 30 * r_link_info->align_link_num && r_link_info->repeat_cont_size_before_link > r_link_info->align_link_size)
	{
		return false;
	}
	if (!r_link_info->is_area_inline && (r_link_info->is_multi_line || r_link_info->tot_sep_num > 0 || r_link_info->other_rep_mean > 0 || r_link_info->other_req_var > 0))
	{
		if (r_link_info->align_link_size <= 12 * r_link_info->align_link_num)
		{
			if (r_link_info->link_size_inline * 2 < con_size_inline)
			{
				return false;
			}
			if (r_link_info->align_link_size * 3 <= con_size_inline)
			{
				return false;
			}
		}
		if (r_link_info->link_size_inline <= 12 * r_link_info->align_link_num && r_link_info->cont_num_inline >= 5 * r_link_info->align_link_num)
		{
			return false;
		}
		if (r_link_info->align_link_size <= 11 * r_link_info->align_link_num)
		{
			if (con_size_inline > 0)
			{
				return false;
			}
			if (r_link_info->is_in_left_side)
			{
				return false;
			}
		}
	}
	return true;
}

/**
 * @brief 是否链接节点位于重复节点vnode的左边框.
 **/
static bool is_link_in_left_side(html_vnode_t *link_vnode, html_vnode_t *vnode)
{
	html_area_t *rep_area = vnode->hp_area;
	html_area_t *sub_area_with_link = link_vnode->hp_area;
	if (sub_area_with_link == rep_area)
	{
		return false;
	}
	while (sub_area_with_link->parentArea != rep_area)
	{
		sub_area_with_link = sub_area_with_link->parentArea;
	}
	if (RELA_LEFT == sub_area_with_link->pos_mark)
	{
		html_area_t *next_area = next_valid_area(sub_area_with_link);
		if (next_area && RELA_MAIN == next_area->pos_mark)
		{
			return true;
		}
	}
	return false;
}

static void check_area_inline(repeat_link_info_t *r_link_info, html_vnode_t *link_vnode, html_vnode_t *repeat_vnode)
{
	if (repeat_vnode->upperNode->hx < link_vnode->hx * 2)
	{
		r_link_info->is_area_inline = 1;
	}
}

static bool has_link(html_vnode_t *vnode)
{
	vnode_list_t *vlist_begin = vnode->hp_area->baseinfo->link_info.url_vnode_list_begin;
	vnode_list_t *vlist_end = vnode->hp_area->baseinfo->link_info.url_vnode_list_end;
	for (vnode_list_t *vlist = vlist_begin; vlist; vlist = vlist->next)
	{
		if (vlist->vnode->inLink)
		{
			return true;
		}
		if (vlist == vlist_end)
		{
			break;
		}
	}
	return false;
}

static bool is_ignore_repeat_vnode(html_vnode_t *rpt_vnode, link_area_info_t *linfo)
{
	if (!IS_REPEAT_STRUCT_PARENT(rpt_vnode->upperNode->property) && rpt_vnode->struct_info->similar_value < 80 && rpt_vnode->struct_info->align_percent < 80)
	{
		marktreeprintfs(MARK_SRC_LINK, "the area is ignore for its parent node is not repeat struct property and similar_value=%d,align_percent=%d at %s(%d)-%s\r\n", rpt_vnode->struct_info->similar_value, rpt_vnode->struct_info->align_percent, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (linfo->lower_found_link_area)
	{
		if (rpt_vnode->struct_info->repeat_num < linfo->repeat_num || rpt_vnode->struct_info->similar_value < linfo->similar_value)
		{
			marktreeprintfs(MARK_SRC_LINK, "the area is ignore for lower found link area  and rpt_vnode->struct_info->repeat_num(%d) < linfo->repeat_num(%d),rpt_vnode->struct_info->similar_value(%d) < linfo->similar_value(%d) at %s(%d)-%s\r\n",
					rpt_vnode->struct_info->repeat_num, linfo->repeat_num, rpt_vnode->struct_info->similar_value, linfo->similar_value, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
	}
	if (rpt_vnode->struct_info->next_similar_vnode == NULL)
	{
		marktreeprintfs(MARK_SRC_LINK, "the area is ignore for rpt_vnode->struct_info->next_similar_vnode == NULL at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (rpt_vnode->struct_info->prev_similar_vnode && has_link(rpt_vnode->struct_info->prev_similar_vnode))
	{
		marktreeprintfs(MARK_SRC_LINK, "the area is ignore for rpt_vnode->struct_info->pre_similar_vnode has link at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	return false;
}

inline int get_cont_size(html_area_t *area)
{
	return area->baseinfo->text_info.con_size;
}

inline int get_anchor_size(html_area_t *area)
{
	return area->baseinfo->link_info.anchor_size;
}

inline int get_total_area(html_area_t *area)
{
	return area->baseinfo->extern_info.extern_area + area->baseinfo->inter_info.in_area + area->baseinfo->pic_info.pic_area + area->baseinfo->text_info.text_area;
}

inline int get_link_area(html_area_t *area)
{
	return area->baseinfo->link_info.link_area;
}

inline int get_other_area(html_area_t *area)
{
	return area->baseinfo->pic_info.pic_area - area->baseinfo->pic_info.link_pic_area + area->baseinfo->inter_info.in_area + area->baseinfo->extern_info.extern_area;
}

inline int get_inter_area(html_area_t *area)
{
	return area->baseinfo->inter_info.in_area;
}

/**
 * @brief 简单根据链接所占比例就可以召回的.
 **/
static bool is_link_area_by_quick_judge(html_area_t *area)
{
	int cont_size = get_cont_size(area);
	int anchor_size = get_anchor_size(area);
	int total_area = get_total_area(area);
	int link_area = get_link_area(area);
	int other_area = get_other_area(area);

//	Debug("anchor:%d, cont:%d; link_area:%d, tot_area:%d other_area:%d",
//			anchor_size, cont_size, link_area, total_area, other_area);

	if (other_area <= 0 && anchor_size >= cont_size && anchor_size > 0)
	{
		return true;
	}

	if (total_area <= link_area && link_area > 0)
	{
		return true;
	}

	return false;
}

/**
 * @brief 记录当前分块的基本信息.
 **/
static void get_area_base_info(repeat_link_info_t *r_link_info, html_area_t *area)
{
	r_link_info->cont_size = get_cont_size(area);
	r_link_info->anchor_size = get_anchor_size(area);
	r_link_info->link_num = get_link_num(area);
	r_link_info->tot_area = get_total_area(area);
	r_link_info->link_area = get_link_area(area);
	r_link_info->other_area = get_other_area(area);
	r_link_info->inter_area = get_inter_area(area);
	r_link_info->pure_text_size = r_link_info->cont_size - r_link_info->anchor_size;
}

static bool is_mainly_link_line(repeat_link_info_t *r_link_info)
{
	if (r_link_info->is_area_inline && r_link_info->other_area * 2 < r_link_info->tot_area)
	{
		if (r_link_info->cont_size < 2 * r_link_info->anchor_size && r_link_info->anchor_size > 0)
		{
			return true;
		}
	}
	return false;
}

static bool is_link_area_final(repeat_link_info_t *r_link_info, bool is_repeat, bool is_link_center)
{
	if (is_repeat && is_link_center)
	{
		return true;
	}
	if (is_repeat && r_link_info->link_num >= 2)
	{
		if (r_link_info->anchor_size * 10 >= r_link_info->cont_size * 8)
		{
			return true;
		}
		if (r_link_info->link_area * 10 >= r_link_info->tot_area * 8)
		{
			return true;
		}
	}
	marktreeprintfs(MARK_SRC_LINK, "the area is not link area final at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	return false;
}

/**
 * @brief 根据链接节点,及当前链接节点所在的重复子节点判断是否索引块.
 * @param [in] rpt_vnode   : html_vnode_t* 重复子节点.
 * @param [in] vtree   : html_vtree_t* 所在的VTREE.
 * @param [in] link_node   : html_vnode_t* 链接节点.
 * @param [in] linfo   : link_area_info_t* 从链接向上遍历过程中,已经标记的索引块信息.
 * @param [in & out] id_smap   : id_spec_map_t*
 **/
static bool is_link_area(html_vnode_t *rpt_vnode, html_vtree_t *vtree, html_vnode_t *link_node, link_area_info_t *linfo, id_spec_map_t *id_smap)
{
	if (is_ignore_repeat_vnode(rpt_vnode, linfo))
	{
		/** 下次再遇到这个节点时,做出的判断仍然一样,因此需要标记一下*/
		set_repeat_vnode_need_skip(rpt_vnode, id_smap);
		marktreeprintfs(MARK_SRC_LINK, "the area is to ignore at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	html_area_t *up_area = rpt_vnode->upperNode->hp_area;
	if (is_link_area_by_quick_judge(up_area))
	{
		return true;
	}

	repeat_link_info_t r_link_info;
	memset(&r_link_info, 0, sizeof(repeat_link_info_t));

	get_area_base_info(&r_link_info, up_area);
	check_area_inline(&r_link_info, link_node, rpt_vnode);

	if (is_mainly_link_line(&r_link_info))
	{
		return true;
	}

	/** 获取主重复链接 */
	main_link_vnode_info_t mlv_info = get_main_link_vnode(link_node, rpt_vnode, vtree, &r_link_info, id_smap);
	if (mlv_info.is_link_area)
	{
		return true;
	}

	html_vnode_t *main_lvnode = mlv_info.main_link_vnode;
	r_link_info.is_in_left_side = is_link_in_left_side(main_lvnode, rpt_vnode);
	get_repeat_link_info(&r_link_info, rpt_vnode, main_lvnode, vtree);
	if (is_link_area_by_quick_judge_2(&r_link_info))
	{
		return true;
	}

	/**
	 *  链接块定义为以链接为中心的重复结构.
	 *  因此核心策略是判断当前分块是否重复结构，以及是否以链接为中心.
	 */
	bool is_repeat = is_repeat_struct(&r_link_info);

	bool is_link_center = is_link_centered(&r_link_info);
	marktreeprintfs(MARK_SRC_LINK, "mark link:is_repeat=%d ,is link_center=%d at %s(%d)-%s\r\n", is_repeat, is_link_center, __FILE__, __LINE__, __FUNCTION__);
	bool is_link_area = is_link_area_final(&r_link_info, is_repeat, is_link_center);

	return is_link_area;
}

/**
 * @brief 若该块的link节点所占面积大于总面积的1/3或锚文数量大于总文本数量1/3，标记该块为link
 */
static void check_tag_link_area(html_area_t *area)
{
	int cont_size = get_cont_size(area);
	int anchor_size = get_anchor_size(area);
	int total_area = get_total_area(area);
	int link_area = get_link_area(area);
	int other_area = get_other_area(area);

	if (link_area * 3 >= total_area)
	{
		tag_area_srctype(area, AREA_SRCTYPE_LINK);
	}
	if (other_area * 3 < total_area && anchor_size * 3 >= cont_size)
	{
		tag_area_srctype(area, AREA_SRCTYPE_LINK);
	}
}

static void go_upper_mark_link_area(html_vnode_t *link_vnode, html_vtree_t *vtree, id_spec_map_t *id_smap)
{
	link_area_info_t linfo;
	linfo.lower_found_link_area = false;
	linfo.repeat_num = 0;
	linfo.similar_value = 0;

	for (html_vnode_t *upper = link_vnode; upper; upper = upper->upperNode)
	{
		if (IS_REPEAT_STRUCT_CHILD(upper->property))
		{
			marktreeprintfs(MARK_SRC_LINK, "mark area no=%d at %s(%d)-%s\r\n", upper->upperNode->hp_area->no, __FILE__, __LINE__, __FUNCTION__);
			if (is_srctype_area(upper->upperNode->hp_area, AREA_SRCTYPE_HUB))
			{
				break;
			}
			if (is_skip_repeat_vnode(upper, id_smap))
			{
				marktreeprintfs(MARK_SRC_LINK, "the area is to skip at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
				continue;
			}
			if (is_link_area(upper, vtree, link_vnode, &linfo, id_smap) && !is_srctype_area(upper->upperNode->hp_area, AREA_SRCTYPE_INTERACTION))
			{
				tag_area_srctype(upper->upperNode->hp_area, AREA_SRCTYPE_HUB);
				check_tag_link_area(upper->upperNode->hp_area);
				linfo.lower_found_link_area = true;
				linfo.repeat_num = upper->struct_info->repeat_num;
				linfo.similar_value = upper->struct_info->similar_value;
			}
		}
	}
}

/**
 * @brief 对所有链接节点,向上查看是否存在重复结构,若存在则判断是否为索引块.
 **/
static void mark_srctype_link_internal(area_tree_t *atree, id_spec_map_t *id_smap)
{
	vnode_list_t *vlist_begin = atree->root->baseinfo->link_info.url_vnode_list_begin;
	for (vnode_list_t *vlist = vlist_begin; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		if (vnode->inLink)
		{
			go_upper_mark_link_area(vnode, (html_vtree_t *) atree->hp_vtree, id_smap); //遍历所有link节点
		}
	}
}

bool mark_srctype_link(area_tree_t *atree)
{
	/**
	 * 创建id_spec_map,用于标记节点,以控制程序流程.
	 */
	id_spec_map_t *id_smap = id_spec_map_create(sizeof(vnode_mark_t), 64);
	if (NULL == id_smap)
	{
		return false;
	}
	mark_srctype_link_internal(atree, id_smap);
	id_spec_map_destroy(id_smap);
	return true;
}

