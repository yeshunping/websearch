/**
 * easou_css_parser_inner.h
 * Description: CSS结构定义和内部公用函数
 *  Created on: 2011-06-20
 * Last modify: 2012-10-31 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#ifndef EASOU_CSS_PARSER_INNER_H_
#define EASOU_CSS_PARSER_INNER_H_

#include <stdio.h>
#include "css_dtd.h"

/**
 * @brief 如果CSS文本太长,重新分配str_heap.
 * @param [in/out] css   : css_t*	CSS结构.
 * @param [in] len   : size_t	当前CSS文本长度.
 * @return  int	分配状态.
 * @retval  -1:失败；0:成功.

 * @date 2011/06/20
 **/
int adjust_str_heap(easou_css_t *css, size_t len);

/**
 * @brief	打印解析出的CSS.

 * @date 2011/06/20
 **/
void print_css(easou_css_t *css, FILE *fout);

/**
 * @brief 为CSS结构创建hash索引，需要配合遍历去渲染节点
 * @param [in/out] css   : css_t*	已经创建好CSS规则集列表,但未创建索引的CSS结构
 * @return 是否成功
 * @author sue
 * @date 2013/04/11
 */
bool css_create_hash_index(easou_css_t *css, hashmap_t* hm);

/**
 * @brief	是否几何属性.

 * @date 2011/06/20
 **/
bool is_geo_property(easou_css_property_t *prop);

/**
 * @brief	是否字体属性.

 * @date 2011/06/20
 **/
bool is_font_property(easou_css_property_t *prop);

/**
 * @brief 扫描注释
 *  若为注释则返回指针指向注释之后的第一个字符.
 * @return  const char* 注释后的第一个字符指针.

 * @date 2011/06/20
 **/
const char *css_skip_comment(const char *pstr);

/**
 * @brief css中的跳过一块{}包含着的.
 * @return  const char* 截止点

 * @date 2011/06/20
 **/
const char *skip_block(const char *pstr);

/**
 * @brief skip at-rule (@import ..)
 * 	if not at-rule, return the input pointer.
 * @param [in] pstr   : const char*
 * @return  const char* 

 * @date 2011/06/20
 **/
const char *css_skip_at_rule(const char *pstr);

/**
 * @brief 跳过当前属性
 * @param [in/out] pstr   : const char*
 * @return  const char*

 * @date 2011/06/20
 **/
const char* skip_current_prop(const char *pstr);

/**
 * @brief 跳过当前的选择子.

 * @date 2011/06/20
 **/
const char *skip_current_selector(const char *pstr);

/**
 * @brief 扫描字符串 assert(*(css_scan->p_next) == '"' || *(css_scan->p_next) == '\'')
 * @param [in/out] css_scan   : css_scan_t*
 * @param [in] str_heap   : css_str_heap_t*
 * @return  void
 * @retval
 * @see

 * @date 2011/06/20
 **/
void scan_string(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap);

/**
 * @brief

 * @date 2011/06/20
 **/
bool is_css_sep_chr_state(easou_css_state_t state, char chr);

/**
 * @brief	扫描属性

 * @date 2011/06/20
 **/
void scan_attr(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap);

/**
 * @brief

 * @date 2011/06/20
 **/
inline void scan_a_step(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap);

/**
 * @brief

 * @date 2011/06/20
 **/
void scan_normal(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap);

/**
 * @brief

 * @date 2011/06/20
 **/
void scan_url(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap);

/**
 * @brief 解析css过程中，跳过一些可以忽略的元素，比如注释
 * @param [in/out] css_scan, 扫描css文件用到的结构体
 * @param [in/out] css, 用于保存解析好了的结构化的css信息

 * @date 2011/06/20
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 **/
void scan_ignore(easou_css_scan_t *css_scan, easou_css_t *css);

#endif /*EASOU_CSS_PARSER_INNER_H_*/
