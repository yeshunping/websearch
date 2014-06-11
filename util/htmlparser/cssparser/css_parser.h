
#ifndef EASOU_CSS_PARSER_H_
#define EASOU_CSS_PARSER_H_

#include "util/htmlparser/htmlparser/html_tree.h"
#include "css_nodepool.h"
#include "css_dtd.h"
#include "css_parser_inner.h"

#define CSS_DEFAULT_TEXT_SIZE	(128*1024)		  /**< CSS文件默认大小  */

/**
 * @brief create css structure and alloc memery
 *
 * @param [in] max_css_text_size   : int CSS文本的最大长度,按此长度分配内存.
 * @return  css_t* 创建好的CSS结构指针.
 * @retval  若失败返回NULL.

 * @date 2011/06/20
 **/
css_t *css_create(int max_css_text_size);

/**
 * @brief 用默认参数创建CSS结构.
 * @return  css_t* 创建的CSS结构.
 * @retval   失败返回NULL.

 * @date 2011/06/20
 **/
css_t *css_create();

/**
 * @brief 销毁CSS结构.
 * @param [in/out] css   : css_t*

 * @date 2011/06/20
 **/
void css_destroy(css_t *css);

/**
 * @brief 清空css_t结构,回到未解析的状态.
 * @param [in/out] css   : css_t*

 * @date 2011/06/20
 **/
void css_clean(css_t *css);

/**
 * @brief 根据CSS文本解析得到CSS结构,并创建索引.
 * @param [in/out] css   : css_t*	已分配空间的CSS结构.
 * @param [in] css_text   : const char*	指向CSS文本.
 * @param [in] css_url, css的url
 * @param [in] is_continue, 如果为true则不重置css参数
 * @param [in] test_import, 是否测试css文件中import的css
 * @return  int 
 * @retval  -1:解析失败;1:解析成功.

 * @date 2011/06/20
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 **/
int css_parse(css_t *css, const char *css_text, const char *css_url, bool is_continue, bool test_import);

/**
 * @brief	解析css文本，不建索引，主要是要找到选择子和选择子对应的规则集.
 * @param [in/out] css   : css_t*	解析好的css结构.
 * @param [in] css_text   : const char*	CSS文本
 * @param [in] css_url, 被解析的CSS的url
 * @param [in] is_continue, 如果为true则不重置css参数
 * @param [in] test_import, 是否测试css文件中import的css
 * @return  int 
 * @retval  -1解析失败;1解析成功.

 * @date 2011/06/20
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 **/
int css_parse_no_index(css_t *css, const char *css_text, const char *css_url, bool is_continue, bool test_import);

/**
 * @brief 是否有用的CSS属性,目前只认为几何属性和字体属性有用.

 * @date 2011/06/20
 **/
bool is_useful_css_prop(css_prop_type_t prop);

/**
 * @brief 根据属性名查找属性类型.
 * @param [in] name   : const char*	属性名称
 * @return  css_prop_type_t	属性类型

 * @date 2011/06/20
 **/
css_prop_type_t css_seek_prop_type(const char *name);

#endif /*EASOU_CSS_PARSER_H_*/
