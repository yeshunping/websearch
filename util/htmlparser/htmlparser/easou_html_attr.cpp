/**
 * @file easou_html_attr.cpp
 * @author xunwu
 * @date 2011/08/02
 * @version 1.0
 * @brief 处理easou自定义属性.
 *
 **/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "easou_html_dom.h"
#include "easou_html_attr.h"
#include "easou_debug.h"

#define EASOU_ATTR "easou_mask"

#define EASOU_MASK_BEGIN	(" "EASOU_ATTR"=\"")		  /**< easou属性开始标识 */

static char tag_char[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //!  %  /
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //?
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //A-Z
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,	0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //a-z
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static inline bool is_start_tag(const char *str)
{
	return str[0] == '<' && tag_char[(unsigned char) str[1]];
}

/**
 * @brief createAttribute
 **/
html_attribute_t* html_tree_create_attribute_by_name(html_tree_t *tree, const char *name)
{
	html_attribute_t *attr = NULL;
	html_attr_type_t type = ATTR_UNKNOWN;
	if (name == NULL)
	{
		return NULL;
	}
	assert(tree);
	type = get_attr_type(name, strlen(name));
	attr = html_tree_create_attribute_by_tag(tree, type);
	if (attr == NULL)
	{
		return NULL;
	}
	if (type == ATTR_UNKNOWN)
	{
		attr->name = name;
	}
	return attr;
}

/**
 * @brief createAttribute by attribute type
 * @note internal use only
 **/
html_attribute_t* html_tree_create_attribute_by_tag(html_tree_t *tree, html_attr_type_t type)
{
	struct html_tree_impl_t *self = NULL;
	html_attribute_t *attr = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	attr = (html_attribute_t*) palloc(self->ht_pool, sizeof(*attr));
	if (attr == NULL)
	{
		return NULL;
	}
	attr->type = type;
	attr->next = NULL;
	if (type != ATTR_UNKNOWN)
	{
		attr->name = get_attr_name(type);
	}
	else
	{
		attr->name = NULL;
	}
	attr->value = "";
	attr->valuelength = 0;
	return attr;
}

/**
 * @brief getAttributeNode
 **/
html_attribute_t* html_node_get_attribute_by_name(html_node_t *node, const char *name)
{
	html_attribute_t *attr = NULL;
	if (name == NULL)
	{
		return NULL;
	}
	for (attr = node->html_tag.attribute; attr; attr = attr->next)
	{
		if (strcasecmp(attr->name, name) == 0)
		{
			return attr;
		}
	}
	return NULL;
}

html_attribute_t* html_node_get_attribute_by_type(html_node_t *node, html_attr_type_t type)
{
	html_attribute_t *attr = NULL;
	for (attr = node->html_tag.attribute; attr; attr = attr->next)
	{
		if (attr->type == type)
			return attr;
	}
	return NULL;
}

/**
 * @brief setAttributeNode
 **/
html_attribute_t* html_node_set_attribute_by_name(html_node_t *node, html_attribute_t *attr)
{
	html_attribute_t *old = NULL;
	assert(node);
	if (attr == NULL)
	{
		return NULL;
	}
	/**需要判断是否有同样的attribute，如果有的话，只进行更新，不生成新的attribute节点*/
	old = html_node_get_attribute_by_type(node, attr->type);
	if (old != NULL)
	{
		old->value = attr->value;
		old->valuelength = attr->valuelength;
		return old;
	}
	else
	{/**否则添加新的attribute节点*/
		attr->next = node->html_tag.attribute;
		node->html_tag.attribute = attr;

		if (attr->type == ATTR_ID)
			node->html_tag.attr_id = attr;
		else if (attr->type == ATTR_CLASS)
			node->html_tag.attr_class = attr;

		return attr;
	}
}

/**
 * @brief removeAttributeNode
 **/
html_attribute_t* html_node_remove_attribute(html_node_t *node, html_attribute_t *attr)
{
	html_attribute_t *target = NULL;
	if (node->html_tag.attribute == attr)
	{
		node->html_tag.attribute = attr->next;
		return attr;
	}
	target = node->html_tag.attribute;
	while (target && target->next != attr)
	{
		target = target->next;
	}
	if (target)
	{
		target->next = attr->next;
	}
	return attr;
}

/**
 * @brief getAttribute
 **/
char* html_node_get_attribute_value(html_node_t *node, const char *name)
{
	html_attribute_t *attr = NULL;
	assert(node);
	attr = html_node_get_attribute_by_name(node, name);
	if (attr)
	{
		return attr->value;
	}
	else
	{
		return NULL;
	}
}

/**
 * @brief removeAttribute
 **/
int html_node_remove_attribute_by_name(html_node_t *node, const char *name)
{
	html_attribute_t *attr = NULL;
	assert(node);
	attr = html_node_get_attribute_by_name(node, name);
	if (attr)
	{
		return html_node_remove_attribute(node, attr) != NULL ? 0 : -1;
	}
	else
	{
		return -1;
	}
}

/**
 * @brief Get attribute value from tag
 **/
char* get_attribute_value(html_tag_t *tag, const char *name)
{
	return html_node_get_attribute_value((html_node_t*) tag, name);
}
/**
 * @brief Get attribute value from tag
 **/
char* get_attribute_value(html_tag_t *tag, html_attr_type_t type)
{
	return html_node_get_attribute_value((html_node_t*) tag, get_attr_name(type));
}

/**
 * @brief Get attribute value from tag
 **/
char* get_attribute_value(html_tree_t *tree, html_tag_t *tag, const char *name)
{
	return get_attribute_value(tag, name);
}

