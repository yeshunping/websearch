/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_blogre.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_blogre.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 	标记博客回复链接
 **/

#include "PageType.h"
#include "easou_link.h"
#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_link_timematch.h"
#include "easou_link_area.h"

#include <ctype.h>

static const int MIN_CMT_AREA_POINT = 10; /**<  评论块最低要求分数      */
static const int MAX_CAND_CMT_AREA_CNT = 500; /**<  候选评论块的个数      */
static const int MAX_CMT_AREA_CNT = 500; /**<  评论块的个数      */

/**
 * @brief short description blog 信息
 */
typedef struct
{
	lt_area_info_t *area_info; /**<  分块统计信息      */
	int page_width; /**<  页面宽度      */
	html_area_t *cand_cmt_areas[MAX_CAND_CMT_AREA_CNT]; /**<  候选评论块      */
	int cand_cmt_area_cnt; /**<  候选评论块的个数      */

	html_area_t *cmt_areas[MAX_CMT_AREA_CNT]; /**<  评论块      */
	int cmt_area_cnt; /**<  评论块的个数      */

	//临时变量
	int cur_max_cmt_point; /**<  当前最大的评论分数      */
	html_area_t *cur_max_cmt_piont_area; /**<  分数最高的块      */
} blog_re_t;

/**
 * @brief 计算分块的评论分数
 */
static int gen_final(area_node_info *pai, html_area_t *area, int page_width)
{
	int point = 0;
	int depth = area->depth;

	if (pai->binfo.comment_point <= 4 && pai->time_cnt == 0)
		return 0;

	int floor_count = pai->time_cnt;
	if (floor_count == 0)
		floor_count = pai->binfo.comment_point / 2;

	int avg_anchor_len = 0;
	int avg_anchor_count = 0;
	int avg_width = 0;
	int avg_height = 0;
	int avg_len = 0;
	if (pai->time_cnt == 1)
		pai->binfo.comment_point = pai->binfo.comment_point > 1 ? 1 : pai->binfo.comment_point;

	if (pai->time_cnt == 0)
		pai->binfo.comment_point = 0;
	else
	{
		avg_anchor_len = pai->inner_anchor_len / pai->time_cnt;
		avg_anchor_count = pai->inner_link_cnt / pai->time_cnt;
		avg_width = area->area_info.width / pai->time_cnt;
		avg_height = (area->area_info.height - 300) / pai->time_cnt;
		avg_len = pai->notanchor_text_len / pai->time_cnt;
	}

	point = pai->time_cnt * 5 + pai->binfo.comment_area_cnt * 20 + pai->binfo.comment_point * 5;

	if (pai->time_cnt != MAX_AREA_TIME_NUM)
	{
		//链接降权
		if (avg_anchor_len >= 30)
			point -= (int) ((avg_anchor_len - 30) * 0.5);
		if (avg_anchor_count >= 7)
			point -= (avg_anchor_count - 5);

		//文字间隔降权
		if (pai->max_time_gap_len >= 600 && pai->binfo.comment_point <= 0 && pai->binfo.comment_area_cnt == 0)
			point -= (int) ((pai->max_time_gap_len - 600) * 0.3);

		if (pai->notanchor_text_len / floor_count < 20)
			point -= (int) ((20 - pai->notanchor_text_len / floor_count));
	}
	//宽高度降权
	if (avg_height >= 250 && pai->binfo.comment_area_cnt == 0 && pai->binfo.comment_point == 0)
		point -= (avg_height - 250);

	if ((avg_len <= 25) && pai->binfo.comment_area_cnt == 0 && pai->binfo.comment_point == 0 && pai->time_cnt >= 4)
		point = point - pai->time_cnt * 5;

	//深度降权
	if (depth == 0 || depth >= 4)
		point = 0;

	if (area->area_info.width * 10 < page_width * 2)
		point = 0;
	else if (area->area_info.width * 10 < page_width * 3 && depth == 1)
		point = 0;

	return point;
}

/**
 * @brief 遍历分块，计算每个分块的分数
 */
static int visit_for_mark_blogre_area(html_area_t *area, void *data)
{
	if (!area->isValid)
		return AREA_VISIT_SKIP;

	blog_re_t *pblogre = (blog_re_t *) data;

	area_node_info *pai = get_area_node(pblogre->area_info, area);
	if (!pai)
		return AREA_VISIT_NORMAL;

	pai->time_cnt = cacu_max_depth_time_count_with_linktime_filter(pai);
	pai->binfo.comment_point = gen_final(pai, area, pblogre->page_width);

	if (pai->binfo.comment_point >= MIN_CMT_AREA_POINT && pblogre->cand_cmt_area_cnt < MAX_CAND_CMT_AREA_CNT)
	{
		pblogre->cand_cmt_areas[pblogre->cand_cmt_area_cnt++] = area;
	}

	if (pai->binfo.comment_point > pblogre->cur_max_cmt_point)
	{
		pblogre->cur_max_cmt_point = pai->binfo.comment_point;
		pblogre->cur_max_cmt_piont_area = area;
	}
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 获得分块的时间个数
 */
static inline int get_time_cnt(html_area_t *area, lt_area_info_t *pai)
{
	area_node_info *pani = get_area_node(pai, area);
	if (pani)
	{
		return pani->time_cnt;
	}
	return 0;
}

/**
 * @brief 获得分块的评论分数
 */
static inline int get_cmt_point(html_area_t *area, lt_area_info_t *pai)
{
	area_node_info *pani = get_area_node(pai, area);
	if (pani)
	{
		return pani->binfo.comment_point;
	}
	return 0;
}

/**
 * @brief 标记blog链接类型
 */
static void mark_blog_link_type(vlink_t *vlink, int link_count, blog_re_t *blog_re)
{
	for (int i = 0; i < link_count; i++)
	{
		if (vlink[i].inner.is_goodlink)
		{
			int in_area = 0;
			for (int j = 0; j < blog_re->cmt_area_cnt; j++)
			{
				if (in_one_area(&vlink[i].inner, blog_re->cmt_areas[j]))
				{
					in_area = 1;
					break;
				}
			}
			if (in_area)
				vlink[i].linkFunc |= VLINK_BLOGRE;
		}
	}
}

/**
 * @brief 下一个分块是否有分数大于0 的
 */
static bool next_area_point_greater_than_zero(int area_count, html_area_t *cur_area, lt_area_info_t *pai)
{
	int cur_area_count = 0;
	html_area_t *iter_area = cur_area->nextArea;
	while (cur_area_count < area_count && iter_area)
	{
		if (get_cmt_point(iter_area, pai) > 0)
			return true;
		iter_area = iter_area->nextArea;
		cur_area_count++;
	}
	return false;
}

/**
 * @brief 从子分块中挑选cand评论分块
 */
static void select_sub_good_area(lt_area_info_t *pai, blog_re_t *pblogre)
{
	int begin_copy = 0;
	pblogre->cmt_area_cnt = 0;
	for (html_area_t *subArea = pblogre->cmt_areas[0]->subArea; subArea; subArea = subArea->nextArea)
	{
		if (get_cmt_point(subArea, pai) > 0 && next_area_point_greater_than_zero(3, subArea, pai))
		{
			begin_copy = 1;
		}

		if (begin_copy && pblogre->cmt_area_cnt < MAX_CMT_AREA_CNT)
		{
			pblogre->cmt_areas[pblogre->cmt_area_cnt++] = subArea;
		}
	}
}

/**
 * @brief 当前分块是不是分块区域的一部分
 */
static int is_area_a_sub_cmt(html_area_t *area, lt_area_info_t *pai)
{
	html_area_t * find_begin = area;
	while (find_begin->prevArea)
	{
		find_begin = find_begin->prevArea;
	}

	int good = 0;
	int continue_good_count = 0;
	int space_node = 0;
	while (find_begin != NULL)
	{
		if (get_cmt_point(find_begin, pai) >= 5)
		{
			continue_good_count++;
			space_node = 1;
			if (continue_good_count >= 3)
			{
				good = 1;
			}
		}
		else if ((space_node--) < 0)
			continue_good_count = 0;
		find_begin = find_begin->nextArea;
	}
	return good;
}

/**
 * @brief 获得时间个数最大的子分块
 */
static html_area_t * get_max_time_cnt_sub_area(html_area_t * area, lt_area_info_t *pai)
{
	html_area_t *maxArea = NULL;
	int max_time_cnt = 0;
	for (html_area_t *subArea = area->subArea; subArea; subArea = subArea->nextArea)
	{
		int time_cnt = get_time_cnt(subArea, pai);
		if (time_cnt > max_time_cnt)
		{
			maxArea = subArea;
			max_time_cnt = time_cnt;
		}
	}
	return maxArea;
}

/**
 * @brief 获得评论分数最大的子分块
 */
static html_area_t *get_max_cmt_point_sub_area(html_area_t *area, lt_area_info_t *pai)
{
	html_area_t *maxArea = NULL;
	int max_cmt_point = 0;
	for (html_area_t *subArea = area->subArea; subArea; subArea = subArea->nextArea)
	{
		int cmt_point = get_cmt_point(subArea, pai);
		if (cmt_point > max_cmt_point)
		{
			maxArea = subArea;
			max_cmt_point = cmt_point;
		}
	}
	return maxArea;
}

/**
 * @brief 检查子分块是否是回复链接，
 */
static bool is_sub_area_split_cmt_area(html_area_t *area, lt_area_info_t *pai)
{
	int continue_good_count = 0;
	int space_node = 0;
	//记录分数比较高的子分块以及时间多的子分块
	for (html_area_t *subArea = area->subArea; subArea; subArea = subArea->nextArea)
	{
		int cmt_area_point = get_cmt_point(subArea, pai);

		if (cmt_area_point >= 5)
		{
			continue_good_count++;
			space_node = 1;
			if (continue_good_count >= 3)
			{
				return true;
			}
		}
		else if ((space_node--) < 0)
			continue_good_count = 0;
	}

	return false;
}

/**
 * @brief 标记博客回复链接
 */
static void mark_blogre_link(blog_re_t *blogre, lt_args_t *pargs, lt_res_t *pvres)
{
	if (blogre->cur_max_cmt_piont_area == NULL)
		return;

	vlink_t *vlink = pargs->vlink;
	blogre->cmt_areas[0] = blogre->cur_max_cmt_piont_area;
	blogre->cmt_area_cnt = 1;

	if (blogre->cur_max_cmt_point < MIN_CMT_AREA_POINT)
	{
		if (is_sub_area_split_cmt_area(blogre->cur_max_cmt_piont_area, pvres->area_info))
		{
			select_sub_good_area(pvres->area_info, blogre);
			mark_blog_link_type(vlink, pargs->link_count, blogre);
		}
		else
		{
			if (is_area_a_sub_cmt(blogre->cmt_areas[0], pvres->area_info))
			{
				blogre->cmt_areas[0] = blogre->cmt_areas[0]->parentArea;
				select_sub_good_area(pvres->area_info, blogre);
				mark_blog_link_type(vlink, pargs->link_count, blogre);
			}
			else
				blogre->cmt_area_cnt = 0;
		}
		return;
	}

	if (blogre->cur_max_cmt_piont_area->subArea != NULL
			&& (blogre->cur_max_cmt_piont_area->depth == 1 || blogre->cur_max_cmt_piont_area->depth == 2))
	{
		html_area_t * max_cmt_point_sub_area = get_max_cmt_point_sub_area(blogre->cur_max_cmt_piont_area,
				pvres->area_info);
		html_area_t *max_time_cnt_sub_area = get_max_time_cnt_sub_area(blogre->cur_max_cmt_piont_area,
				pvres->area_info);

		int parent_time_cnt = get_time_cnt(blogre->cur_max_cmt_piont_area, pvres->area_info);
		int max_time_count = get_time_cnt(max_time_cnt_sub_area, pvres->area_info);

		//找到时间大到一定程度能代替父分块的子分块
		if (max_time_count == parent_time_cnt && max_time_count >= 2)
		{
			blogre->cur_max_cmt_point = get_cmt_point(max_time_cnt_sub_area, pvres->area_info);
			blogre->cmt_areas[0] = max_time_cnt_sub_area;
		}
		else if (max_time_count * 3 >= parent_time_cnt && max_time_count >= 4)
		{
			blogre->cur_max_cmt_point -= parent_time_cnt * 5;
		}

		if (blogre->cur_max_cmt_point < MIN_CMT_AREA_POINT)
		{
			if (get_cmt_point(max_cmt_point_sub_area, pvres->area_info) < MIN_CMT_AREA_POINT)
			{
				blogre->cmt_areas[0] = NULL;
				blogre->cmt_area_cnt = 0;
				for (int i = 0; i < blogre->cand_cmt_area_cnt; i++)
				{
					if (blogre->cand_cmt_areas[i] != blogre->cur_max_cmt_piont_area
							&& blogre->cand_cmt_areas[i] != blogre->cur_max_cmt_piont_area->parentArea)
						blogre->cmt_areas[blogre->cmt_area_cnt++] = blogre->cand_cmt_areas[i];
				}
				if (blogre->cmt_area_cnt == 0)
				{
					return;
				}
			}
			else
				blogre->cmt_areas[0] = max_cmt_point_sub_area;
		}
	}

	if (blogre->cmt_area_cnt == 1)
	{
		//填充其它好的分数也很高的分块
		for (int i = 0; i < blogre->cand_cmt_area_cnt; i++)
		{
			if (blogre->cand_cmt_areas[i] != blogre->cmt_areas[0]
					&& blogre->cand_cmt_areas[i] != blogre->cmt_areas[0]->parentArea
					&& blogre->cmt_area_cnt < MAX_CMT_AREA_CNT)
			{
				int base_point = get_cmt_point(blogre->cmt_areas[0], pvres->area_info);
				int good_point = get_cmt_point(blogre->cand_cmt_areas[i], pvres->area_info);

				if (base_point == good_point
						|| (base_point >= 20 && base_point - good_point <= MIN_CMT_AREA_POINT && base_point > good_point))
				{
					blogre->cmt_areas[blogre->cmt_area_cnt++] = blogre->cand_cmt_areas[i];
				}
			}
		}

		if (blogre->cmt_area_cnt == 1 && is_sub_area_split_cmt_area(blogre->cur_max_cmt_piont_area, pvres->area_info))
		{
			select_sub_good_area(pvres->area_info, blogre);
		}
	}

	mark_blog_link_type(vlink, pargs->link_count, blogre);
}

/**
 * @brief 博客回复链接标记
 *
 * @param [in/out] pargs   : lt_args_t*	输入包
 * @param [in/out] pvres   : lt_res_t*		标记所用的资源
 * @return  int 
 * @retval   	=0	函数正常返回
 * @retval	<0	函数出错
 **/
int mark_linktype_blogre(lt_args_t *pargs, lt_res_t *pvres)
{
	if (!PT_IS_BLOG(pargs->pagetype))
	{
		return 0;
	}
	get_area_info(pargs->atree, pvres->area_info, pargs->url, pvres->ptime_match);

	blog_re_t blogre;
	memset(&blogre, 0, sizeof(blogre));
	blogre.area_info = pvres->area_info;
	blogre.page_width = pargs->atree->root->area_info.width;
	areatree_visit(pargs->atree, (FUNC_START_T) visit_for_mark_blogre_area, NULL, &blogre);

	mark_blogre_link(&blogre, pargs, pvres);

	return 0;
}
