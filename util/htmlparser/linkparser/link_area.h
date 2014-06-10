/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_area.h,v 1.2 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_link_area.h
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 分块信息统计
 **/

#ifndef _EASOU_LINK_AREA_H
#define _EASOU_LINK_AREA_H

#include "easou_vhtml_parser.h"
#include "easou_ahtml_area.h"
#include "easou_link_timematch.h"

#define MAX_AREA_NUM 	2500
#define MAX_TIME_NUM	1000
#define MAX_AREA_TIME_NUM	60
#define MAX_VNODE_NUM	50000

/**
 * @brief short description 时间结点
 */
typedef struct
{
	html_vnode_t *vnode; /**<  结点      */
	unsigned int next_gap; /**<  结点到下一个时间结点的文字间隔      */
	unsigned int last_gap; /**<  结点到上一个时间结点的文字间隔      */
	bool next_has_cont; /**<  结点与下一个时间结点之间是否有正文      */
	bool last_has_cont; /**<  结点与上一个时间结点之间是否有正文      */
	unsigned int route_sign; /**<  路径签名      */
} time_node_info;

/**
 * @brief short description 分块内部信息，时间信息
 */
typedef struct
{
	time_node_info *ptime_nodes[MAX_AREA_TIME_NUM]; /**< 时间结点     */
	unsigned int time_node_cnt; /**<  时间个数      */
} area_node_inner_info;

/**
 * @brief short description  blog bbs属性
 */
typedef struct
{
	int comment_point; /**<  评论分数      */
	int comment_area_cnt; /**<  评论块的个数      */
} blog_bbs_info;

/**
 * @brief short description 单个分块信息
 */
typedef struct
{
	area_node_inner_info inner; /**<   分块内部数据      */
	int time_cnt; /**<  时间个数      */
	int inner_link_cnt; /**<  内链个数      */
	int outer_link_cnt; /**<  外链个数      */
	int inner_anchor_len; /**<  内链anchor长度      */
	int anchor_len; /**<  anchor总长度      */
	int notanchor_text_len; /**<  正文长度      */
	int max_time_gap_len; /**<  时间节点之间最大间隔文字长度      */
	int anchor_cnt; /**<  anchorg个数      */
	blog_bbs_info binfo; /**<  blog bbs属性      */
} area_node_info;

/**
 * @brief short description 页面总的分块信息
 */
typedef struct _lt_area_info_t
{
	time_node_info time_nodes[MAX_TIME_NUM]; /**<  时间结点buffer      */
	area_node_info *area_nodes; /**<  分块属性  */
	int area_node_size;
	unsigned int time_nodes_cnt; /**<  时间结点个数      */
} lt_area_info_t;

static inline void del_area_info(lt_area_info_t *parea_info)
{
	if (parea_info)
	{
		free(parea_info->area_nodes);
		free(parea_info);
	}
}

/**
 * @brief 创建分块信息buffer
 */
static inline lt_area_info_t * create_area_info()
{
	lt_area_info_t *parea_info = (lt_area_info_t *) calloc(1, sizeof(lt_area_info_t));
	if (parea_info == NULL)
	{
		Error("calloc area_info error.");
		del_area_info(parea_info);
		return NULL;
	}

	parea_info->area_nodes = (area_node_info *) calloc(MAX_AREA_NUM, sizeof(area_node_info));
	if (parea_info->area_nodes == NULL)
	{
		Error("calloc vhplk_area_node_info error.");
		del_area_info(parea_info);
		return NULL;
	}

	parea_info->area_node_size = MAX_AREA_NUM;

	return parea_info;
}

/**
 * @brief 获得指定分块对应的分块统计信息
 */
static inline area_node_info *get_area_node(lt_area_info_t *pai, html_area_t *area)
{
	if (!area)
		return NULL;
	if (area->no >= MAX_AREA_NUM)
	{
		Info("parea->no %d too big", area->no);
		return NULL;
	}
	return &pai->area_nodes[area->no];
}

/**
 * @brief 获得分块信息
 */
void get_area_info(area_tree_t *pareas, lt_area_info_t *pai, const char * url, timematch_pack_t *ptimematch);

/**
 * @brief 统计当前分块按路径签名相同的时间最多个数
 */
unsigned int cacu_max_depth_time_count(area_node_info *pcur_node);

/**
 * @brief 统计当前分块按路径签名相同的时间最多个数，过滤时间链接
 */
unsigned int cacu_max_depth_time_count_with_linktime_filter(area_node_info *pcur_node);

#endif
