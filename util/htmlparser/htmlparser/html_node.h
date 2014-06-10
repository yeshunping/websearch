#ifndef EASOU_HTML_NODE_H_
#define EASOU_HTML_NODE_H_

#include "easou_html_dom.h"
#include"easou_html_parser.h"

/**
 * @brief createElement
 **/
html_node_t* html_tree_create_element(html_tree_t *tree, const char *tagName);

/**
 * @brief createElement by tag type
 **/
html_node_t* html_tree_create_element_by_tag(html_tree_t*tree, html_tag_type_t tagName);

/**
 * @brief createTextNode
 **/
html_node_t* html_tree_create_text_node(html_tree_t *tree, char *text);

/**
 * @brief createComment
 **/
html_node_t* html_tree_create_comment(html_tree_t *tree, char *comment);

/**
 * @brief createDoctype
 **/
html_node_t* html_tree_create_doctype(html_tree_t *tree, char *doctype);

/**
 * @brief Destroy node
 **/
void html_node_destroy(html_node_t *node);

/**
 * @brief Destroy node of document
 **/
void html_tree_destroy_node(html_tree_t *tree, html_node_t *node);

/**
 * @brief appendChild
 **/
html_node_t* html_node_append_child(html_node_t *parent, html_node_t *node);

/**
 * @brief insertBefore
 **/
html_node_t* html_node_insert_before(html_node_t *parent, html_node_t *node, html_node_t *ref);
/**
 * @brief removeChild
 **/
html_node_t* html_node_remove_child(html_node_t *parent, html_node_t *node);

#endif /* EASOU_HTML_NODE_H_ */
