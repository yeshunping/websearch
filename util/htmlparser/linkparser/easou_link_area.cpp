/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_area.h,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_link_area.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 分块统计信息
 **/

#include <stdio.h>
#include <string.h>

#include "easou_ahtml_area.h"
#include "easou_mark_parser.h"
#include "easou_html_attr.h"

#include "easou_link_area.h"
#include "easou_link_timematch.h"
#include "easou_link_tree.h"
#include "easou_link_common.h"

static const int MAX_STR_LEN = 128;

/**
 * @brief short description 评论块文字特征
 */
typedef struct _cmt_point_t
{
	char string[MAX_STR_LEN]; /**<  字符串      */
	int value; /**<  分数      */
	int limit_len; /**<  限制长度      */
} cmt_point_t;

/**
 * @brief short description 评论文字分数映射表
 */
static const cmt_point_t g_comment_text_point[] =
{
{ "回复(", -1, 20 },
{ "回复（", -1, 20 },
{ "引用（", -1, 20 },
{ "Says:", 1, 20 },
{ "回复", 1, 20 },
{ "reply", 1, 20 },
{ "Reply", 1, 20 },
{ "发表评论", 2, 20 },
{ "评论(", -1, 20 },
{ "阅读( ", -1, 20 },
{ "阅读(", -1, 20 },
{ "评论：", 0, 30 },
{ "评论", 1, 30 },
{ "Responses", 1, 50 },
{ "Response", 1, 50 },
{ "请留言", 2, 20 },
{ "留言", 1, 20 },
{ "阅读全文", -3, 20 },
{ "访问量", -1, 20 },
{ "日志数", -1, 20 },
{ "数据统计", -1, 20 },
{ "comment", 1, 20 },
{ "Comment", 1, 20 },
{ "楼", 1, 5 },
{ "查看全文", -1, 20 },
{ "收藏", -1, 10 },
{ "分享", -1, 10 },
{ "标签", -1, 10 },
{ "TAG", -1, 10 },
{ "分类", -1, 10 },
{ "查看次数", -1, 10 },
{ "查看引用通告", -1, 10 },
{ "回应", -1, 10 },
{ "回应 (", -1, 10 },
{ "引用 (", -1, 10 },
{ "上一篇", -1, 10 },
{ "下一篇", -1, 10 },
{ "订阅", -1, 10 },
{ "引用通告地址", -1, 10 } };

static unsigned int g_comment_text_point_len = sizeof(g_comment_text_point) / sizeof(g_comment_text_point[0]);

/**
 * @brief 获得该文字的评论分数
 */
static int get_cmt_point(const char *text)
{
	if (!text)
	{
		return 0;
	}
	for (unsigned int i = 0; i < g_comment_text_point_len; i++)
	{
		const char *p =strstr(text, g_comment_text_point[i].string);
		if (get_chword_len(text) < g_comment_text_point[i].limit_len && p != NULL)
		{
			return g_comment_text_point[i].value;
		}
	}
	return 0;
}

/**
 * @brief 重置area_info
 */
static void reset_area_info(lt_area_info_t *pai)
{
	pai->time_nodes_cnt = 0;
	memset(pai->time_nodes, 0, sizeof(pai->time_nodes));
}

/**
 * @brief short description 分块访问结构
 */
typedef struct
{
	unsigned int route_sign; /**<  路径签名      */
	lt_area_info_t *pai; /**<  带获取的分块信息      */
	const char *url; /**<  网页url      */
	timematch_pack_t *ptime_match; /**<  时间判断字典      */
} area_visit_t;

/**
 * @brief 前序遍历
 */
static int start_for_area_info(html_area_t *area, void *data)
{
	area_visit_t *pav = (area_visit_t *) data;

	pav->route_sign += area->begin->hpNode->html_tag.tag_type;
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 是否外链
 */
static int is_outer_link(char *url, const char *g_url)
{
	char s[UL_MAX_SITE_LEN];
	char t[UL_MAX_SITE_LEN];

	char s2[UL_MAX_SITE_LEN];
	char t2[UL_MAX_SITE_LEN];
	if (easou_get_site(url, s) == NULL)
		return 0;
	if (easou_fetch_trunk(s, t, UL_MAX_SITE_LEN) != 1)
		return 0;
	easou_trans2lower(t, t);

	if (easou_get_site(g_url, s2) == NULL)
		return 0;
	if (easou_fetch_trunk(s2, t2, UL_MAX_SITE_LEN) != 1)
		return 0;
	easou_trans2lower(t2, t2);

	if (strcmp(t, t2) != 0)
		return 1;
	else
		return 0;
}

/**
 * @brief 计算当前分块中时间结点间隔最大文字长度
 */
static unsigned int cacu_max_time_gap_len(area_node_info *pcur_node)
{
	area_node_inner_info * pinner = &pcur_node->inner;
	if (pinner->time_node_cnt == 0)
		return pcur_node->notanchor_text_len + pcur_node->anchor_len;

	unsigned int max_gap_len = pinner->ptime_nodes[0]->last_gap;

	for (unsigned int i = 0; i < pinner->time_node_cnt; i++)
	{
		if (max_gap_len < pinner->ptime_nodes[i]->next_gap)
		{
			max_gap_len = pinner->ptime_nodes[i]->next_gap;
		}
	}

	return max_gap_len;
}

/**
 * @brief 计算当前分块按路径深度分类的最大时间个数，排除时间链接的干扰
 */
unsigned int cacu_max_depth_time_count_with_linktime_filter(area_node_info *pcur_node)
{
	unsigned int route_signs[MAX_AREA_TIME_NUM];
	unsigned int route_signs_cnt[MAX_AREA_TIME_NUM];
	bool route_signs_has_cont[MAX_AREA_TIME_NUM];
	unsigned int route_sign_num = 0;

	area_node_inner_info * pinner = &pcur_node->inner;
	for (unsigned int i = 0; i < pinner->time_node_cnt; i++)
	{
		int find = 0;
		for (unsigned int j = 0; j < route_sign_num; j++)
		{
			if (pinner->ptime_nodes[i]->route_sign == route_signs[j])
			{
				find = 1;
				route_signs_cnt[j]++;
				if (route_signs_cnt[j] < 4)
				{
					route_signs_has_cont[j] |= pinner->ptime_nodes[i]->next_has_cont;
				}
				break;
			}
		}
		if (!find && route_sign_num < MAX_AREA_TIME_NUM)
		{
			route_signs[route_sign_num] = pinner->ptime_nodes[i]->route_sign;
			route_signs_cnt[route_sign_num] = 1;
			route_signs_has_cont[route_sign_num] = false;
			route_sign_num++;
		}
	}

	unsigned int max = 0;
	for (unsigned int i = 0; i < route_sign_num; i++)
	{
		if (route_signs_has_cont[i] || route_signs_cnt[i] <= 2)
		{
			if (max < route_signs_cnt[i])
				max = route_signs_cnt[i];
		}
	}

	return max;
}

/**
 * @brief 计算当前分块按路径分类的最大时间个数
 */
unsigned int cacu_max_depth_time_count(area_node_info *pcur_node)
{
	unsigned int route_signs[MAX_AREA_TIME_NUM];
	unsigned int route_signs_cnt[MAX_AREA_TIME_NUM];
	unsigned int route_sign_num = 0;

	area_node_inner_info * pinner = &pcur_node->inner;
	for (unsigned int i = 0; i < pinner->time_node_cnt; i++)
	{
		int find = 0;
		for (unsigned int j = 0; j < route_sign_num; j++)
		{
			if (pinner->ptime_nodes[i]->route_sign == route_signs[j])
			{
				find = 1;
				route_signs_cnt[j]++;
				break;
			}
		}
		if (!find && route_sign_num < MAX_AREA_TIME_NUM)
		{
			route_signs[route_sign_num] = pinner->ptime_nodes[i]->route_sign;
			route_signs_cnt[route_sign_num] = 1;
			route_sign_num++;
		}
	}

	unsigned int max = 0;
	for (unsigned int i = 0; i < route_sign_num; i++)
	{
		if (max < route_signs_cnt[i])
			max = route_signs_cnt[i];
	}

	return max;
}

/**
 * @brief 计算一个结点的路径签名
 */
static unsigned int cacu_sign(html_vnode_t *vnode)
{
	unsigned int sign = 0;
	html_vnode_t *iter_vnode = vnode;
	while (iter_vnode)
	{
		sign += get_tag_type_from_vnode(vnode);
		iter_vnode = iter_vnode->upperNode;
	}
	return sign;
}

/**
 * @brief 填充分块信息，该分块没有子分块了
 */
static void fill_area_info_with_subarea(html_area_t *area, void *data)
{
	area_visit_t *pav = (area_visit_t *) data;
	lt_area_info_t *pai = pav->pai;
	area_node_info *pcur_node = get_area_node(pai, area);
	if (!pcur_node)
		return;

	memset(pcur_node, 0, sizeof(area_node_info));

	unsigned int area_head_gap = 0;
	bool area_head_has_cont = false;
	time_node_info *time_node = NULL;

	//根据子分块，填充当前分块的属性
	for (html_area_t *subArea = area->subArea; subArea; subArea = subArea->nextArea)
	{
		area_node_info *psub_node = get_area_node(pai, subArea);
		if (!psub_node)
			return;
		pcur_node->anchor_len += psub_node->anchor_len;
		pcur_node->inner_link_cnt += psub_node->inner_link_cnt;
		pcur_node->outer_link_cnt += psub_node->outer_link_cnt;
		pcur_node->notanchor_text_len += psub_node->notanchor_text_len;
		pcur_node->inner_anchor_len += psub_node->inner_anchor_len;
		pcur_node->anchor_cnt += psub_node->anchor_cnt;
		pcur_node->binfo.comment_area_cnt += psub_node->binfo.comment_area_cnt;
		pcur_node->binfo.comment_point += psub_node->binfo.comment_point;

		//更新时间的间隔文字长度
		if (psub_node->inner.time_node_cnt == 0)
		{
			if (time_node == NULL)
			{
				area_head_gap += psub_node->notanchor_text_len + psub_node->anchor_len;
				if (psub_node->notanchor_text_len > 0)
					area_head_has_cont = true;
			}
			else
			{
				if (psub_node->notanchor_text_len > 0)
					time_node->next_has_cont = true;
				time_node->next_gap += psub_node->notanchor_text_len + psub_node->anchor_len;
			}
		}
		else
		{
			if (time_node == NULL)
			{
				time_node = psub_node->inner.ptime_nodes[0];
				time_node->last_gap += area_head_gap;
				time_node->last_has_cont = area_head_has_cont | time_node->last_has_cont;
			}
			else
			{
				time_node->next_has_cont = psub_node->inner.ptime_nodes[0]->last_has_cont | time_node->next_has_cont;
				psub_node->inner.ptime_nodes[0]->last_has_cont = time_node->next_has_cont;
				time_node->next_gap += psub_node->inner.ptime_nodes[0]->last_gap;
				psub_node->inner.ptime_nodes[0]->last_gap = time_node->next_gap;
			}
			time_node = psub_node->inner.ptime_nodes[psub_node->inner.time_node_cnt - 1];
		}

		//继承子分块的时间
		for (unsigned int i = 0; i < psub_node->inner.time_node_cnt; i++)
		{
			if (pcur_node->inner.time_node_cnt < MAX_AREA_TIME_NUM)
			{
				pcur_node->inner.ptime_nodes[pcur_node->inner.time_node_cnt++] = psub_node->inner.ptime_nodes[i];
			}
		}
	}

	pcur_node->max_time_gap_len = cacu_max_time_gap_len(pcur_node);
}

/**
 * @brief short description 结点访问结构
 */
typedef struct
{
	int text_len; /**<  文字长度      */
	int anchor_len; /**<  链接长度      */
	int anchor_count; /**<  链接个数      */
	int inner_link_count; /**<  内链个数      */
	int inner_anchor_len; /**<  内链anchor长度      */
	const char *url; /**<  网页url      */
	bool is_in_inner_anchor; /**<  是否内链      */
	bool is_in_anchor; /**<  是否链接      */
	timematch_pack_t *ptime_match; /**<  时间判断词典      */
	html_vnode_t *time_vnode; /**< 时间结点      */
	int last_gap_len; /**<  时间前文字长度      */
	int next_gap_len; /**<  时间后文字长度      */
	unsigned int route_sign; /**<  路径签名      */
	unsigned int time_sign; /**<  时间路径签名      */
	int comment_point; /**<  评论分数      */
	lt_area_info_t *parea_info; /**<  分块信息      */
	bool next_has_cont; /**<  时间后是否有文字  */
	bool last_has_cont; /**<  时间前是否有文字      */
} node_visit_t;

/**
 * @brief 前序遍历分块中的结点，获取统计信息
 */
static int start_visit_for_single_area_info(html_vnode_t *vnode, void *data)
{
	node_visit_t *pnv = (node_visit_t *) data;
	html_tag_t *ptag = &vnode->hpNode->html_tag;

	pnv->route_sign += ptag->tag_type;
	if (ptag->tag_type == TAG_A)
	{
		char *href = get_attribute_value(ptag, ATTR_HREF);
		if (href && is_outer_link(href, pnv->url) != 1)
		{
			pnv->is_in_inner_anchor = true;
		}
		pnv->is_in_anchor = true;
	}

	if (ptag->tag_type == TAG_PURETEXT)
	{
		int len = get_valid_text_len(vnode);

		if (ptag->text && strlen(ptag->text) < 150 && hastime(pnv->ptime_match, ptag->text, 0))
		{
			pnv->time_vnode = vnode;
			pnv->time_sign = pnv->route_sign;
		}
		else if (pnv->time_vnode)
		{
			if (pnv->is_in_anchor == false && len > 0)
				pnv->next_has_cont = true;
			pnv->next_gap_len += len;
		}
		else
		{
			if (pnv->is_in_anchor == false && len > 0)
				pnv->last_has_cont = true;
			pnv->last_gap_len += len;
		}
		pnv->comment_point += get_cmt_point(ptag->text);
		if (pnv->is_in_anchor)
		{
			if (pnv->is_in_inner_anchor)
			{
				pnv->inner_anchor_len += len;
				pnv->inner_link_count++;
			}
			pnv->anchor_len += len;
			pnv->anchor_count++;
		}
		else
		{
			pnv->text_len += len;
		}
	}
	return VISIT_NORMAL;
}

/**
 * @brief 后续遍历分块中的结点，更新状态
 */
static int finish_visit_for_single_area_info(html_vnode_t *vnode, void *data)
{
	node_visit_t *pnv = (node_visit_t *) data;
	html_tag_t *ptag = &vnode->hpNode->html_tag;
	pnv->route_sign -= ptag->tag_type;
	if (ptag->tag_type == TAG_A)
	{
		pnv->is_in_anchor = false;
		pnv->is_in_inner_anchor = false;
	}
	return VISIT_NORMAL;
}

/**
 * @brief 是否评论块
 */
static bool is_comment_area(html_vnode_t *vnode)
{
	int depth = 10;
	html_vnode_t *pnode = vnode->upperNode;
	while (pnode != NULL && depth-- > 0)
	{
		html_tag_t *tmptag = &pnode->hpNode->html_tag;
		char *id = get_attribute_value(tmptag, ATTR_ID);
		char * cla = get_attribute_value(tmptag, ATTR_CLASS);
		if (id && (strstr(id, "comment") != NULL || strstr(id, "Comment") != NULL))
		{
			return true;
		}
		if (cla && (strstr(cla, "comment") != NULL || strstr(cla, "Comment") != NULL))
		{
			return true;
		}
		pnode = pnode->upperNode;
	}
	return false;
}

/**
 * @brief 填充分块信息，通过结点解析
 */
static void fill_area_info_with_no_subarea(html_area_t *area, void *data)
{
	area_visit_t *pav = (area_visit_t *) data;
	lt_area_info_t *pai = pav->pai;
	area_node_info *pcur_node = get_area_node(pai, area);
	if (!pcur_node)
		return;

	memset(pcur_node, 0, sizeof(area_node_info));
	node_visit_t nv;
	memset(&nv, 0, sizeof(nv));
	nv.url = pav->url;
	nv.ptime_match = pav->ptime_match;
	nv.route_sign = pav->route_sign;
	nv.parea_info = pai;
	nv.next_has_cont = false;
	nv.last_has_cont = false;
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		html_vnode_visit(vnode, start_visit_for_single_area_info, finish_visit_for_single_area_info, &nv);
		if (vnode == area->end)
			break;
	}

	pcur_node->anchor_len = nv.anchor_len;
	pcur_node->inner_link_cnt = nv.inner_link_count;
	pcur_node->notanchor_text_len = nv.text_len;
	pcur_node->inner_anchor_len = nv.inner_anchor_len;
	pcur_node->anchor_cnt = nv.anchor_count;
	pcur_node->binfo.comment_point = nv.comment_point;

	//时间块
	if (nv.time_vnode && pai->time_nodes_cnt < MAX_TIME_NUM && pcur_node->inner.time_node_cnt < MAX_AREA_TIME_NUM)
	{
		//补充该块的信息
		pcur_node->time_cnt = 1;
		pcur_node->max_time_gap_len = 0;
		if (is_comment_area(nv.time_vnode))
			pcur_node->binfo.comment_area_cnt++;

		//填充时间信息
		time_node_info *ptime_node = &pai->time_nodes[pai->time_nodes_cnt++];
		pcur_node->inner.ptime_nodes[pcur_node->inner.time_node_cnt++] = ptime_node;

		ptime_node->vnode = nv.time_vnode;

		ptime_node->route_sign = cacu_sign(nv.time_vnode);
		ptime_node->last_gap = nv.last_gap_len;
		ptime_node->next_gap = nv.next_gap_len;
		ptime_node->next_has_cont = nv.next_has_cont;
		ptime_node->last_has_cont = nv.last_has_cont;
	}
	else
	{
		pcur_node->max_time_gap_len = pcur_node->notanchor_text_len + pcur_node->anchor_len;
	}
}

/**
 * @brief 后续遍历获取分块属性
 */
static int finish_for_area_info(html_area_t *area, void *data)
{
	if (area->subArea)
		fill_area_info_with_subarea(area, data);
	else
		fill_area_info_with_no_subarea(area, data);

	return AREA_VISIT_NORMAL;
}

/**
 * @brief 获取分块统计信息
 */
void get_area_info(area_tree_t *atree, lt_area_info_t *pai, const char *url, timematch_pack_t *ptimematch)
{
	reset_area_info(pai);
	area_visit_t av;
	av.route_sign = 0;
	av.pai = pai;
	av.url = url;
	av.ptime_match = ptimematch;

	areatree_visit(atree->root, start_for_area_info, finish_for_area_info, &av);
}
