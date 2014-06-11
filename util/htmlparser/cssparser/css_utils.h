/**
 * css_utils.h
 * Description: CSS解析对外接口
 *  Created on: 2011-06-20
 * Last modify: 2012-10-31 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#ifndef EASOU_CSS_UTILS_H_
#define EASOU_CSS_UTILS_H_

#include "css_pool.h"
#include "util/htmlparser/htmlparser/html_tree.h"

/**
 * @brief 对网页中css的描述
 */
typedef struct _page_css_t
{
	short style_txt_num; /**< style css文本的数量  */
	const char *style_txt[MAX_CSS_NUM_IN_POOL]; /**< css style 文本  */
	char css_url[MAX_CSS_NUM_IN_POOL][MAX_URL_SIZE]; /**< 保存外部css对应的url */
} page_css_t;

/**
 * @brief CSS解析环境.
 */
typedef struct _css_env_t
{
	page_css_t page_css; /**< 页面中css     */
	css_pool_t css_pool; /**< CSS的解析结果      */
} css_env_t;

/**
 * @brief 页面的css信息，主要是内部使用
 */
typedef struct _cssinfo_keep_t
{
	const char *url; /**< 页面URL  */
	bool is_skip_child; /**< 是否处理子孙节点  */
	page_css_t *page_css; /**页面中的css信息*/
} cssinfo_keep_t;

/**
 * @brief 判断该节点以及该节点到子节点是否支持screen_media
 * @param [in]html_tag: html_tag_t *  节点的类型.

 * @date 2011/06/20
 **/
bool is_apply_for_screen_media(html_tag_t *html_tag);

/**
 * @brief CSS环境的创建.
 * @param [in] max_css_page_size   : int 最大css
 * @param [in] css_num   : int	最多解析css的数量.
 * @return  css_env_t*	css解析环境.

 * @date 2011/06/20
 **/
css_env_t *css_env_create(int max_css_page_size, int css_num);

/**
 * @brief 销毁css解析环境.

 * @date 2011/06/20
 **/
void css_env_destroy(css_env_t *env);

/**
 * @brief	从页面中获取css信息
 * @param [out] page_css   : page_css_t*	页面中的css信息
 * @param [in] html_tree   : const html_tree_t*	创建好的dom树
 * @param [in] url   : const char*	页面的url

 * @date 2011/06/20
 **/
void get_page_css_info(page_css_t *page_css, const html_tree_t *html_tree, const char *url);

/**
 * @brief 解析页面中的css
 * @param [in] test_import, 是否测试css文件中import的css

 * @date 2011/06/20
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 **/
void parse_internal_css(css_env_t *css_env, page_css_t *page_css, const char *url, bool test_import = false);

/**
 * @brief	获取并解析页面中的css.
 * @param [out] css_env   : css_env_t*	css解析环境.
 * @param [in] html_tree   : html_tree_t*	解析过的html树.
 * @param [in] url   : const char*	页面URL.
 * @param [in] test_import, 是否测试css文件中import的css

 * @date 2011/06/20
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 **/
int get_parse_css_inpage(css_env_t *css_env, const html_tree_t *html_tree, const char *url, bool test_import = false);

/**
 * @brief 调试接口，打印解析好的css信息到文件中
 * @param [in] csspool	:	解析好的css结果
 * @param [in] filename	:	保存的文件名称
 * @author sue
 * @date 2012/04/09
 */
void csspool_print_selector(const css_pool_t *csspool, const char* filename);

/**
 * @brief 复位css解析环境
 * @param [in] cc:css_env_t * css环境
 */
void css_env_reset(css_env_t *cc);

/**
 * @brief 添加外部css
 * @param [in/out]page_css_t *css_keep css环境
 * @param [in] ptxt 外部css文本，ptxt空间在解析完成之前不能释放
 * @param [in] css_url, 外部css的url
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 */
void add_out_style_text(page_css_t *css_keep, char *ptxt, char *css_url);

bool is_css_link_tag(html_tag_t *html_tag);

#endif /*EASOU_CSS_UTILS_H_*/
