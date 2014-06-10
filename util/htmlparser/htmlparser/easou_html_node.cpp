#include "assert.h"
#include "string.h"
#include "ctype.h"
#include "easou_html_node.h"

SLAB_DEFINE(node, _html_node_t);

/**
 * @brief createElement
 **/
html_node_t* html_tree_create_element(html_tree_t *tree, const char *name)
{
	struct html_tree_impl_t *self = NULL;
	html_node_t *node = NULL;
	char *text = NULL;
	int i = 0;
	int len = 0;
	html_tag_type_t tag = TAG_UNKNOWN;

	if (name == NULL)
	{
		return NULL;
	}
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	len = strlen(name);
	tag = get_tag_type(name, len);
	node = html_tree_create_element_by_tag(tree, tag);
	if (node == NULL)
	{
		return NULL;
	}
	if (tag != TAG_UNKNOWN)
	{
		node->html_tag.tag_name = get_tag_name(tag);
	}
	else
	{
		text = html_tree_strndup(tree, name, len);
		if (text == NULL)
		{
			return NULL;
		}
		for (i = 0; i < len; i++)
		{
			text[i] = tolower(text[i]);
		}
		node->html_tag.tag_name = text;
	}
	return node;
}

/**
 * @brief createElement by tagType
 * @note internal use only
 **/
html_node_t* html_tree_create_element_by_tag(html_tree_t *tree, html_tag_type_t tag)
{
	struct html_tree_impl_t *self = NULL;
	html_node_t *node = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	node = salloc_node(self->ht_node_slab);
	if (node == NULL)
	{
		return NULL;
	}
	node->owner = tree;
	node->html_tag.page_offset = -1;
	node->html_tag.tag_code = self->ht_tag_code++;
	node->html_tag.tag_type = tag;
	node->html_tag.html_node = node;
	return node;
}

/**
 * @brief createTextNode
 **/
html_node_t* html_tree_create_text_node(html_tree_t *tree, char *text)
{
	html_node_t *node = NULL;
	assert(tree);
	node = html_tree_create_element_by_tag(tree, TAG_PURETEXT);
	if (node == NULL)
	{
		return NULL;
	}
	node->html_tag.text = text;
	return node;
}

/**
 * @brief createComment
 **/
html_node_t* html_tree_create_comment(html_tree_t *tree, char *text)
{
	html_node_t *node = NULL;
	assert(tree);
	node = html_tree_create_element_by_tag(tree, TAG_COMMENT);
	if (node == NULL)
	{
		return NULL;
	}
	node->html_tag.text = text;
	return node;
}

/**
 * @brief createDoctype
 **/
html_node_t* html_tree_create_doctype(html_tree_t *tree, char *text)
{
	html_node_t *node = NULL;
	assert(tree);
	node = html_tree_create_element_by_tag(tree, TAG_DOCTYPE);
	if (node == NULL)
	{
		return NULL;
	}
	node->html_tag.text = text;
	return node;
}

/**
 * @brief Destroy node
 **/
void html_node_destroy(html_node_t *node)
{
	if (node)
	{
		html_tree_destroy_node(node->owner, node);
	}
}

/**
 * @brief destroy node
 **/
void html_tree_destroy_node(html_tree_t *tree, html_node_t *node)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	if (node)
	{
		sfree_node(self->ht_node_slab, node);
	}
}

/**
 * @brief appendChild
 **/
html_node_t* html_node_append_child(html_node_t *parent, html_node_t *node)
{
	assert(parent);
	assert(node);
	if (node->parent)
	{
		html_node_remove_child(node->parent, node);
	}
	if (!parent->child)
	{
		parent->child = node;
		parent->last_child = node;
	}
	else
	{
		assert(parent->last_child);
		parent->last_child->next = node;
		node->prev = parent->last_child;
		parent->last_child = node;
	}
	node->parent = parent;
	node->next = NULL;

	return node;
}

/**
 * @brief insertBefore
 **/
html_node_t* html_node_insert_before(html_node_t *parent, html_node_t *node, html_node_t *ref)
{
	html_node_t *prev = NULL;
	assert(parent);
	assert(ref);
	assert(parent->child);
	assert(ref->parent == parent);
	if (!node)
	{
		return NULL;
	}
	if (node->parent)
	{
		html_node_remove_child(node->parent, node);
	}
	node->parent = parent;
	prev = ref->prev;
	node->next = ref;
	ref->prev = node;
	if (prev)
	{
		prev->next = node;
		node->prev = prev;
	}
	else
	{
		parent->child = node;
		node->prev = NULL;
	}
	return node;
}

/**
 * @brief removeChild
 **/
html_node_t* html_node_remove_child(html_node_t *parent, html_node_t *node)
{
	if (!node)
	{
		return NULL;
	}
	assert(parent);
	assert(node->parent == parent);
	assert(parent->child);
	assert(parent->last_child);

	node->parent = NULL;
	if (node->prev)
	{
		node->prev->next = node->next;
	}
	if (node->next)
	{
		node->next->prev = node->prev;
	}
	if (parent->child == node)
	{
		parent->child = node->next;
	}
	if (parent->last_child == node)
	{
		parent->last_child = node->prev;
	}
	node->prev = NULL;
	node->next = NULL;
	return node;
}
