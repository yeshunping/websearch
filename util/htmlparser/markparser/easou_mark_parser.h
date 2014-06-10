/*
 * easou_mark_parser.h
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */

#ifndef EASOU_MARK_PARSER_H_
#define EASOU_MARK_PARSER_H_

#include "easou_ahtml_tree.h"

#define IS_VOID_AREA_MARK(bits)	(0==bits._mark_bits)		  /**< 空的分块标注  */

/**
* @brief VTREE分块标注输入包
*/
typedef struct _vmark_input_t {
	const char *anchor;		  /**< 前链anchor       */
}vmark_input_t;

/**
 * @brief 对VTREE分块进行标注.
 * 	可利用shut_mark_type()接口对某些标注类型进行关闭.
 * @param [in/out] atree   : area_tree_t*	待标注的vtree.
 * @param [in] url   : const char*	页面URL 。
 * @param [in] vmark_in   : vmark_input_t*	标注输入包.
 * @return  bool	是否成功.
 * @author xunwu
 * @date 2011/07/06
**/
bool mark_area_tree(area_tree_t * atree, const char *url, vmark_input_t *vmark_in = NULL);

/**
 * @brief 释放mark tree的空间
**/
void mark_tree_destory(area_tree_t * atree);

/**
 * @brief 清空mark tree的mark痕迹
**/
void mark_tree_clean(area_tree_t * atree);

/**
 * @brief 获取标注结果，如果没有结果返回NULL
 * @author xunwu
 * @date 2011/07/06
**/
bool get_mark_result( area_tree_t * atree);

/**
 * @brief 复位mark tree的空间，回复初始化状态
 * @param [in] atree
**/
void mark_tree_reset(area_tree_t * atree);

#endif /* EASOU_MARK_PARSER_H_ */
