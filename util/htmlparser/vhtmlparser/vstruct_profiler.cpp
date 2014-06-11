/**
 * easou_vstruct_profiler.cpp
 * Description: 计算子树结构信息
 *  Created on: 2011-06-27
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "util/htmlparser/utils/log.h"
#include "util/htmlparser/utils/id_map.h"
#include "util/htmlparser/htmlparser/html_attr.h"
#include "util/htmlparser/vhtmlparser/vhtml_inner.h"
#include "util/htmlparser/vhtmlparser/vhtml_parser.h"
#include "util/htmlparser/vhtmlparser/vstruct_profiler.h"

#include <math.h>

static const int MIN_REPEAT_VALUE = 65;
static const int MAX_ALIGN_PERCENT = 100;
static const int MIN_TEXT_SIZE = 2;

static max_common_tree_info_t
get_max_common_tree_info(html_vnode_t *vnode, html_vnode_t *another_vnode, bool is_trace);

static int visit_clear_trace_info(html_vnode_t *vnode, void *data);

#define IS_PURETEXT_VNODE(vnode)	(TAG_PURETEXT == vnode->hpNode->html_tag.tag_type)

/**
 * @brief

 * @date 2011/06/27
 **/
static int get_interpolation_value(const int *value_table, int table_size, int interval, int index)
{
	int hi_index = (index + interval - 1) / interval;
	int low_index = index / interval;
	int distant = index % interval;
	if (hi_index >= table_size)
	{
		return value_table[table_size - 1];
	}
	return value_table[low_index] + (value_table[hi_index] - value_table[low_index]) * distant / interval;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int get_align_percent_std_dev(int node_cnt)
{
	static int SMALL_STD_DEV_TABLE[] =
	{ 0, //for 0
			15, //for 1
			15, //for 2
			25, //for 3
			35, //for 4
			};
	static const int N_SMALL_DEV_TABLE = sizeof(SMALL_STD_DEV_TABLE) / sizeof(int);
	static const int STD_DEV_TABLE[] =
	{ 15, //for 0
			40, //for 20
			36, //for 40
			33, //for 60
			30, //for 80
			26, //for 100
			23, //for 120
			20, //for 140
			};
	static const int N_STD_DEV_TABLE = sizeof(STD_DEV_TABLE) / sizeof(int);
	static const int STD_DEV_TABLE_INERVAL = 20;

	if (node_cnt < N_SMALL_DEV_TABLE)
	{
		return SMALL_STD_DEV_TABLE[node_cnt];
	}

	return get_interpolation_value(STD_DEV_TABLE, N_STD_DEV_TABLE, STD_DEV_TABLE_INERVAL, node_cnt);
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int get_align_percent_mean(int node_cnt)
{
	static const int SMALL_MEAN_TABLE[] =
	{ 100, //for 0
			96, //for 1
			96, //for 2
			86, //for 3
			77, //for 4
			72, //for 5
			68, //for 6
			65, //for 7
			62, //for 8
			59, //for 9
			57, //for 10
			};
	static const int N_SMALL_MEAN_TABLE = sizeof(SMALL_MEAN_TABLE) / sizeof(int);
	static const int MEAN_TABLE[] =
	{ 70, //for 0
			44, //for 20
			31, //for 40
			24, //for 60
			20, //for 80
			17, //for 100
			15, //for 120
			14, //for 140
			};
	static const int N_MEAN_TABLE = sizeof(MEAN_TABLE) / sizeof(int);
	static const int MEAN_TABLE_INTERVAL = 20;

	if (node_cnt < N_SMALL_MEAN_TABLE)
	{
		return SMALL_MEAN_TABLE[node_cnt];
	}

	return get_interpolation_value(MEAN_TABLE, N_MEAN_TABLE, MEAN_TABLE_INTERVAL, node_cnt);
}

/**
 * @brief

 * @date 2011/06/27
 **/
void vhtml_struct_prof_del(html_vtree_t * html_vtree)
{
	if (html_vtree->struct_np_inited)
	{
		nodepool_destroy(&html_vtree->struct_np);
	}
	html_vtree->struct_np_inited = 0;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int vhtml_struct_prof_init(html_vtree_t *html_vtree)
{
	if (!html_vtree->struct_np_inited)
	{
		html_vtree->struct_np_inited = 1;
		if (!nodepool_init(&html_vtree->struct_np, sizeof(vstruct_info_t)))
		{
			goto ERR;
		}
	}
	return 1;

	ERR: vhtml_struct_prof_del(html_vtree);
	return -1;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static void vhtml_struct_prof_reset(html_vtree_t *html_vtree)
{
	nodepool_reset(&html_vtree->struct_np);
}

/**
 * @brief

 * @date 2011/06/27
 **/
inline html_vnode_t *next_valid(html_vnode_t *vnode)
{
	for (html_vnode_t *n = vnode; n; n = n->nextNode)
	{
		if (n->isValid)
		{
			return n;
		}
	}
	return NULL;
}

/**
 * @brief 获取vnode节点等宽等高的孩子节点

 * @date 2011/06/27
 **/
static html_vnode_t *get_sub_avail_node(html_vnode_t *vnode)
{
	html_vnode_t *valid_child = NULL;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
		{
			continue;
		}
		if (child->wx == vnode->wx && child->hx == vnode->hx)
		{
			return child;
		}
		else if (valid_child != NULL)
		{
			return NULL;
		}
		valid_child = child;
	}
	return valid_child;
}

/**
 * @brief true：对齐，判断两个节点是否tag type是否相同

 * @date 2011/06/27
 **/
inline bool is_align_vnode(html_vnode_t *vnode, html_vnode_t *another_vnode)
{
	html_tag_t *html_tag = &vnode->hpNode->html_tag;
	html_tag_t *th_html_tag = &another_vnode->hpNode->html_tag;
	if (html_tag->tag_type != th_html_tag->tag_type)
	{
		return false;
	}
	switch (html_tag->tag_type)
	{
	case TAG_PURETEXT:
	{
		if (vnode->inLink != another_vnode->inLink)
		{
			return false;
		}
		if (vnode->textSize <= MIN_TEXT_SIZE || another_vnode->textSize <= MIN_TEXT_SIZE)
		{
			if (is_space_text(html_tag->text) || is_space_text(th_html_tag->text))
			{
				return false;
			}
		}
		return true;
	}
	case TAG_UNKNOWN:
	{
		if (html_tag->tag_name && th_html_tag->tag_name && strcmp(html_tag->tag_name, th_html_tag->tag_name) == 0)
		{
			return true;
		}
		return false;
	}
	case TAG_INPUT:
	{
		const char *type_a = get_attribute_value(html_tag, ATTR_TYPE);
		const char *type_b = get_attribute_value(th_html_tag, ATTR_TYPE);
		if (type_a && type_b && strcasecmp(type_a, type_b) == 0)
		{
			return true;
		}
		if (type_a == type_b)
		{ /** the type value are NULL*/
			return true;
		}
		return false;
	}
	default:
		return true;
	}
}

/**
 * @brief

 * @date 2011/06/27
 **/
static bool downstairs_align_vnode(html_vnode_t * &down_node, html_vnode_t * to_align_vnode)
{
	for (; down_node; down_node = get_sub_avail_node(down_node))
	{
		if (is_align_vnode(down_node, to_align_vnode))
		{
			return true;
		}
	}
	return false;
}

/**
 * @brief 判断vnode与another_vnode的儿子是否对齐

 * @date 2011/06/27
 **/
static int vertical_misplace_align_vnode(html_vnode_t * &vnode, html_vnode_t * &another_vnode)
{
	if (vnode->struct_info->max_depth >= another_vnode->struct_info->max_depth)
	{
		if (downstairs_align_vnode(vnode, another_vnode) == true)
		{
			return 1;
		}
	}
	else
	{
		if (downstairs_align_vnode(another_vnode, vnode) == true)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int visit_clear_trace_info(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
	{
		return VISIT_SKIP_CHILD;
	}
	assert(vnode->struct_info);

	vnode->struct_info->align_vnode_for_trace = NULL;
	vnode->struct_info->is_best_aligned_for_trace = 0;
	return VISIT_NORMAL;
}

static const max_common_tree_info_t MCT_INFO_INIT_VALUE =
{ 0, 0 };

#define IS_SUBTREE_ALIGNED(mct_info)	(mct_info.align_node_cnt > 0)

/**
 * @brief

 * @date 2011/06/27
 **/
static max_common_tree_info_t misplace_align_node(html_vnode_t * &vnode, html_vnode_t * &another_vnode, bool is_trace)
{
	html_vnode_t *misplace_node = IS_PURETEXT_VNODE(another_vnode) ? NULL : next_valid(vnode->nextNode);
	html_vnode_t *another_misplace_node = IS_PURETEXT_VNODE(vnode) ? NULL : next_valid(another_vnode->nextNode);

	while (misplace_node || another_misplace_node)
	{
		max_common_tree_info_t left_move_mct_info = MCT_INFO_INIT_VALUE;
		max_common_tree_info_t right_move_mct_info = MCT_INFO_INIT_VALUE;
		if (misplace_node)
		{
			left_move_mct_info = get_max_common_tree_info(misplace_node, another_vnode, is_trace);
		}
		if (another_misplace_node)
		{
			right_move_mct_info = get_max_common_tree_info(vnode, another_misplace_node, is_trace);
		}
		if (left_move_mct_info.align_node_cnt >= right_move_mct_info.align_node_cnt)
		{
			if (IS_SUBTREE_ALIGNED(left_move_mct_info))
			{
				if (is_trace)
				{
					html_vnode_visit_ex(vnode, visit_clear_trace_info, NULL, NULL);
				}
				vnode = misplace_node;
				return left_move_mct_info;
			}
		}
		else
		{
			if (IS_SUBTREE_ALIGNED(right_move_mct_info))
			{
				if (is_trace)
				{
					html_vnode_visit_ex(misplace_node, visit_clear_trace_info, NULL, NULL);
				}
				another_vnode = another_misplace_node;
				return right_move_mct_info;
			}
		}
		if (is_trace)
		{
			html_vnode_visit_ex(vnode, visit_clear_trace_info, NULL, NULL);
			html_vnode_visit_ex(misplace_node, visit_clear_trace_info, NULL, NULL);
		}
		if (misplace_node)
		{
			misplace_node = next_valid(misplace_node->nextNode);
		}
		if (another_misplace_node)
		{
			another_misplace_node = next_valid(another_misplace_node->nextNode);
		}
	}

	return MCT_INFO_INIT_VALUE;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static void record_align_info(html_vnode_t *vnode, html_vnode_t *align_vnode)
{
	html_vnode_t *top_vnode = vnode;
	vertical_misplace_align_vnode(vnode, align_vnode);
	vnode->struct_info->is_best_aligned_for_trace = 1;
	vnode->struct_info->align_vnode_for_trace = align_vnode;
	if (top_vnode != vnode)
	{
		html_vnode_t *upper = vnode;
		do
		{
			upper = upper->upperNode;
			upper->struct_info->is_best_aligned_for_trace = 1;
		} while (upper != top_vnode);
	}
}

/**
 * @brief true：对齐，判断两个节点在深度差1范围内是否对齐

 * @date 2011/06/27
 **/
static bool try_align_vnode(html_vnode_t * &vnode, html_vnode_t * & another_vnode)
{
	if (!is_align_vnode(vnode, another_vnode))
	{
		switch (vertical_misplace_align_vnode(vnode, another_vnode))
		{
		case 0:
			return false;
		case 1:
			return true;
		default:
			assert(0);
			break;
		}
	}
	return true;
}

/**
 * @brief vnode与another_vnode节点后裔中对齐节点个数

 * @date 2011/06/27
 **/
static max_common_tree_info_t get_max_common_tree_info(html_vnode_t *vnode, html_vnode_t *another_vnode, bool is_trace)
{
	max_common_tree_info_t mct_info = MCT_INFO_INIT_VALUE;
	if (!try_align_vnode(vnode, another_vnode))
	{
		return MCT_INFO_INIT_VALUE;
	}

	html_vnode_t *child = next_valid(vnode->firstChild);
	html_vnode_t *another_child = next_valid(another_vnode->firstChild);
	while (child && another_child)
	{
		max_common_tree_info_t child_mct_info = get_max_common_tree_info(child, another_child, is_trace);
		if (!IS_SUBTREE_ALIGNED(child_mct_info))
		{
			child_mct_info = misplace_align_node(child, another_child, is_trace);
		}

		if (is_trace && IS_SUBTREE_ALIGNED(child_mct_info))
		{
			record_align_info(child, another_child);
		}

		mct_info.align_node_cnt += child_mct_info.align_node_cnt;
		if (child_mct_info.similar_depth > mct_info.similar_depth)
		{
			mct_info.similar_depth = child_mct_info.similar_depth;
		}

		child = next_valid(child->nextNode);
		another_child = next_valid(another_child->nextNode);
	}

	mct_info.align_node_cnt++;
	mct_info.similar_depth++;

	return mct_info;
}

typedef struct
{
	COMMON_TREE_TRAVEL_FUNC start_travel_common_tree; /**< */
	COMMON_TREE_TRAVEL_FUNC finish_travel_common_tree; /**< */
	void *data; /**< */
} common_tree_travel_pack_t;

static int start_visit_for_common_tree(html_vnode_t *vnode, void *data)
{
	common_tree_travel_pack_t *ctt = (common_tree_travel_pack_t *) data;

	vstruct_info_t *sp_node = vnode->struct_info;

	if (sp_node == NULL || !sp_node->is_best_aligned_for_trace)
	{
		return VISIT_SKIP_CHILD;
	}

	if (sp_node && sp_node->align_vnode_for_trace && ctt->start_travel_common_tree)
	{
		return ctt->start_travel_common_tree(vnode, sp_node->align_vnode_for_trace, ctt->data);
	}

	return VISIT_NORMAL;
}

static int finish_visit_for_common_tree(html_vnode_t *vnode, void *data)
{
	common_tree_travel_pack_t *ctt = (common_tree_travel_pack_t *) data;

	if (ctt->finish_travel_common_tree)
	{
		vstruct_info_t *sp_node = vnode->struct_info;
		if (sp_node && sp_node->is_best_aligned_for_trace && sp_node->align_vnode_for_trace)
		{
			return ctt->finish_travel_common_tree(vnode, sp_node->align_vnode_for_trace, ctt->data);
		}
	}

	return VISIT_NORMAL;
}

typedef struct
{
	int align_percent; /**<  */
	int node_cnt_sum; /**< */
	int align_node_cnt; /**< */
	int similar_depth; /**<  */
} similar_value_t;

static const similar_value_t SIMILAR_INIT_VALUE =
{ 0, 0, 0, 0 };

/**
 * @brief 计算两个节点的相似度

 * @date 2011/06/27
 **/
static similar_value_t get_html_tree_similarity(html_vnode_t *vnode, html_vnode_t *another_vnode)
{
	if (IS_PURETEXT_VNODE(vnode) || IS_PURETEXT_VNODE(another_vnode))
	{
		if (vnode->textSize != another_vnode->textSize)
		{
			return SIMILAR_INIT_VALUE;
		}
	}

	max_common_tree_info_t mct_info = get_max_common_tree_info(vnode, another_vnode, false);

	similar_value_t svalue = SIMILAR_INIT_VALUE;

	svalue.node_cnt_sum = vnode->struct_info->valid_node_num + another_vnode->struct_info->valid_node_num;
	if (svalue.node_cnt_sum <= 0)
	{
		goto OUT;
	}
	svalue.align_percent = 2 * MAX_ALIGN_PERCENT * mct_info.align_node_cnt / svalue.node_cnt_sum;
	svalue.align_node_cnt = mct_info.align_node_cnt;
	svalue.similar_depth = mct_info.similar_depth;

	if (svalue.align_percent > MAX_ALIGN_PERCENT)
	{
		svalue.align_percent = MAX_ALIGN_PERCENT;
	}

	OUT: return svalue;
}

/**
 * @brief 该节点为无效、空白节点、宽度或高度<0,忽略该节点，返回true

 * @date 2011/06/27
 **/
inline bool is_ignore_vnode(html_vnode_t *vnode)
{
	if (!vnode->isValid)
	{
		return true;
	}
	if (IS_PURETEXT_VNODE(vnode) && is_space_text(vnode->hpNode->html_tag.text))
	{
		return true;
	}
	if (vnode->wx <= 0 || vnode->hx <= 0)
	{
		return true;
	}

	return false;
}

/**
 * @brief
 */
typedef struct
{
	int align_percent; /**<  */
	int mean; /**< */
	int stddev; /**< */
	int similar_num; /**< */
	int sign; /**< */
} similar_info_t;

static const int N_SIMILAR_TYPE_LIMIT = 32; /**<  */
static const int N_MAX_SIGN_NUM = 64; /**<  */

/**
 * @brief
 */
typedef struct
{
	similar_info_t similar_info[N_SIMILAR_TYPE_LIMIT]; /**<  */
	int sign_to_index[N_MAX_SIGN_NUM]; /**<  */
	int type_num; /**< */
	int sign_num; /**< */
} similar_info_table_t;

static void similar_info_table_clean(similar_info_table_t *sinfo_table)
{
	sinfo_table->type_num = 0;
	sinfo_table->sign_num = 0;
}

/**<  */
#define SWAP(i,j)	do{\
	i^=j;\
	j^=i;\
	i^=j;\
}while(0);

static void similar_info_add(similar_info_t *sinfo_to_be_add, similar_info_t *sinfo_add)
{
	sinfo_to_be_add->align_percent += sinfo_add->align_percent;
	sinfo_to_be_add->similar_num += sinfo_add->similar_num;
	sinfo_to_be_add->mean += sinfo_add->mean;
	sinfo_to_be_add->stddev += sinfo_add->stddev;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static similar_info_t *similar_info_table_merge(similar_info_table_t *sinfo_table, int sign, int next_sign)
{
	int small_index = sinfo_table->sign_to_index[sign];
	int big_index = sinfo_table->sign_to_index[next_sign];
	if (small_index > big_index)
	{
		SWAP(small_index, big_index);
	}

	similar_info_t *sinfo = sinfo_table->similar_info;

	similar_info_add(&sinfo[small_index], &sinfo[big_index]);

	for (int i = big_index; i < sinfo_table->type_num - 1; i++)
	{
		sinfo[i] = sinfo[i + 1];
		sinfo_table->sign_to_index[sinfo[i].sign] = i;
	}

	sinfo_table->type_num--;

	return &sinfo[small_index];
}

/**
 * @brief

 * @date 2011/06/27
 **/
static void change_prev_node_sign(html_vnode_t *vnode, int dest_sign, int sign_to_change)
{
	for (html_vnode_t *prev = vnode->prevNode; prev; prev = prev->prevNode)
	{
		if (IS_REPEAT_STRUCT_CHILD(prev->property))
		{
			if (prev->struct_info->similar_sign == sign_to_change)
			{
				prev->struct_info->similar_sign = dest_sign;
			}
		}
	}
}

/**
 * @brief

 * @date 2011/06/27
 **/
static void mark_sibling_similar(html_vnode_t *vnode, similar_info_t *sinfo, int index, int similar_value)
{
	html_vnode_t *prev_similar_vnode = NULL;
	vstruct_info_t *prev_similar_sp_node = NULL;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (is_ignore_vnode(child))
		{
			continue;
		}
		vstruct_info_t *sp_node = child->struct_info;
		if (sp_node->similar_sign == sinfo[index].sign)
		{
			SET_REPEAT_STRUCT_CHILD(child->property);
			sp_node->is_repeat_with_sibling = 1;
			sp_node->align_percent = sinfo[index].align_percent;
			sp_node->similar_value = similar_value;
			sp_node->repeat_num = sinfo[index].similar_num + 1;
			sp_node->prev_similar_vnode = prev_similar_vnode;
			if (prev_similar_sp_node)
			{
				prev_similar_sp_node->next_similar_vnode = child;
			}
			sp_node->similar_sign = index; /**change similar sign equal to index*/
			prev_similar_vnode = child;
			prev_similar_sp_node = sp_node;
		}
	}
}

static int pre_visit_for_repeat_struct(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
	{
		return VISIT_SKIP_CHILD;
	}

	return VISIT_NORMAL;
}

inline int min_similar_align_percent(int mean, int stddev)
{
	int min_per = mean + stddev / 2;
	if (min_per > MAX_ALIGN_PERCENT)
	{
		return MAX_ALIGN_PERCENT;
	}
	return min_per;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static void get_next_best_similar_sibling(similar_info_t & best_similar_info, html_vnode_t * & best_align_vnode, html_vnode_t *vnode)
{
	best_similar_info.align_percent = 0;
	best_similar_info.mean = 0;
	best_similar_info.stddev = 0;
	best_similar_info.similar_num = 0;

	for (html_vnode_t *next = vnode->nextNode; next; next = next->nextNode)
	{
		if (is_ignore_vnode(next))
		{
			continue;
		}
		similar_value_t svalue = get_html_tree_similarity(vnode, next);
		if (svalue.align_percent <= 0)
		{
			continue;
		}
		int mean = get_align_percent_mean(svalue.node_cnt_sum);
		int stddev = get_align_percent_std_dev(svalue.node_cnt_sum);
		if (svalue.align_percent >= min_similar_align_percent(mean, stddev) && svalue.align_percent > best_similar_info.align_percent)
		{
			best_similar_info.align_percent = svalue.align_percent;
			best_similar_info.mean = mean;
			best_similar_info.stddev = stddev;
			best_align_vnode = next;
		}

		if (MAX_ALIGN_PERCENT == svalue.align_percent)
		{
			break;
		}
	}
}

/**
 * @brief

 * @date 2011/06/27
 **/
static similar_info_t *request_new_similar_info(similar_info_table_t *sinfo_table)
{
	similar_info_t *sinfo = NULL;
	if (sinfo_table->type_num < N_SIMILAR_TYPE_LIMIT && sinfo_table->sign_num < N_MAX_SIGN_NUM)
	{
		sinfo = &(sinfo_table->similar_info[sinfo_table->type_num]);
		sinfo->align_percent = 0;
		sinfo->mean = 0;
		sinfo->stddev = 0;
		sinfo->similar_num = 0;
		sinfo->sign = sinfo_table->sign_num++;
		sinfo_table->sign_to_index[sinfo->sign] = sinfo_table->type_num;
		sinfo_table->type_num++;
	}

	return sinfo;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int collect_similar_info(similar_info_table_t *sinfo_table, html_vnode_t *similar_vnode, html_vnode_t *next_similar_vnode, similar_info_t *new_sinfo)
{
	similar_info_t *sinfo = NULL;
	vstruct_info_t *sp_node = similar_vnode->struct_info;
	vstruct_info_t *next_sp_node = next_similar_vnode->struct_info;

	if (!IS_REPEAT_STRUCT_CHILD(similar_vnode->property) && !IS_REPEAT_STRUCT_CHILD(next_similar_vnode->property))
	{
		//New
		sinfo = request_new_similar_info(sinfo_table);
		if (NULL == sinfo)
		{
			goto FAIL;
		}
		sp_node->similar_sign = sinfo->sign;
		next_sp_node->similar_sign = sinfo->sign;
	}
	else if (IS_REPEAT_STRUCT_CHILD(similar_vnode->property) && IS_REPEAT_STRUCT_CHILD(next_similar_vnode->property))
	{
		assert(sp_node->similar_sign != next_sp_node->similar_sign);
		if (sp_node->similar_sign < next_sp_node->similar_sign)
		{
			sinfo = similar_info_table_merge(sinfo_table, sp_node->similar_sign, next_sp_node->similar_sign);
			change_prev_node_sign(similar_vnode, sp_node->similar_sign, next_sp_node->similar_sign);
			next_sp_node->similar_sign = sp_node->similar_sign;
		}
		else
		{
			sinfo = similar_info_table_merge(sinfo_table, next_sp_node->similar_sign, sp_node->similar_sign);
			change_prev_node_sign(similar_vnode, next_sp_node->similar_sign, sp_node->similar_sign);
			sp_node->similar_sign = next_sp_node->similar_sign;
		}
	}
	else if (IS_REPEAT_STRUCT_CHILD(similar_vnode->property))
	{
		sinfo = &sinfo_table->similar_info[sinfo_table->sign_to_index[sp_node->similar_sign]];
		next_sp_node->similar_sign = sp_node->similar_sign;
	}
	else if (IS_REPEAT_STRUCT_CHILD(next_similar_vnode->property))
	{
		sinfo = &sinfo_table->similar_info[sinfo_table->sign_to_index[next_sp_node->similar_sign]];
		sp_node->similar_sign = next_sp_node->similar_sign;
	}

	similar_info_add(sinfo, new_sinfo);

	return 1;

	FAIL: return -1;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int get_repeat_probability(int percent, int mean, int stddev)
{
	/**
	 * X:
	 * Y:
	 * X	Y
	 * ----------
	 * 0	64.9
	 * 1	81
	 * 3	86
	 * 5	89.6
	 * 15	93.9
	 * 20	92.5
	 * 30	96.5
	 * -----------
	 * Y = (a * b + c * pow(x,d)) / (b + pow(X,d))
	 *  a = 64.902565; b = 1.2265201; double c = 100.46079; double d = 0.59962087.
	 */
	static const int PRE_COMPUTED_VAL[] =
	{ 75, 80, 84, 86, 88, 89, 89, 90, 91, 91, 92, 92, 92, 93, 93, 93, 93, 93, 94, 94, 94, 94, 94, 94, 94, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 97, 97, 97 };

	int index = 10 * (percent - mean - stddev) / stddev;

	if (mean + stddev > MAX_ALIGN_PERCENT)
	{ /***/
		if (index > -5)
		{
			return 70;
		}
		if (index > -10)
		{
			return 65;
		}
	}

	if ((unsigned) index < sizeof(PRE_COMPUTED_VAL) / sizeof(PRE_COMPUTED_VAL[0]))
	{
		return PRE_COMPUTED_VAL[index];
	}

	// if negative or too large, simplely give these values
	if (index < 0)
	{
		return 55;
	}

	return 99;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int get_single_type_cumulate_similarity(similar_info_t *sinfo)
{
	int repeat_num = sinfo->similar_num;
	int ave_mean = sinfo->mean / repeat_num;
	int ave_stddev = (int) (sinfo->stddev / (repeat_num * sqrtf(repeat_num)));
	if (ave_stddev < 1)
	{
		ave_stddev = 1;
	}
	int ave_per = sinfo->align_percent / repeat_num;
	sinfo->align_percent = ave_per;
	int cur_similar_value = get_repeat_probability(ave_per, ave_mean, ave_stddev);
	if (cur_similar_value > MAX_ALIGN_PERCENT)
	{
		cur_similar_value = MAX_ALIGN_PERCENT;
	}

	return cur_similar_value;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int compute_cumulate_similarity(html_vnode_t *vnode, similar_info_table_t *sinfo_table)
{
	int cumulate_similar_value = 0;
	int similar_num = 0;

	similar_info_t *sinfo = sinfo_table->similar_info;

	for (int i = 0; i < sinfo_table->type_num; i++)
	{
		int cur_similar_value = get_single_type_cumulate_similarity(sinfo + i);

		if (cur_similar_value >= MIN_REPEAT_VALUE)
		{
			assert(i <= sinfo[i].sign);
			mark_sibling_similar(vnode, sinfo, i, cur_similar_value);
		}

		cur_similar_value *= (sinfo[i].similar_num + 1);
		cumulate_similar_value += cur_similar_value;
		similar_num += sinfo[i].similar_num + 1;
	}

	if (similar_num > 0)
	{
		cumulate_similar_value /= similar_num;
	}

	return cumulate_similar_value;
}

static int post_visit_for_repeat_struct(html_vnode_t *vnode, void *data)
{
	similar_info_table_t sinfo_table;
	similar_info_table_clean(&sinfo_table);

	int tot_area = 0;
	int simi_area = 0;

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (is_ignore_vnode(child))
		{
			continue;
		}
		tot_area += child->wx * child->hx;
		similar_info_t best_similar_info;
		html_vnode_t *best_align_vnode = NULL;

		get_next_best_similar_sibling(best_similar_info, best_align_vnode, child);

		if (best_align_vnode != NULL)
		{
			best_similar_info.similar_num = 1;

			int ret = collect_similar_info(&sinfo_table, child, best_align_vnode, &best_similar_info);
			if (-1 == ret)
			{
				continue;
			}

			if (!IS_REPEAT_STRUCT_CHILD(child->property))
			{
				simi_area += child->wx * child->hx;
			}
			if (!IS_REPEAT_STRUCT_CHILD(best_align_vnode->property))
			{
				simi_area += best_align_vnode->wx * best_align_vnode->hx;
			}

			SET_REPEAT_STRUCT_CHILD(child->property);
			SET_REPEAT_STRUCT_CHILD(best_align_vnode->property);
		}
	}

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		CLEAR_REPEAT_STRUCT_CHILD(child->property);
	}

	int self_similar_value = compute_cumulate_similarity(vnode, &sinfo_table);

	if (tot_area > 0 && self_similar_value > 0)
	{
		self_similar_value *= (int) sqrtf(10000 * (float) simi_area / tot_area);
		self_similar_value /= 100;
	}

	if (self_similar_value >= MIN_REPEAT_VALUE)
	{
		SET_REPEAT_STRUCT_PARENT(vnode->property);
		vnode->struct_info->is_self_repeat = 1;
		vnode->struct_info->self_similar_value = self_similar_value;
		vnode->struct_info->repeat_type_num = sinfo_table.type_num;
	}

	return VISIT_NORMAL;
}

static vstruct_info_t *struct_info_node_create(html_vtree_t *html_vtree)
{
	vstruct_info_t *vsi_node = (vstruct_info_t *) nodepool_get(&html_vtree->struct_np);
	if (vsi_node)
	{
		memset(vsi_node, 0, sizeof(vstruct_info_t));
		vsi_node->similar_sign = -1;
	}
	return vsi_node;
}

/**
 * @brief 计算vnode节点最大深度和有效后裔节点数量（包括自己）

 * @date 2011/06/27
 **/
static int visit_for_prev_info(html_vnode_t *vnode, void *data)
{
	html_vtree_t *html_vtree = (html_vtree_t *) data;
	if (NULL == vnode->struct_info)
	{
		vnode->struct_info = struct_info_node_create(html_vtree);
		if (NULL == vnode->struct_info)
		{
			goto ERR;
		}
	}

	if (!vnode->isValid)
	{
		return VISIT_NORMAL;
	}

	if (!vnode->firstChild)
	{
		vnode->struct_info->valid_leaf_num = 1;
	}

	if (IS_PURETEXT_VNODE(vnode) && is_space_text(vnode->hpNode->html_tag.text))
	{
		return VISIT_NORMAL;
	}

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
		{
			continue;
		}
		vnode->struct_info->valid_child_num++;
		vstruct_info_t *child_vsi = child->struct_info;
		if (child_vsi->max_depth > vnode->struct_info->max_depth)
		{
			vnode->struct_info->max_depth = child_vsi->max_depth;
		}
		vnode->struct_info->valid_node_num += child_vsi->valid_node_num;
		vnode->struct_info->hr_num += child_vsi->hr_num;
		vnode->struct_info->interaction_tag_num += child_vsi->interaction_tag_num;
		vnode->struct_info->valid_leaf_num += child->struct_info->valid_leaf_num;
	}

	vnode->struct_info->max_depth++;
	vnode->struct_info->valid_node_num++;
	if(vnode->hpNode->html_tag.tag_type == TAG_HR)
	{
		vnode->struct_info->hr_num++;
	}
	if(interaction_tag_types[vnode->hpNode->html_tag.tag_type])
	{
		vnode->struct_info->interaction_tag_num++;
	}

	return VISIT_NORMAL;
	ERR: return VISIT_ERROR;
}

/**
 * @brief 计算每个vnode节点最大深度和有效后裔节点数量（包括自己）

 * @date 2011/06/27
 **/
static int vhtml_struct_add_normal_info(html_vtree_t *html_vtree)
{
	if (!html_vtree->normal_struct_info_added)
	{
		if (!html_vtree->struct_np_inited && vhtml_struct_prof_init(html_vtree) == -1)
		{
			goto ERR;
		}
		vhtml_struct_prof_reset(html_vtree);

		if (html_vnode_visit_ex(html_vtree->root, NULL, visit_for_prev_info, html_vtree) == VISIT_ERROR)
		{
			goto ERR;
		}
		html_vtree->normal_struct_info_added = 1;
	}

	return 1;
	ERR: return -1;
}

/**
 * @brief

 * @date 2011/06/27
 **/
static int vhtml_struct_add_repeat_struct_info(html_vtree_t *html_vtree)
{
	if (!html_vtree->repeat_struct_info_added)
	{
		if (html_vnode_visit_ex(html_vtree->root, pre_visit_for_repeat_struct, post_visit_for_repeat_struct, html_vtree) == VISIT_ERROR)
		{
			goto ERR;
		}
		html_vtree->repeat_struct_info_added = 1;
	}

	return 1;
	ERR: return -1;
}

/**
 * @brief

 * @date 2011/06/27
 **/
int vhtml_struct_prof(html_vtree_t *html_vtree, unsigned int flag)
{
	if (flag & (ADD_NORMAL_STRUCT_INFO | ADD_REPEAT_STRUCT_INFO))
	{
		if (vhtml_struct_add_normal_info(html_vtree) == -1)
		{
			goto ERR;
		}
	}

	if (flag & ADD_REPEAT_STRUCT_INFO)
	{
		if (vhtml_struct_add_repeat_struct_info(html_vtree) == -1)
		{
			goto ERR;
		}
	}

	return 1;
	ERR: return -1;
}

/**
 * @brief .

 * @date 2011/06/27
 **/
max_common_tree_info_t travel_max_common_tree(html_vnode_t *vnode, html_vnode_t *another_vnode, html_vtree_t *vtree, html_vtree_t *another_vtree, COMMON_TREE_TRAVEL_FUNC start_travel_common_tree, COMMON_TREE_TRAVEL_FUNC finish_travel_common_tree, void *data)
{
	if (!vtree->normal_struct_info_added)
	{
		vhtml_struct_add_normal_info(vtree);
	}
	if (!another_vtree->normal_struct_info_added)
	{
		vhtml_struct_add_normal_info(another_vtree);
	}

	html_vnode_visit_ex(vnode, visit_clear_trace_info, NULL, NULL);

	max_common_tree_info_t mct_info = get_max_common_tree_info(vnode, another_vnode, true);

	if (IS_SUBTREE_ALIGNED(mct_info))
	{
		record_align_info(vnode, another_vnode);
	}

	common_tree_travel_pack_t ctt;
	ctt.start_travel_common_tree = start_travel_common_tree;
	ctt.finish_travel_common_tree = finish_travel_common_tree;
	ctt.data = data;

	html_vnode_visit_ex(vnode, start_visit_for_common_tree, finish_visit_for_common_tree, &ctt);

	return mct_info;
}
