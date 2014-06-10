/*
 * easou_mark_inner.h
 *
 *  Created on: 2011-11-20
 *      Author: xunwu_chen@staff.easou.com
 *     Version: 1.0
 */

#ifndef EASOU_MARK_INNER_H_
#define EASOU_MARK_INNER_H_

#include "easou_ahtml_tree.h"
#include "easou_mark_conf.h"
#include "easou_mark_markinfo.h"

#define IS_COMPUTED_INFO(flag) ((flag) & 0x01)
#define SET_COMPUTED_INFO(flag) ((flag)|= 0x01)
#define IS_MARKED_POS(flag)     ((flag) & 0x02)
#define SET_MARKED_POS(flag)     ((flag)|= 0x02)
#define IS_MARKED_SRCTYPE(flag) ((flag) & 0x04)
#define SET_MARKED_SRCTYPE(flag) ((flag) |= 0x04)
#define IS_MARKED_FUNC(flag) ((flag) & 0x08)
#define SET_MARKED_FUNC(flag) ((flag) |= 0x08)
#define IS_MARKED_SEM(flag) ((flag) & 0x10)
#define SET_MARKED_SEM(flag) ((flag) |= 0x10)

#define MARK_TYPE_POS	(1)		  /**< 标注位置 */
#define MARK_TYPE_BASE_INFO	(2)		  /**< 标注基本信息 */

extern const unsigned int mark_type_to_mask[];

void insert_area_list(area_list_t * area_list, area_list_node_t * area_node);

int insert_area_on_list(area_list_t area_list_arr[], int index, html_area_t *area, nodepool_t *area_pool);

/**
 * @brief 获取先序深度优先遍历的下一个节点.
 **/
html_area_t *html_area_iter_next(html_area_t *area, int visit_cmd);

/**
 * @brief 获取下一个可视块
 **/
html_area_t *next_valid_area(html_area_t *area);

/**
 * @brief 是否关闭了某种类型的标注.
 **/
template<typename AREA_TYPE_T>
bool is_shutted_type(area_tree_t *atree, AREA_TYPE_T t);

/**
 * @brief 是否选择了某种类型的标注.
 **/
template<typename AREA_TYPE_T>
bool is_selected_type(area_tree_t *atree, AREA_TYPE_T t);

#endif /* EASOU_MARK_INNER_H_ */
