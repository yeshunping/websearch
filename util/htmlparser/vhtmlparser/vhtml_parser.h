#ifndef _EASOU_HTMLPS_H_
#define _EASOU_HTMLPS_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/htmlparser/utils/nodepool.h"
#include "util/htmlparser/htmlparser/html_tree.h"
#include "util/htmlparser/cssparser/css_parser.h"
#include "util/htmlparser/cssparser/css_utils.h"
#include "util/htmlparser/vhtmlparser/vhtml_inner.h"
#include "util/htmlparser/vhtmlparser/vhtml_basic.h"
#include "util/htmlparser/vhtmlparser/vstruct_profiler.h"

#define DEFAULT_TABLE_COL_NUM	128		  /**<   */
#define MAX_COLSPAN_NUM	128 /**<  */
#define MAX_AVAIL_SPACE_NUM  64		  /**<   */

#define PARSE_ERROR   -1		  /**<        */
#define PARSE_SUCCESS   1		  /**<       */
#define PARSE_TABLE_FAIL   2		  /**<        */

/**
 * @brief
 */
typedef struct avail_space_t
{
	int x; /**<  */
	int y; /**<   */
	int width; /**<  */
} avail_space_t;

typedef struct _space_mgr_t
{
	avail_space_t space[MAX_AVAIL_SPACE_NUM]; /**< */
	int top; /**<   */
} space_mgr_t;

typedef struct _table_col_t
{
	int wx; /**<  */
	int wxlimit; /**<  */
	int span;
	int min_wx; /**< */
	int max_wx; /**<  */
} table_col_t;

/**
 * @brief

 * @date 2011/06/27
 **/
int html_vtree_init();

/**
 * @brief 创建V树

 * @date 2011/06/27
 **/
html_vnode_t *construct_vtree(nodepool_t *np, html_node_t *node, int depth, int &id, html_vtree_t * vtree);

/**
 * @brief

 * @date 2011/06/27
 **/
void html_vtree_get_html_property(html_vtree_t *html_vtree);

/**
 * @brief 使用css渲染vtree

 * @date 2011/06/27
 **/
void html_vtree_add_info_with_css(html_vtree_t *html_vtree, css_pool_t *css_pool);

/**
 * @brief 使用css渲染vtree，该方法使用css的hash索引进行渲染
 * @author sue
 * @date 2013/04/12
 */
void html_vtree_add_info_with_css2(html_vtree_t *html_vtree, css_pool_t *css_pool);

/**
 * @brief

 * @date 2011/06/27
 **/
void get_root_wx(html_vnode_t *root, int page_width);

/**
 * @brief 计算V树节点的宽度

 * @date 2011/06/27
 **/
int html_vtree_compute_wx(html_vnode_t *vnode, table_col_t *default_col);

/**
 * @brief

 * @date 2011/06/27
 **/
void layout_down_top(html_vnode_t *vnode, space_mgr_t *space_mgr, int depth = 0);

/**
 * @brief 计算绝对位置

 * @date 2011/06/27
 **/
void compute_absolute_pos(html_vnode_t *vnode);

/**
 * @brief 调整宽度

 * @date 2011/06/27
 **/
void get_page_width(html_vnode_t *vnode);

/**
 * @brief 解析字体。
 *	vtree语法树已建好，但可以尚未进行位置和大小计算.

 * @date 2011/06/27
 **/
int html_vtree_parse_font(html_vtree_t *html_vtree);

/**
 * @brief 非递归的，后序遍历V树

 * @date 2011/06/27
 **/
void trans_down_top(html_vnode_t *root, int page_width);

/**
 * @brief 获取节点的css属性值

 * @date 2011/06/27
 **/
char *get_css_attribute(html_vnode_t *html_vnode, css_prop_type_t type);

/**
 * @brief 判断vnode的子节点是否在同一行上

 * @date 2011/06/27
 **/
bool is_child_in_aline(html_vnode_t *vnode);

#endif /* _EASOU_HTMLPS_H_ */
