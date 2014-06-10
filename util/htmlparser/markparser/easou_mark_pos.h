/**
 * easou_mark_pos.h
 *	Description: 标注分块的位置。包括每个分块相对父分块的位置，以及每个分块在页面中的相对位置和绝对位置。
 *  Created on: 2011-06-27
 *      Author: xunwu_chen@staff.easou.com
 *     Version: 1.0
 */
#ifndef EASOU_MARK_POS_H_
#define EASOU_MARK_POS_H_

#include "easou_ahtml_tree.h"

/**
 * @brief mark每个分块节点相对于父分块的相对位置.
 **/
void area_markPos(area_tree_t *atree);

/**
 * @brief mark每个分块相对于页面的位置
 **/
bool mark_pos_area(area_tree_t * atree);

#endif /* EASOU_MARK_POS_H_ */
