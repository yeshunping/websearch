/*
 * easou_mark_func.h
 *
 *  Created on: 2011-11-22
 * Last modify: 2012-11-10 sue_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easou.com
 *      Version: 1.1
 */

#ifndef EASOU_MARK_FUNC_H_
#define EASOU_MARK_FUNC_H_

#include "easou_ahtml_tree.h"
#include "easou_mark_conf.h"
#include "easou_mark_inner.h"

/**
 * @brief 对当前块标记功能类型.
**/
void tag_area_func(html_area_t * area , html_area_func_t type);


/**
 * @brief 对当前块清除某种功能类型标注.
 * @date 2012-10-29
 * @author sue
**/
void clear_func_area_tag(html_area_t *area, html_area_func_t type);

/**
 * @brief 标记功能类型.
**/
bool mark_func_area(area_tree_t * atree );

/**
 * @brief 当前块是否某种功能类型.
**/
bool is_func_area(const html_area_t *area, html_area_func_t func);

/**
 * @brief 当前块是否在某种功能类型.当前块就是这种类型的情况也返回true;
**/
bool is_in_func_area(const html_area_t *area, html_area_func_t func);

/**
 * @brief 当前块是否包含某个功能类型块. 当前块就是这种类型的情况也返回true;

**/
bool is_contain_func_area(const html_area_t *area, html_area_func_t func);

/**
 * @brief 对特定的块进行功能标注
**/
bool mark_spec_func_on_area(area_tree_t *atree, html_area_func_t func);

/**
 * @brief 获取某种功能类型的标注结果，如果没有结果返回NULL
**/
const area_list_t * get_func_mark_result(area_tree_t * atree , html_area_func_t func) ;


/**
 * 以下是各种分块类型对应的标注函数.
 */
bool mark_func_time(area_tree_t *atree);

bool mark_func_copyright(area_tree_t *atree);

bool mark_func_mypos(area_tree_t *atree);

bool mark_func_nav(area_tree_t *atree);

/**未实现*/
bool mark_func_relate_link(area_tree_t *atree);

bool mark_func_friend(area_tree_t *atree);

bool mark_func_article_sign(area_tree_t *atree);

bool mark_func_subtitle(area_tree_t *atree);

bool mark_func_subtit_forrt(area_tree_t *atree);

/**
 * @brief 标记翻页块
 * @date 2012-10-17
 * @author sue
 */
bool mark_func_turnpage(area_tree_t *atree);

/**
 * @brief 标记文章的内容块，主要针对新闻/内容页
 * @date 2012-10-28
 * @author sue
 */
bool mark_func_article_content(area_tree_t *atree);

/**
bool is_func_key_value_area(const html_area_t * area ,mark_area_info_t* mark_info);

bool mark_func_key_value(area_tree_t *atree);

bool mark_func_time(area_tree_t *atree);

bool mark_func_subtit_forrt(area_tree_t *atree);

bool mark_func_table(area_tree_t *atree);

bool mark_func_reply(area_tree_t *atree);

bool mark_func_posts(area_tree_t *atree);

bool mark_func_meta_footer(area_tree_t *atree);
*/
#endif /* EASOU_MARK_FUNC_H_ */
