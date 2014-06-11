/*
 * html_extractor.h
 *
 *  Created on: 2012-1-9
 *      Author: xunwu
 */

#ifndef EASOU_HTML_EXTRACTOR_H_
#define EASOU_HTML_EXTRACTOR_H_

#include "html_tree.h"

/**
 * @brief link的抽象
 **/
typedef struct _link_t
{
	char url[UL_MAX_URL_LEN];
	char text[UL_MAX_TEXT_LEN];
} link_t;

/**
 * @brief 提取标题tagtitle
 **/
int html_tree_extract_title(html_tree_t *html_tree, char* title, int size);

/**
 * @brief 提取摘要
 **/
int html_tree_extract_abstract(html_tree_t *tree, char *abstract, int size, int merge = 1);

/**
 * @brief 提取链接
 * @return the number of links extracted
 **/
int html_tree_extract_link(html_tree_t *html_tree, char* baseUrl, link_t* link, int& num);

int html_tree_extract_link(html_node_list_t* list, char* baseUrl, link_t* link, int& num);

/**
 * @brief 提取摘要
 **/
int html_tree_extract_keywords(html_tree_t *tree, char *keywords, int size, int merge = 1);

/**
 * @brief 提取css链接
 **/
int html_tree_extract_csslink(html_tree_t *html_tree, const char* baseUrl, link_t* link, int& num);

/**
 * @brief 提取某一节点及其下的内容
 **/
int html_node_extract_content(html_node_t *html_node, char* content, int size);
int html_combine_url(char *result_url, const char *src, char *base_domain, char *base_path, char *base_port);
//获取页面内base tag所指定的URL
int get_base_url(char *base_url, html_tree_t *html_tree);
#endif /* EASOU_HTML_EXTRACTOR_H_ */
