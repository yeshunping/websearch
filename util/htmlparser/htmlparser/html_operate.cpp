#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "easou_html_dom.h"
#include "easou_html_attr.h"
#include "easou_html_node.h"
#include "easou_html_script.h"
#include "easou_html_tokenizer.h"
#include "easou_html_constructor.h"
#include "easou_html_operate.h"

extern struct html_tag_category_t g_html_tag_category_map[];

/**
 * @brief Get node in given stack top
 **/
#define STACK(parser) (SLIST_FIRST((parser)->hp_stack)->si_node)

/**
 * @brief Check node is the given type
 **/
#define IS_A(node, tag) ((node)->html_tag.tag_type == (tag))

/**
 * @brief Check given node x y is same type
 **/
#define IS_MATCH(x, y) ((x)->html_tag.tag_type == (y)->html_tag.tag_type && ((x)->html_tag.tag_type == TAG_UNKNOWN ? strcmp((x)->html_tag.tag_name, (y)->html_tag.tag_name) == 0 : 1))
/**
 * @brief is special tag
 **/
#define is_special_tag(tag) (g_html_tag_category_map[(tag)].tc_is_special)
/**
 * @brief is formatting tag
 **/
#define is_formatting_tag(tag) (g_html_tag_category_map[(tag)].tc_is_formatting)
/**
 * @brief is scope tag
 **/
#define is_scope_tag(tag) (g_html_tag_category_map[(tag)].tc_is_scope)

SLAB_DEFINE(stack, stack_item_t);
SLAB_DEFINE(foster, foster_item_t);

static int is_in_stack(struct html_stack_t *stack, html_node_t *node)
{
	struct stack_item_t *item = NULL;
	assert(stack);
	assert(node);
	SLIST_FOREACH(item, stack, si_entries)
	{
		if (item->si_node == node)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * @brief HTML5 Constructor meta code append_no_stack
 **/
int do_append_no_stack(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	html_node_t *parent = NULL;
	assert(parser);
	assert(next);

	// shuangwei add 2012-03-23 for delete whitespace node
	stack = parser->hp_stack;
	assert(stack);
	parent = SLIST_FIRST(stack)->si_node;
	assert(parent);
	assert(!(next->html_tag.is_close_tag));
	if (next->html_tag.tag_type == TAG_WHITESPACE)
	{
		if (parser->hp_ignore_space)
		{
			return 0;
		}
		else
		{
			//whitespace node can't be the child of the nodes ,like:table,tr,select,ul,dl,thead,tbody,tfoot,head,html
			if (TAG_TABLE == parent->html_tag.tag_type || TAG_TR == parent->html_tag.tag_type || TAG_SELECT == parent->html_tag.tag_type || TAG_UL == parent->html_tag.tag_type || TAG_DL == parent->html_tag.tag_type || TAG_THEAD == parent->html_tag.tag_type || TAG_TBODY == parent->html_tag.tag_type || TAG_TFOOT == parent->html_tag.tag_type || TAG_HEAD == parent->html_tag.tag_type || TAG_HTML == parent->html_tag.tag_type)
			{
				return 0;
			}
			next->html_tag.tag_type = TAG_PURETEXT;
			if (next->html_tag.text && strlen(next->html_tag.text) > 1)
			{
				next->html_tag.text[0] = ' ';
				next->html_tag.text[1] = '\0';
			}
		}

	}
	// shuangwei add 2012-03-23 for delete whitespace node

// shuangwei delete 2012-03-23
//	if (next->html_tag.tag_type == TAG_WHITESPACE) {
//		next->html_tag.tag_type = TAG_PURETEXT;
//		if (parser->hp_ignore_space) {
//			return 0;
//		}
//	}

//	stack = parser->hp_stack;
//	assert(stack);
//	parent = SLIST_FIRST(stack)->si_node;
//	assert(parent);
//	assert(!next->html_tag.is_close_tag);
	/* TODO merge adjacent text nodes */
	// shuangwei delete 2012-03-23
	html_node_append_child(parent, next);
	return 0;
}

/**
 * @brief HTML5 Constructor meta code append
 **/
int do_append(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	int ret = 0;
	ret = do_append_no_stack(parser, next);
	if (ret != 0)
	{
		return ret;
	}
	if (IS_A(next, TAG_BODY) || IS_A(next, TAG_WAP_CARD) && parser->hp_body == NULL)
	{
		parser->hp_body = next;
	}
	else if (IS_A(next, TAG_HTML) || IS_A(next, TAG_WAP_WML))
	{
		parser->hp_html = next;
	}
	if ((parser->hp_xml_compatible || parser->hp_wml_compatible) && next->html_tag.is_self_closed)
	{
		return 0;
	}
	stack = parser->hp_stack;
	item = salloc_stack(parser->hp_stack_slab);
	if (item == NULL)
	{
		return -1;
	}
	item->si_node = next;
	SLIST_INSERT_HEAD(stack, item, si_entries);
	return 0;
}

/**
 * @brief HTML5 Constructor meta code switch
 **/
int do_switch(struct html_parser_t *parser, html_node_t *next, state_handler_t handler)
{
	assert(parser);
	assert(handler);
	parser->hp_handler = handler;
	return 0;
}

/**
 * @brief HTML5 Constructor meta code popup
 **/
int do_popup(struct html_parser_t *parser, html_node_t *next)
{
	struct stack_item_t *top = NULL;
	assert(parser);
	top = SLIST_FIRST(parser->hp_stack);
	//if(next->html_tag.tag_type==top->si_node->html_tag.tag_type)
	{
		SLIST_REMOVE_HEAD(parser->hp_stack, si_entries);
		sfree_stack(parser->hp_stack_slab, top);
		assert(!SLIST_EMPTY(parser->hp_stack));
	}

	return 0;
}

/**
 * @brief HTML5 Constructor meta code ignore 忽略
 **/
int do_ignore(struct html_parser_t *parser, html_node_t *next)
{
	return 0;
}

/**
 * @brief HTML5 Constructor meta code error 错误
 **/
int do_error(struct html_parser_t *parser, html_node_t *next)
{
	return -1;
}

/**
 * @brief HTML5 Constructor meta code warning  警告
 **/
int do_warning(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	assert(next);
	assert(STACK(parser));
	return 0;
}

/**
 * @brief HTML5 Constructor meta code redo
 **/
int do_redo(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	return parser->hp_handler(parser, next);
}

/**
 * @brief HTML5 Constructor meta code as_if
 **/
int do_as_if(struct html_parser_t *parser, html_node_t *next, html_tag_type_t tag)
{
	html_node_t *ins = NULL;
	assert(parser);
	ins = html_tree_create_element_by_tag(parser->hp_tree, tag);
	if (ins == NULL)
	{
		return -1;
	}
	ins->html_tag.tag_name = get_tag_name(tag);
	return do_redo(parser, ins);
}

/**
 * @brief HTML5 Constructor meta code as_if_end
 **/
int do_as_if_end(struct html_parser_t *parser, html_node_t *next, html_tag_type_t tag)
{
	html_node_t *ins = NULL;
	assert(parser);
	ins = html_tree_create_element_by_tag(parser->hp_tree, tag);
	if (ins == NULL)
	{
		return -1;
	}
	ins->html_tag.is_close_tag = 1;
	ins->html_tag.tag_name = get_tag_name(tag);
	//shuangwei add 20120530
	if (next)
	{
		ins->html_tag.page_offset = next->html_tag.page_offset;
	}

	return do_redo(parser, ins);
}

/**
 * @brief HTML5 Constructor meta code as_if_end_this
 **/
int do_as_if_end_this(struct html_parser_t *parser, html_node_t *next)
{
	return do_as_if_end(parser, next, STACK(parser)->html_tag.tag_type);
}

/**
 * @brief HTML5 Constructor meta code record_head
 **/
int do_record_head(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	assert(next->html_tag.tag_type == TAG_HEAD);
	parser->hp_tree->head = next;
	return 0;
}

/**
 * @brief HTML5 Constructor meta code forward
 **/
int do_forward(struct html_parser_t *parser, html_node_t *next, state_handler_t handler)
{
	assert(handler);
	return handler(parser, next);
}

/**
 * @brief HTML5 Constructor meta code assert_in_tag
 **/
int do_assert_in_tag(struct html_parser_t *parser, html_node_t *next, html_tag_type_t tag)
{
	return do_is(parser, next, tag);
}

/**
 * @brief HTML5 Constructor meta code save
 **/
int do_save(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	parser->hp_last_handler = parser->hp_handler;
	return 0;
}

/**
 * @brief HTML5 Constructor meta code restore
 **/
int do_restore(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	assert(parser->hp_last_handler);
	parser->hp_handler = parser->hp_last_handler;
	return 0;
}

/**
 * @brief HTML5 Constructor meta code enter_head
 **/
int do_enter_head(struct html_parser_t *parser, html_node_t *next)
{
	struct stack_item_t *item = NULL;
	assert(parser);
	assert(parser->hp_tree->head);
	item = salloc_stack(parser->hp_stack_slab);
	if (item == NULL)
	{
		return -1;
	}
	item->si_node = parser->hp_tree->head;
	SLIST_INSERT_HEAD(parser->hp_stack, item, si_entries);
	return 0;
}

/**
 * @brief HTML5 Constructor meta code exit_head
 **/
int do_exit_head(struct html_parser_t *parser, html_node_t *next)
{
	struct stack_item_t *top = NULL;
	assert(parser);
	assert(!SLIST_EMPTY(parser->hp_stack));
	top = SLIST_FIRST(parser->hp_stack);
	SLIST_REMOVE_HEAD(parser->hp_stack, si_entries);
	sfree_stack(parser->hp_stack_slab, top);
	assert(!SLIST_EMPTY(parser->hp_stack));
	return 0;
}

/**
 * @brief HTML5 Constructor meta code as_if_this_end
 **/
int do_as_if_this_end(struct html_parser_t *parser, html_node_t *next)
{
	assert(next);
	return do_as_if_end(parser, next, next->html_tag.tag_type);
}

/**
 * @brief HTML5 Constructor meta code close_tag
 **/
int do_close_tag(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	html_tag_type_t tag = TAG_UNKNOWN;
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;

	assert(parser);
	assert(next);
	cal_node_length(parser, next);
	tag = next->html_tag.tag_type;
	stack = parser->hp_stack;

	current = SLIST_FIRST(stack);
	if (!IS_MATCH(current->si_node, next))
	{
		ret = do_warning(parser, next);
		if (ret != 0)
		{
			return ret;
		}
	}
	while (!SLIST_EMPTY(stack))
	{
		current = SLIST_FIRST(stack);
		if (current->si_node && next->html_tag.tag_name && next->html_tag.page_offset >= 0 && current->si_node->html_tag.nodelength >= 0)
		{
			int thistaglength = strlen(next->html_tag.tag_name) + 3;
			current->si_node->html_tag.nodelength = next->html_tag.page_offset - thistaglength - current->si_node->html_tag.page_offset;
			if (current->si_node->html_tag.nodelength < 0)
			{
				current->si_node->html_tag.nodelength = 0;
			}
		}
		if (current->si_node && IS_MATCH(current->si_node, next))
		{
			SLIST_REMOVE_HEAD(stack, si_entries);
			sfree_stack(parser->hp_stack_slab, current);
			break;
		}
		SLIST_REMOVE_HEAD(stack, si_entries);
		sfree_stack(parser->hp_stack_slab, current);
	}
	assert(!SLIST_EMPTY(stack));
	return 0;
}

/**
 * @brief HTML5 Constructor meta code close_these_tags
 **/
int do_close_these_tags(struct html_parser_t *parser, html_node_t *next, ...)
{
	int ret = -1;
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	html_tag_type_t target = TAG_ROOT;
	html_tag_type_t tag = TAG_ROOT;
	va_list ap;

	assert(parser);
	assert(next);

	stack = parser->hp_stack;
	item = SLIST_FIRST(stack);
	if (!IS_MATCH(item->si_node, next))
	{
		ret = do_warning(parser, next);
		if (ret != 0)
		{
			return ret;
		}
	}
	while (!SLIST_EMPTY(stack))
	{
		item = SLIST_FIRST(stack);
		target = item->si_node->html_tag.tag_type;
		va_start(ap, next);
		while ((tag = (html_tag_type_t) va_arg(ap, int)) != TAG_ROOT)
		{
			if (target == tag)
			{
				SLIST_REMOVE_HEAD(stack, si_entries);
				sfree_stack(parser->hp_stack_slab, item);
				va_end(ap);
				return 0;
			}
		}
		va_end(ap);
		SLIST_REMOVE_HEAD(stack, si_entries);
		sfree_stack(parser->hp_stack_slab, item);
	}

	return -1;
}

/**
 * @brief HTML5 Constructor meta code eat_next_newline
 **/
int do_eat_next_newline(struct html_parser_t *parser, html_node_t *next)
{
	return 0;
}

/**
 * @brief HTML5 Constructor meta code is_in_tag
 **/
int do_is_in_tag(struct html_parser_t *parser, html_node_t *next, html_tag_type_t tag)
{
	struct stack_item_t *item = NULL;
	assert(parser);
	SLIST_FOREACH(item, parser->hp_stack, si_entries)
	{
		if (item->si_node->html_tag.tag_type == tag)
		{
			return 0;
		}
	}
	return -1;
}

/**
 * @brief HTML5 Constructor meta code is_in_these_tags
 **/
int do_is_in_these_tags(struct html_parser_t *parser, html_node_t *next, ...)
{
	html_tag_type_t stag = TAG_UNKNOWN;
	html_tag_type_t tag = TAG_ROOT;
	va_list ap;
	assert(parser);
	stag = STACK(parser)->html_tag.tag_type;
	va_start(ap, next);
	while ((tag = (html_tag_type_t) va_arg(ap, int)) != TAG_ROOT)
	{
		if (stag == tag)
		{
			va_end(ap);
			return 0;
		}
	}
	va_end(ap);
	return -1;
}

/**
 * @brief HTML5 Constructor meta code merge_attr
 **/
int do_merge_attr(struct html_parser_t *parser, html_node_t *next)
{
	html_node_t *target = NULL;
	html_attribute_t *attr = NULL;

	assert(parser);
	assert(next);

//	printf("%s\n", next->html_tag.tag_name);
	if (IS_A(next, TAG_HTML))
	{
		target = parser->hp_html;
	}
	else if (IS_A(next, TAG_BODY))
	{
		target = parser->hp_body;
	}
	else
	{
		return -1;
	}
	while ((attr = next->html_tag.attribute) != NULL)
	{
		html_node_remove_attribute(next, attr);
		html_node_set_attribute_by_name(target, attr);
	}
	return 0;
}

/**
 * @brief HTML5 Constructor meta code proc_body_a
 **/
int do_proc_body_a(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	struct html_stack_t *actfmt = NULL;
	struct stack_item_t *item = NULL;
	assert(parser);
	actfmt = parser->hp_actfmt_list;
	SLIST_FOREACH(item, actfmt, si_entries)
	{
		if (!item->si_node)
		{
			break;
		}
		if (item->si_node->html_tag.tag_type == TAG_A)
		{
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end(parser, next, TAG_A);
			if (ret != 0)
			{
				return ret;
			}
			return 0;
		}
	}
	return 0;
}

/**
 * @brief HTML5 Constructor meta code proc_body_dd_dt
 **/
int do_proc_body_dd_dt(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	struct stack_item_t *current = NULL;
	html_node_t *node = NULL;
	assert(parser);
	assert(next);
	SLIST_FOREACH(current, parser->hp_stack, si_entries)
	{
		node = current->si_node;
		if (IS_A(node, TAG_DD))
		{
			ret = do_as_if_end(parser, next, TAG_DD);
			if (ret != 0)
			{
				return ret;
			}
			break;
		}
		if (IS_A(node, TAG_DT))
		{
			ret = do_as_if_end(parser, next, TAG_DT);
			if (ret != 0)
			{
				return ret;
			}
			break;
		}
		if (is_special_tag(node->html_tag.tag_type) && !IS_A(node, TAG_ADDRESS) && !IS_A(node, TAG_DIV) && !IS_A(node, TAG_P))
		{
			break;
		}
	}
	if (do_has_p_in_button_scope(parser, next))
	{
		ret = do_as_if_end(parser, next, TAG_P);
		if (ret != 0)
		{
			return ret;
		}
	}
	return do_append(parser, next);
}

/**
 * @brief HTML5 Constructor meta code proc_body_end
 **/
int do_proc_body_end(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	struct stack_item_t *current = NULL;
	struct stack_item_t *tmp = NULL;

	assert(parser);
	assert(next);

	stack = parser->hp_stack;
	SLIST_FOREACH_MUTABLE(item, stack, si_entries, tmp)
	{
		if (IS_MATCH(item->si_node, next))
		{
			while (!SLIST_EMPTY(stack))
			{
				current = SLIST_FIRST(stack);
				SLIST_REMOVE_HEAD(stack, si_entries);
				if (parser->hp_actfmt_list && parser->hp_actfmt_list->slh_first && parser->hp_actfmt_list->slh_first->si_node && IS_MATCH(parser->hp_actfmt_list->slh_first->si_node,current->si_node))
				{
					struct stack_item_t *tmpact = SLIST_FIRST(parser->hp_actfmt_list);
					SLIST_REMOVE_HEAD(parser->hp_actfmt_list, si_entries);
					sfree_stack(parser->hp_stack_slab, tmpact);

				}
				if (current == item)
				{
					sfree_stack(parser->hp_stack_slab, current);
					break;
				}
				else
				{
					sfree_stack(parser->hp_stack_slab, current);
				}
			}
			return 0;
		}
		else if (is_special_tag(item->si_node->html_tag.tag_type))
		{
			return do_warning(parser, next);
		}
	}

	return 0;
}

/**
 * @brief HTML5 Constructor meta code proc_body_form
 **/
int do_proc_body_form(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	assert(parser);
	assert(next);
	if (parser->hp_form)
	{
		return do_warning(parser, next);
	}
	else
	{
		if (do_has_p_in_button_scope(parser, next))
		{
			ret = do_as_if_end(parser, next, TAG_P);
			if (ret != 0)
			{
				return ret;
			}
		}
		ret = do_append(parser, next);
		parser->hp_form = next;
		return ret;
	}
	return ret;
}

/**
 * @brief HTML5 Constructor meta code proc_body_formatting_elements_end
 **/
int do_proc_body_formatting_elements_end(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	struct html_stack_t *stack = NULL;
	struct html_stack_t *actfmt = NULL;
	struct stack_item_t *item = NULL; /* iteration variable */
	struct stack_item_t *begin = NULL; /* respective open node in actfmt */
	struct stack_item_t *mid = NULL; /* non-formatting node before begin */
	struct stack_item_t *last = NULL; /* latest used stack item */
	struct stack_item_t *ins = NULL; /* new created stack item */
	struct stack_item_t *tmp = NULL; /* temp variable, used to free stack item */
	html_node_t *node = NULL;

	assert(parser);
	assert(next);
	stack = parser->hp_stack;
	actfmt = parser->hp_actfmt_list;

	/* Find the begin node from active formatting list.
	 * Error if not satisfied.
	 */SLIST_FOREACH(item, actfmt, si_entries)
	{
		if (item->si_node == NULL)
		{
			break;
		}
		if (IS_A(item->si_node, next->html_tag.tag_type))
		{
			begin = item;
			break;
		}
	}
	if (begin == NULL || (is_in_stack(stack, begin->si_node) && do_has_this_tag_in_scope(parser, begin->si_node) != 0))
	{
		return do_warning(parser, next);
	}
	if (begin && !is_in_stack(stack, begin->si_node))
	{
		SLIST_REMOVE(actfmt, begin, stack_item_t, si_entries);
		sfree_stack(parser->hp_stack_slab, begin);
		return do_warning(parser, next);
	}

	/* find the last non-formatting node before begin in stack */
	if (begin->si_node != SLIST_FIRST(stack)->si_node)
	{
		ret = do_warning(parser, next);
		if (ret != 0)
		{
			return ret;
		}
	}
	SLIST_FOREACH(item, stack, si_entries)
	{
		if (item->si_node == begin->si_node)
		{
			break;
		}
		if (!is_formatting_tag(item->si_node->html_tag.tag_type))
		{
			mid = item;
		}
	}

	if (mid)
	{
		/* step 1: promote the non-formatting element */
		html_node_append_child(begin->si_node->parent, mid->si_node);

		/* step 2: close elements between begin (including) and mid in stack */
		while ((item = SLIST_NEXT(mid, si_entries))->si_node != begin->si_node)
		{
			SLIST_REMOVE_NEXT(stack, mid, si_entries);
			sfree_stack(parser->hp_stack_slab, item);
		}
		assert(SLIST_NEXT(mid, si_entries)->si_node == begin->si_node);
		item = SLIST_NEXT(mid, si_entries);
		SLIST_REMOVE_NEXT(stack, mid, si_entries);
		sfree_stack(parser->hp_stack_slab, item);

		/* step 3: reopen under mid */SLIST_FOREACH(item, stack, si_entries)
		{
			if (SLIST_NEXT(item, si_entries) == mid)
			{
				last = item;
			}
		}
		last = NULL;
		SLIST_FOREACH(item, actfmt, si_entries)
		{
			node = html_tree_create_element_by_tag(parser->hp_tree, item->si_node->html_tag.tag_type);
			if (node == NULL)
			{
				return -1;
			}
			node->html_tag.tag_name = get_tag_name(item->si_node->html_tag.tag_type);
			//while (mid->si_node->child)
			if (mid->si_node->child)
			{
				html_node_append_child(node, mid->si_node->child);
			}
			html_node_append_child(mid->si_node, node);
			if (item == begin)
			{
				/* step 4: reopen after mid */
				while ((tmp = SLIST_FIRST(stack)) != mid)
				{
					SLIST_REMOVE_HEAD(stack, si_entries);
					sfree_stack(parser->hp_stack_slab, tmp);
				}
				last = NULL;
				SLIST_FOREACH(item, actfmt, si_entries)
				{
					if (item == begin)
					{
						SLIST_REMOVE(actfmt, item, stack_item_t, si_entries);
						sfree_stack(parser->hp_stack_slab, item);
						break;
					}
					ins = salloc_stack(parser->hp_stack_slab);
					if (ins == NULL)
					{
						return -1;
					}
					ins->si_node = html_tree_create_element_by_tag(parser->hp_tree, item->si_node->html_tag.tag_type);
					if (ins->si_node == NULL)
					{
						return -1;
					}
					ins->si_node->html_tag.tag_name = get_tag_name(item->si_node->html_tag.tag_type);
					if (last)
					{
						html_node_append_child(last->si_node->parent, ins->si_node);
						html_node_append_child(ins->si_node, last->si_node);
						SLIST_INSERT_AFTER(last, ins, si_entries);
					}
					else
					{
						html_node_append_child(SLIST_FIRST(stack)->si_node, ins->si_node);
						SLIST_INSERT_HEAD(stack, ins, si_entries);
					}
					last = ins;
				}
				break;
			}
		}
	}
	else
	{
		/*
		 * Here we have to do four actions:
		 * 1. Pop stack until begin (including) in stack, aka close
		 * 2. Push stack until begin in active formatting list, aka reopen
		 * 3. Promote the lastest pushed item node as begin's nextSibling
		 * 4. Remove begin from active formatting list
		 */
		/* step 1 */
		for (item = SLIST_FIRST(stack); item; item = SLIST_FIRST(stack))
		{
			SLIST_REMOVE_HEAD(stack, si_entries);
			if (item->si_node == begin->si_node)
			{
				sfree_stack(parser->hp_stack_slab, item);
				break;
			}
			else
			{
				sfree_stack(parser->hp_stack_slab, item);
			}
		}
		/* step 2 */SLIST_FOREACH(item, actfmt, si_entries)
		{
			if (item == begin)
			{
				break;
			}
			ins = salloc_stack(parser->hp_stack_slab);
			if (ins == NULL)
			{
				return -1;
			}
			ins->si_node = html_tree_create_element_by_tag(parser->hp_tree, item->si_node->html_tag.tag_type);
			if (ins->si_node == NULL)
			{
				return -1;
			}
			ins->si_node->html_tag.tag_name = get_tag_name(item->si_node->html_tag.tag_type);
			if (!last)
			{
				html_node_append_child(SLIST_FIRST(stack)->si_node, ins->si_node);
				SLIST_INSERT_HEAD(stack, ins, si_entries);
			}
			else
			{
				html_node_append_child(ins->si_node, last->si_node);
				html_node_append_child(SLIST_NEXT(last, si_entries)->si_node, ins->si_node);
				SLIST_INSERT_AFTER(last, ins, si_entries);
			}
			last = ins;
			item->si_node = ins->si_node;
		}
		/* step 3 */
		if (last)
		{
			html_node_append_child(begin->si_node->parent, last->si_node);
		}
		/* step 4 */
		SLIST_REMOVE(actfmt, begin, stack_item_t, si_entries);
		sfree_stack(parser->hp_stack_slab, begin);
	}

	return 0;
}

/**
 * @brief HTML5 Constructor meta code proc_body_form_end
 **/
int do_proc_body_form_end(struct html_parser_t *parser, html_node_t *next)
{
	html_node_t *form = NULL;
	assert(parser);
	assert(next);
	form = parser->hp_form;
	parser->hp_form = NULL;
	if (form == NULL || do_has_this_tag_in_scope(parser, form) != 0)
	{
		return do_warning(parser, next);
	}
	else
	{
		return do_close_tag(parser, next);
	}
}

/**
 * @brief HTML5 Constructor meta code proc_body_li
 **/
int do_proc_body_li(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	struct stack_item_t *current = NULL;
	html_node_t *node = NULL;
	assert(parser);
	assert(next);
	SLIST_FOREACH(current, parser->hp_stack, si_entries)
	{
		node = current->si_node;
		if (IS_A(node, TAG_LI))
		{
			ret = do_as_if_end(parser, next, TAG_LI);
			if (ret != 0)
			{
				return ret;
			}
			break;
		}
		if (is_special_tag(node->html_tag.tag_type) && !IS_A(node, TAG_ADDRESS) && !IS_A(node, TAG_DIV) && !IS_A(node, TAG_P))
		{
			break;
		}
	}
	if (do_has_p_in_button_scope(parser, next))
	{
		ret = do_as_if_end(parser, next, TAG_P);
		if (ret != 0)
		{
			return ret;
		}
	}
	return do_append(parser, next);
}

/**
 * @brief HTML5 Constructor meta code proc_body_select
 **/
int do_proc_body_select(struct html_parser_t *parser, html_node_t *next)
{
	state_handler_t handler = NULL;
	assert(parser);
	handler = parser->hp_handler;
	if (handler == on_in_table || handler == on_in_caption || handler == on_in_column_group || handler == on_in_table_body || handler == on_in_row || handler == on_in_cell)
	{
		return do_switch(parser, next, on_in_select_in_table);
	}
	else
	{
		return do_switch(parser, next, on_in_select);
	}
}

/**
 * @brief HTML5 Constructor meta code close_to
 **/
int do_close_to(struct html_parser_t *parser, html_node_t *next, ...)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *top = NULL;
	va_list ap;
	html_tag_type_t tag = TAG_ROOT;
	html_tag_type_t test = TAG_ROOT;
	int got = 0;
	assert(parser);
	assert(sizeof(html_tag_type_t) == sizeof(int));
	stack = parser->hp_stack;
	while (!SLIST_EMPTY(stack))
	{
		top = SLIST_FIRST(stack);
		tag = top->si_node->html_tag.tag_type;
		va_start(ap, next);
		while ((test = (html_tag_type_t) va_arg(ap, int)) != TAG_ROOT)
		{
			if (tag == test)
			{
				got = 1;
				va_end(ap);
				break;
			}
		}
		va_end(ap);
		if (got == 1)
		{
			break;
		}
		SLIST_REMOVE_HEAD(stack, si_entries);
		sfree_stack(parser->hp_stack_slab, top);
	}
	assert(!SLIST_EMPTY(stack));
	return 0;
}

/**
 * @brief HTML5 Constructor meta code clear_to_table_row_context
 **/
int do_clear_to_table_row_context(struct html_parser_t *parser, html_node_t *next)
{
	return do_close_to(parser, next, TAG_TR, TAG_HTML, TAG_ROOT);
}

/**
 * @brief HTML5 Constructor meta code clear_to_table_body_context
 **/
int do_clear_to_table_body_context(struct html_parser_t *parser, html_node_t *next)
{
	return do_close_to(parser, next, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_ROOT);
}

/**
 * @brief HTML5 Constructor meta code clear_to_table_context
 **/
int do_clear_to_table_context(struct html_parser_t *parser, html_node_t *next)
{
	return do_close_to(parser, next, TAG_TABLE, TAG_HTML, TAG_ROOT);
}

/**
 * @brief HTML5 Constructor meta code has_this_tag_in_scope
 **/
int do_has_this_tag_in_scope(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;
	html_tag_type_t tag = TAG_UNKNOWN;
	html_tag_type_t test = TAG_UNKNOWN;

	assert(parser);
	assert(next);
	test = next->html_tag.tag_type;
	stack = parser->hp_stack;
	SLIST_FOREACH(current, stack, si_entries)
	{
		tag = current->si_node->html_tag.tag_type;
		if (tag == test)
		{
			return 0;
		}
		if (is_scope_tag(tag))
		{
			return -1;
		}
	}

	return -1;
}

/**
 * @brief HTML5 Constructor meta code has_these_tags_in_scope
 **/
int do_has_these_tags_in_scope(struct html_parser_t *parser, html_node_t *next, ...)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;
	html_tag_type_t tag = TAG_UNKNOWN;
	html_tag_type_t test = TAG_UNKNOWN;
	va_list ap;

	assert(parser);
	assert(next);
	stack = parser->hp_stack;
	SLIST_FOREACH(current, stack, si_entries)
	{
		tag = current->si_node->html_tag.tag_type;
		va_start(ap, next);
		while ((test = (html_tag_type_t) va_arg(ap, int)) != TAG_ROOT)
		{
			if (tag == test)
			{
				va_end(ap);
				return 0;
			}

		}
		va_end(ap);
		if (is_scope_tag(tag))
		{
			return -1;
		}
	}

	return -1;
}

/**
 * @brief HTML5 Constructor meta code has_this_tag_in_list_item_scope
 **/
int do_has_this_tag_in_list_item_scope(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;
	html_tag_type_t tag = TAG_UNKNOWN;
	html_tag_type_t test = TAG_UNKNOWN;

	assert(parser);
	assert(next);
	stack = parser->hp_stack;
	test = next->html_tag.tag_type;
	SLIST_FOREACH(current, stack, si_entries)
	{
		tag = current->si_node->html_tag.tag_type;
		if (tag == test)
		{
			return 0;
		}
		else if (is_scope_tag(tag) || tag == TAG_OL || tag == TAG_UL)
		{
			return -1;
		}
	}

	return -1;

}

/**
 * @brief HTML5 Constructor meta code has_p_in_button_scope
 **/
int do_has_p_in_button_scope(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;
	html_tag_type_t tag = TAG_UNKNOWN;

	assert(parser);
	assert(next);
	stack = parser->hp_stack;
	SLIST_FOREACH(current, stack, si_entries)
	{
		tag = current->si_node->html_tag.tag_type;
		if (tag == TAG_P)
		{
			return 0;
		}
		else if (is_scope_tag(tag) || tag == TAG_BUTTON)
		{
			return -1;
		}
	}

	return -1;
}

/**
 * @brief HTML5 Constructor meta code has_this_tag_in_table_scope
 **/
int do_has_this_tag_in_table_scope(struct html_parser_t *parser, html_node_t *next)
{
	return do_has_these_tags_in_table_scope(parser, next, next->html_tag.tag_type, TAG_ROOT);
}

/**
 * @brief HTML5 Constructor meta code has_these_tags_in_table_scope
 **/
int do_has_these_tags_in_table_scope(struct html_parser_t *parser, html_node_t *next, ...)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;
	html_tag_type_t tag = TAG_UNKNOWN;
	html_tag_type_t test = TAG_UNKNOWN;
	va_list ap;

	assert(parser);
	assert(next);
	stack = parser->hp_stack;
	SLIST_FOREACH(current, stack, si_entries)
	{
		tag = current->si_node->html_tag.tag_type;
		va_start(ap, next);
		while ((test = (html_tag_type_t) va_arg(ap, int)) != TAG_ROOT)
		{
			if (tag == test)
			{
				va_end(ap);
				return 0;
			}
		}
		va_end(ap);
		if (tag == TAG_TABLE || tag == TAG_HTML)
		{
			return -1;
		}
	}

	return -1;
}

/**
 * @brief HTML5 Constructor meta code close_cell
 **/
int do_close_cell(struct html_parser_t *parser, html_node_t *next)
{
	if (do_has_these_tags_in_table_scope(parser, next, TAG_TD, TAG_ROOT) == 0)
	{
		return do_as_if_end(parser, next, TAG_TD);
	}
	if (do_has_these_tags_in_table_scope(parser, next, TAG_TH, TAG_ROOT) == 0)
	{
		return do_as_if_end(parser, next, TAG_TH);
	}
	return -1;
}

/**
 * @brief HTML5 Constructor meta code has_form
 **/
int do_has_form(struct html_parser_t *parser, html_node_t *next)
{
	return parser->hp_form ? 0 : -1;
}

/**
 * @brief HTML5 Constructor meta code is
 **/
int do_is(struct html_parser_t *parser, html_node_t *next, html_tag_type_t tag)
{
	assert(parser);
	assert(!SLIST_EMPTY(parser->hp_stack));
	return IS_A(STACK(parser), tag) ? 0 : -1;
}

/**
 * @brief HTML5 Constructor meta code proc_table_input
 **/
int do_proc_table_input(struct html_parser_t *parser, html_node_t *next)
{
	html_attribute_t *attr = NULL;
	int ok = 0;
	int ret = -1;
	assert(next);
	for (attr = next->html_tag.attribute; attr; attr = attr->next)
	{
		if (attr->name && attr->value && attr->type == ATTR_TYPE && strcasecmp(attr->value, "hidden") == 0)
		{
			ok = 1;
			break;
		}
	}
	if (ok)
	{
		ret = do_warning(parser, next);
		if (ret != 0)
		{
			return ret;
		}
		ret = do_append(parser, next);
		if (ret != 0)
		{
			return ret;
		}
		return do_popup(parser, next);
	}
	else
	{
		ret = do_warning(parser, next);
		if (ret != 0)
		{
			return ret;
		}
		if (do_is_in_these_tags(parser, next, TAG_TABLE, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_TR, TAG_ROOT) == 0)
		{
			ret = do_enter_foster(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_forward(parser, next, on_in_body);
			if (ret != 0)
			{
				return ret;
			}
			return do_exit_foster(parser, next);
		}
	}
	return -1;
}

/**
 * @brief HTML5 Constructor meta code reset
 **/
int do_reset(struct html_parser_t *parser, html_node_t *next)
{
	/* Here we do NOT support the foreign node */
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	int last = 0;
	assert(parser);
	stack = parser->hp_stack;
	SLIST_FOREACH(item, stack, si_entries)
	{
		if (SLIST_NEXT(item, si_entries) == NULL)
		{
			last = 1;
		}
		switch (item->si_node->html_tag.tag_type)
		{
		case TAG_SELECT:
			return do_switch(parser, next, on_in_select);
		case TAG_TD:
		case TAG_TH:
			if (!last)
			{
				return do_switch(parser, next, on_in_cell);
			}
			else
			{
				break;
			}
		case TAG_TR:
			return do_switch(parser, next, on_in_row);
		case TAG_TBODY:
		case TAG_THEAD:
		case TAG_TFOOT:
			return do_switch(parser, next, on_in_table_body);
		case TAG_CAPTION:
			return do_switch(parser, next, on_in_caption);
		case TAG_COLGROUP:
			return do_switch(parser, next, on_in_column_group);
		case TAG_TABLE:
			return do_switch(parser, next, on_in_table);
		case TAG_HEAD:
			if (parser->hp_html && TAG_HTML == parser->hp_html->html_tag.tag_type)
			{
				return do_switch(parser, next, on_in_body);
			}
			else if (parser->hp_html && TAG_WAP_WML == parser->hp_html->html_tag.tag_type)
			{
				return do_switch(parser, next, on_in_card);
			}
		case TAG_BODY:
			return do_switch(parser, next, on_in_body);
		case TAG_WAP_CARD:
			return do_switch(parser, next, on_in_card);
		case TAG_FRAMESET:
			return do_switch(parser, next, on_in_frameset);
		case TAG_HTML:
			return do_switch(parser, next, on_before_head);
		case TAG_WAP_WML:
			return do_switch(parser, next, on_before_head);
		default:
			break;
		}
		if (last)
		{
			if (parser->hp_html && TAG_HTML == parser->hp_html->html_tag.tag_type)
			{
				return do_switch(parser, next, on_in_body);
			}
			else if (parser->hp_html && TAG_WAP_WML == parser->hp_html->html_tag.tag_type)
			{
				return do_switch(parser, next, on_in_card);
			}
		}
	}
	return 0;
}

/**
 * @brief HTML5 Constructor meta code prepare_foster
 **/
int do_prepare_foster(struct html_parser_t *parser, html_node_t *next)
{
	html_node_t *foster = NULL;
	html_node_t *chroot = NULL;
	html_node_t *table = NULL;
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;
	struct foster_item_t *fi = NULL;

	assert(parser);

	stack = parser->hp_stack;
	SLIST_FOREACH(current, stack, si_entries)
	{
		if (IS_A(current->si_node, TAG_TABLE))
		{
			if (current->si_node->parent)
			{
				foster = current->si_node->parent;
			}
			else
			{
				foster = SLIST_NEXT(current, si_entries)->si_node;
			}
			table = current->si_node;
			break;
		}
		if (IS_A(current->si_node, TAG_HTML))
		{
			assert(IS_A(current->si_node->parent, TAG_ROOT));
			foster = current->si_node;
			break;
		}
	}
	chroot = html_tree_create_element_by_tag(parser->hp_tree, TAG_HTML);
	if (chroot == NULL)
	{
		return -1;
	}
	if (table && table->parent == foster)
	{
		html_node_insert_before(foster, chroot, table);
	}
	else
	{
		html_node_append_child(foster, chroot);
	}
	current = salloc_stack(parser->hp_stack_slab);
	if (current == NULL)
	{
		return -1;
	}
	current->si_node = chroot;
	fi = salloc_foster(parser->hp_foster_slab);
	if (fi == NULL)
	{
		return -1;
	}
	SLIST_INIT(&fi->fi_stack);
	SLIST_INSERT_HEAD(&fi->fi_stack, current, si_entries);
	SLIST_INSERT_HEAD(parser->hp_foster, fi, fi_entries);
	return 0;
}

/**
 * @brief HTML5 Constructor meta code merge_foster
 **/
int do_merge_foster(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	struct foster_item_t *foster = NULL;
	html_node_t *root = NULL;
	html_node_t *node = NULL;
	assert(parser);
	assert(!SLIST_EMPTY(parser->hp_foster));
	stack = &(SLIST_FIRST(parser->hp_foster))->fi_stack;
	SLIST_FOREACH(item, stack, si_entries)
	{
		if (SLIST_NEXT(item, si_entries) == NULL)
		{
			root = item->si_node;
			break;
		}
	}
	assert(root);
	while ((node = root->child) != NULL)
	{
		assert(node->parent == root);
		html_node_insert_before(root->parent, node, root);
	}
	html_node_remove_child(root->parent, root);
	while (!SLIST_EMPTY(stack))
	{
		item = SLIST_FIRST(stack);
		SLIST_REMOVE_HEAD(stack, si_entries);
		sfree_stack(parser->hp_stack_slab, item);
	}
	foster = SLIST_FIRST(parser->hp_foster);
	SLIST_REMOVE_HEAD(parser->hp_foster, fi_entries);
	sfree_foster(parser->hp_foster_slab, foster);

	return 0;
}

/**
 * @brief HTML5 Constructor meta code enter_foster
 **/
int do_enter_foster(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	assert(!SLIST_EMPTY(parser->hp_foster));
	parser->hp_foster_stack = parser->hp_stack;
	parser->hp_stack = &SLIST_FIRST(parser->hp_foster)->fi_stack;
	return 0;
}

/**
 * @brief HTML5 Constructor meta code exit_foster
 **/
int do_exit_foster(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	parser->hp_stack = parser->hp_foster_stack;
	parser->hp_foster_stack = NULL;
	return 0;
}

/**
 * @brief HTML5 Constructor meta code reconstruct_actfmt_list
 **/
int do_reconstruct_actfmt_list(struct html_parser_t *parser, html_node_t *next)
{
	/*
	 * Here is the logic.
	 *
	 *   A active formatting list is used to handle nested closed tags,
	 * formatting tags is critical for visualizing but easily mistook,
	 * so we cannot close too much if nested.
	 *   Consider the following sequence, '<b>1<i>2</b>3</i>4', we cannot
	 * just close <i> when meeting </b>, the author want the text to be
	 * italic until </i>, so we have to reopen <i> after </b> which means
	 * reopen all tags which is not closed explicitly.
	 *   So the final DOM should be '<b>1<i>2</i></b><i>3</i>4'.
	 *   Consider this again, '<b>1<p>2</b>3</p>4', <p> is quite different
	 * with <i> as it is not formatting but paragraph. A paragraph means
	 * a new block of text, independe to previous visual style, and thus
	 * we should close <b> confronting with <p>. However, after <p> we
	 * meet </b>, which means that the author need a <b> style here, thus
	 * we reopen <b> after <p>.
	 *   So the final DOM should be '<b>1</b><p><b>2</b>3</p>4'.
	 *
	 *   In this 'reconstruct active formatting list' action, we should
	 * reopen, a.k.a. push into open elements stack, formatting elements
	 * from the list one by one until it is in open elements stack or
	 * a marker.
	 */
	struct html_stack_t *stack = NULL;
	struct html_stack_t *list = NULL;
	struct stack_item_t *item = NULL;
	struct stack_item_t *ins = NULL;
	assert(parser);
	stack = parser->hp_stack;
	list = parser->hp_actfmt_list;
	SLIST_FOREACH(item, list, si_entries)
	{
		if (item->si_node == NULL || is_in_stack(stack, item->si_node))
		{
			break;
		}
		else
		{
			ins = salloc_stack(parser->hp_stack_slab);
			if (ins == NULL)
			{
				return -1;
			}
			ins->si_node = item->si_node;
			SLIST_INSERT_HEAD(stack, ins, si_entries);
		}
	}
	return 0;
}

/**
 * @brief HTML5 Constructor meta code append_actfmt_list
 **/
int do_append_actfmt_list(struct html_parser_t *parser, html_node_t *next)
{
	struct stack_item_t *item = NULL;
	assert(parser);
	assert(next);
	item = salloc_stack(parser->hp_stack_slab);
	if (item == NULL)
	{
		return -1;
	}
	item->si_node = next;
	SLIST_INSERT_HEAD(parser->hp_actfmt_list, item, si_entries);
	return 0;
}

/**
 * @brief HTML5 Constructor meta code append_actfmt_list_marker
 **/
int do_append_actfmt_list_marker(struct html_parser_t *parser, html_node_t *next)
{
	struct stack_item_t *item = NULL;
	assert(parser);
	item = salloc_stack(parser->hp_stack_slab);
	if (item == NULL)
	{
		return -1;
	}
	item->si_node = NULL;
	SLIST_INSERT_HEAD(parser->hp_actfmt_list, item, si_entries);
	return 0;
}

/**
 * @brief HTML5 Constructor meta code clear_actfmt_list
 **/
int do_clear_actfmt_list(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	assert(parser);
	stack = parser->hp_actfmt_list;
	while (!SLIST_EMPTY(stack))
	{
		item = SLIST_FIRST(stack);
		SLIST_REMOVE_HEAD(stack, si_entries);
		if (item->si_node == NULL)
		{
			sfree_stack(parser->hp_stack_slab, item);
			break;
		}
		else
		{
			sfree_stack(parser->hp_stack_slab, item);
		}
	}
	return 0;
}

/**
 * @brief HTML5 Constructor meta code proc_select_optgroup_end
 **/
int do_proc_select_optgroup_end(struct html_parser_t *parser, html_node_t *next)
{
	struct html_stack_t *stack = NULL;
	struct stack_item_t *item = NULL;
	int ret = -1;
	assert(parser);
	stack = parser->hp_stack;
	item = SLIST_FIRST(stack);
	if (IS_A(item->si_node, TAG_OPTION))
	{
		item = SLIST_NEXT(item, si_entries);
		if (IS_A(item->si_node, TAG_OPTGROUP))
		{
			ret = do_as_if_end(parser, next, TAG_OPTION);
			if (ret != 0)
			{
				return -1;
			}
		}
	}
	item = SLIST_FIRST(stack);
	if (IS_A(item->si_node, TAG_OPTGROUP))
	{
		return do_popup(parser, next);
	}
	else
	{
		return do_warning(parser, next);
	}
	return -1;
}

/**
 * @brief HTML5 Constructor meta code parse_rcdata
 **/
int do_parse_rcdata(struct html_parser_t *parser, html_node_t *next)
{
	int ret = -1;
	html_node_t *node = NULL;
	size_t len = 0;
	assert(parser);
	if (parser->hp_use_nest_tokenizer)
	{
		html_tokenizer_switch_to_rcdata(parser->hp_nest_tokenizer);
		node = html_tokenize(parser->hp_nest_tokenizer, parser->hp_tree);
	}
	else
	{
		html_tokenizer_switch_to_rcdata(parser->hp_tokenizer);
		node = html_tokenize(parser->hp_tokenizer, parser->hp_tree);
	}
	//html_tokenizer_switch_to_rcdata(parser->hp_tokenizer);
	//node = html_tokenize(parser->hp_tokenizer, parser->hp_tree);
	if (node == NULL)
	{
		return -1;
	}
	if (node->html_tag.tag_type == TAG_WHITESPACE)
	{
		node->html_tag.tag_type = TAG_PURETEXT;
	}
	if (node->html_tag.tag_type == TAG_PURETEXT)
	{
		if (node->html_tag.text[0] != '\0')
		{
			if (IS_A(STACK(parser), TAG_STYLE))
			{
				STACK(parser)->html_tag.text = node->html_tag.text;
			}
			else if (IS_A(STACK(parser), TAG_SCRIPT))
			{
				STACK(parser)->html_tag.text = node->html_tag.text;
				if (parser->hp_script_parsing && (!parser->hp_xml_compatible || !parser->hp_wml_compatible))
				{
					len = strlen(node->html_tag.text);
					parser->hp_document_write = (char*) html_tree_palloc(parser->hp_tree, len);
					if (parser->hp_document_write == NULL)
					{
						return -1;
					}
					ret = js_eval(node->html_tag.text, parser->hp_document_write, len);
					if (ret > 0)
					{
						html_tokenizer_reset(parser->hp_nest_tokenizer, parser->hp_document_write, len);
						parser->hp_use_nest_tokenizer = 1;
					}
				}
			}
			else
			{
				ret = do_append_no_stack(parser, node);
				if (ret != 0)
				{
					return ret;
				}
			}
		}
		return do_popup(parser, next);
	}
	else
	{
		return -1;
	}
}

/**
 * @brief HTML5 Constructor meta code is_match
 **/
int do_is_match(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	assert(next);
	if (IS_MATCH(STACK(parser), next))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/**
 * @brief HTML5 Constructor meta code is_root
 **/
int do_is_root(struct html_parser_t *parser, html_node_t *next)
{
	assert(parser);
	if (IS_A(STACK(parser), TAG_ROOT))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
int cal_node_length(const html_parser_t *parser, html_node_t *next)
{
	html_tag_type_t tag = TAG_UNKNOWN;
	struct html_stack_t *stack = NULL;
	struct stack_item_t *current = NULL;

	assert(parser);
	assert(next);

	tag = next->html_tag.tag_type;
	stack = parser->hp_stack;

	current = SLIST_FIRST(stack);
	if (next && current && current->si_node && IS_MATCH(current->si_node, next))
	{
		//shuangwei add 计算节点长度
		if (next->html_tag.is_close_tag && next->html_tag.page_offset >= 0 && current->si_node->html_tag.page_offset >= 0)
		{
			current->si_node->html_tag.nodelength = next->html_tag.page_offset - current->si_node->html_tag.page_offset;
		}
	}

	return 0;
}
