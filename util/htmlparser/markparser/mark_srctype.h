/*
 * easou_mark_srctype.h
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#ifndef EASOU_MARK_SRCTYPE_H_
#define EASOU_MARK_SRCTYPE_H_

#include "easou_ahtml_tree.h"
#include "easou_mark_conf.h"
#include "easou_mark_inner.h"

void tag_area_srctype(html_area_t * area , html_area_srctype_t type );

/**
 * @brief 标注资源类型.
**/
bool mark_srctype_area(area_tree_t * atree );

/**
 * @brief 当前块是否某种资源类型.
**/
bool is_srctype_area(const html_area_t *area, html_area_srctype_t srctype);

/**
 * @brief 当前块是否在某个资源类型块内. 当前块就是这种类型的情况也返回true;
**/
bool is_in_srctype_area(const html_area_t *area, html_area_srctype_t srctype);

/**
 * @brief 当前块是否包含某个资源类型块. 当前块就是这种类型的情况也返回true;
**/
bool is_contain_srctype_area(const html_area_t *area, html_area_srctype_t srctype);

/**
 * @brief 获取某种资源类型的标注结果，如果没有结果返回NULL
**/
const area_list_t * get_srctype_mark_result(area_tree_t * atree , html_area_srctype_t srctype);

/**
 * 以下是各种资源分块类型对应的标注函数.
 */
/**其他块*/
bool mark_other_srctypes(area_tree_t * area_tree);

//interaction块
bool mark_srctype_interaction(area_tree_t * area_tree);

//picture��
bool mark_srctype_picture(area_tree_t * area_tree);

/**link块*/
bool mark_srctype_link(area_tree_t *atree);

#endif /* EASOU_MARK_SRCTYPE_H_ */
