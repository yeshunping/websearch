/**
 * @file html_attr.h

 * @date 2011/08/02
 * @version 1.0
 * @brief 对属性的描述，包含easou自定义属怄1�7.
 *  
 **/

#ifndef  __EASOU_HTML_ATTR_H_
#define  __EASOU_HTML_ATTR_H_

#include "html_dom.h"

/**
 * @brief createAttribute
 **/
html_attribute_t* html_tree_create_attribute_by_name(html_tree_t *tree, const char *name);

/**
 * @brief createAttribute by attribute type
 * @note internal use only
 **/
html_attribute_t* html_tree_create_attribute_by_tag(html_tree_t *tree, html_attr_type_t type);

/**
 * @brief getAttributeNode
 **/
html_attribute_t* html_node_get_attribute_by_name(html_node_t *node, const char *name);

html_attribute_t* html_node_get_attribute_by_type(html_node_t *node, html_attr_type_t type);

/**
 * @brief setAttributeNode
 **/
html_attribute_t* html_node_set_attribute_by_name(html_node_t *node, html_attribute_t *attribute);

/**
 * @brief removeAttributeNode
 **/
html_attribute_t* html_node_remove_attribute(html_node_t *node, html_attribute_t *attribute);

/**
 * @brief getAttribute
 **/
char* html_node_get_attribute_value(html_node_t *node, const char *name);

/**
 * @brief removeAttribute
 **/
int html_node_remove_attribute_by_name(html_node_t *node, const char *name);

/**
 * @brief get Attribute value
 **/
char *get_attribute_value(html_tag_t *html_tag, const char *name);
char *get_attribute_value(html_tag_t *html_tag, html_attr_type_t type);
char *get_attribute_value(html_tree_t * html_tree, html_tag_t *html_tag, const char *name);

#endif  //__EASOU_HTML_ATTR_H_
