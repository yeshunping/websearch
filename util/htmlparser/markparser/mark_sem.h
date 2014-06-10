/*
 * easou_mark_sem.h
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */

#ifndef EASOU_MARK_SEM_H_
#define EASOU_MARK_SEM_H_

#include "easou_ahtml_tree.h"
#include "easou_mark_conf.h"
#include "easou_mark_inner.h"

void tag_area_sem(html_area_t *area, html_area_sem_t type);

/**
 * @brief 对当前块清除某种语义类型标注.
**/
void clear_sem_area_tag(html_area_t *area, html_area_sem_t type);

/**
 * @brief 标注各种语义分块类型. 该接口只在非lazy模式, 或已选择了某些类型时才调用.
**/
bool mark_sem_area(area_tree_t *atree);

/**
 * @brief 当前块是否某种语义类型.
**/
bool is_sem_area(const html_area_t *area, html_area_sem_t sem);

/**
 * @brief 当前块是否包含某个语义类型块. 当前块就是这种类型的情况也返回true;
**/
bool is_in_sem_area(const html_area_t *area, html_area_sem_t sem);

/**
 * @brief 当前块是否包含某个语义类型块. 当前块就是这种类型的情况也返回true;
**/
bool is_contain_sem_area(const html_area_t *area, html_area_sem_t sem);

/**
 * @brief 获取某种语义类型的标注结果，如果没有结果返回NULL
**/
const area_list_t * get_sem_mark_result(area_tree_t * atree , html_area_sem_t sem);


/**
 * 以下是各种语义类型对应的标注函数.
 */
bool mark_sem_realtitle(area_tree_t *atree);

bool mark_sem_central(area_tree_t *atree);
bool mark_sem_hub_central(area_tree_t *atree);
#endif /* EASOU_MARK_SEM_H_ */
