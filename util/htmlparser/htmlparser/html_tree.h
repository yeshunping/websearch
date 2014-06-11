/**
 * easou_html_tree.h
 *
 *  Created on: 2011-11-8
 *      Author: xunwu
 */

#ifndef EASOU_HTML_TREE_H_
#define EASOU_HTML_TREE_H_

#include "html_dom.h"

struct printDomS
{
	char *psrccode;
	int availpos;
	int buflen;
};

/**
 * @brief 初始化dom树
 **/
html_tree_t *html_tree_create(int max_page_len);

/**
 * @brief 清空dom树
 void html_tree_clean(html_tree_t *html_tree);
 **/

/**
 * @brief 销毁dom树
 **/
void html_tree_del(html_tree_t *html_tree);

/**
 * @brief 设置是否解析js
 **/
void html_tree_set_script_parsing(html_tree_t *tree, int enable);

/**
 * @brief 判断是否解析js
 **/
int html_tree_is_script_parsing(html_tree_t *tree);

/**
 * @brief 将html网页解析成dom树
 * @param [in/out] html_tree   : html_tree_t*	dom树结构
 * @param [in] page   : char*	页面源代码
 * @param [in] page_len   : int	页面源代码的长度
 * @return  int
 * @retval  0:解析失败;1:解析成功.

 * @date 2011/08/02
 **/
int html_tree_parse(html_tree_t *html_tree, char *page, int page_len, bool ignore_space = true);

/**
 * @brief Pre visitor
 **/
typedef int (*start_visit_t)(html_tag_t* html_tag, void* result, int flag);

/**
 * @brief Post visitor
 **/
typedef int (*finish_visit_t)(html_tag_t* html_tag, void* result);

/**
 * @brief 遍历dom树
 * @param html_tree :建立好的dom树
 * @param start_visit,函数指针，用于深度遍历dom树，进入时使用
 * @param finish_visit,函数指针，用于深度遍历dom树，访问节点及其子树完成时使用
 **/
int html_tree_visit(html_tree_t *html_tree, start_visit_t start_visit, finish_visit_t finish_visit, void *result, int flag);

/**
 * @brief 遍历节点
 *  @param node :建立好的dom树的节点
 * @param start_visit,函数指针，用于深度遍历dom树，进入时使用
 * @param finish_visit,函数指针，用于深度遍历dom树，访问节点及其子树完成时使用
 **/
int html_node_visit(html_node_t *node, start_visit_t start_visit, finish_visit_t finish_visit, void *result, int flag);

/**
 * @brief 重置树,不释放空间
 **/
int html_tree_reset_no_destroy(struct html_tree_impl_t *self);

/**
 * 打印节点及其属性
 */
void printNode(html_node_t *html_node);

/**
 * 根据树确定文档类型
 * @param:
 * html_tree, [in], 解析好的html树
 * url, [in], URL地址
 * @return:
 * 0, 成功；-1，失败
 */
int determine_doctype(html_tree_t *html_tree, const char* url);

/**
 * 打印节点及其属性
 */
void printTree(html_tree_t *html_tree);

void PrintNode(html_node_t *html_node, int level);

/**
 * 标注dom树的每个节点含有的子节点类型
 * html_tree：in
 * 结果：-1：参数错误；0：成功；>0:其他错误
 *
 */
int markDomTreeSubType(html_tree_t *html_tree);

int PrintNodeSrc(html_node_t *html_node, printDomS *pPrintSrc, int level);

int printTreeSrc(html_tree_t *html_tree, char *srcodebuf, int buflen);

#endif /* EASOU_HTML_TREE_H_ */
