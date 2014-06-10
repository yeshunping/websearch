/*
 * easou_html_parser.cpp
 *
 *  Created on: 2011-11-8
 *      Author: xunwu
 */

#include <assert.h>
#include <string.h>
#include "easou_html_pool.h"
#include "easou_html_node.h"
#include "easou_html_tokenizer.h"
#include "easou_html_constructor.h"
#include "easou_html_operate.h"
#include "easou_html_parser.h"

SLAB_DEFINE(stack, stack_item_t);
SLAB_DEFINE(foster, foster_item_t);

static int html_parser_reset_all(struct html_parser_t *parser);
/**
 * @brief Create the parser
 **/
struct html_parser_t* html_parser_create()
{
	struct mem_pool_t *pool = NULL;
	struct html_parser_t *parser = NULL;

	/* create memory pool, all things are allocated here */
	pool = mem_pool_create(NULL, POOLSZ);
	if (pool == NULL)
	{
		return NULL;
	}
	/* create parser */
	parser = (struct html_parser_t*) palloc(pool, sizeof(*parser));
	if (parser == NULL)
	{
		mem_pool_destroy(pool);
		return NULL;
	}
	memset(parser, 0, sizeof(*parser));
	parser->hp_pool = pool;
	parser->hp_xml_compatible = 0;
	parser->hp_wml_compatible = 0;
	parser->hp_script_parsing = 1;
	/* create tokenizer */
	parser->hp_tokenizer = html_tokenizer_create(parser->hp_pool);
	if (parser->hp_tokenizer == NULL)
	{
		html_parser_destroy(parser);
		return NULL;
	}
	parser->hp_nest_tokenizer = html_tokenizer_create(parser->hp_pool);
	if (parser->hp_nest_tokenizer == NULL)
	{
		html_parser_destroy(parser);
		return NULL;
	}
	/* create open elements stack */
	// parser->hp_stack = (struct html_stack_t*)palloc(pool, sizeof(struct html_stack_t));
	parser->hp_stack = (struct html_stack_t*) palloc(parser->hp_pool, sizeof(struct html_stack_t));
	if (parser->hp_stack == NULL)
	{
		html_parser_destroy(parser);
		return NULL;
	}
	SLIST_INIT(parser->hp_stack);

	/* create foster parents stack */
	// parser->hp_foster = (struct html_foster_t*)palloc(pool, sizeof(struct html_foster_t));
	parser->hp_foster = (struct html_foster_t*) palloc(parser->hp_pool, sizeof(struct html_foster_t));
	if (parser->hp_foster == NULL)
	{
		html_parser_destroy(parser);
		return NULL;
	}
	SLIST_INIT(parser->hp_foster);

	/* create active formatting list */
	//parser->hp_actfmt_list =  (struct html_stack_t*)palloc(pool, sizeof(struct html_stack_t));
	parser->hp_actfmt_list = (struct html_stack_t*) palloc(parser->hp_pool, sizeof(struct html_stack_t));
	if (parser->hp_actfmt_list == NULL)
	{
		html_parser_destroy(parser);
		return NULL;
	}
	SLIST_INIT(parser->hp_actfmt_list);

	/* create slab for stack_item_t */
	parser->hp_stack_slab = slab_create(parser->hp_pool, sizeof(struct stack_item_t));
	if (parser->hp_stack_slab == NULL)
	{
		return NULL;
	}

	/* create slab for foster_item_t */
	parser->hp_foster_slab = slab_create(parser->hp_pool, sizeof(struct foster_item_t));
	if (parser->hp_foster_slab == NULL)
	{
		return NULL;
	}
	return parser;
}

/**
 * @brief 从parser中获取内存
 **/
void* html_parser_palloc(struct html_parser_t *parser, size_t size)
{
	return palloc(parser->hp_pool, size);
}

/**
 * @brief 从tree中获取内存，实际上还是才能够parser上获取
 **/
void* html_tree_palloc(html_tree_t *tree, size_t len)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	return html_parser_palloc(self->ht_parser, len);
}

/**
 * @brief 从tree中获取内存，同时进行初始化，实际上还是才能够parser上获取
 */
char* html_tree_strndup(html_tree_t *tree, const char *text, size_t len)
{
	char *str = NULL;
	assert(tree);
	str = (char*) html_tree_palloc(tree, len + 1);
	if (str == NULL)
	{
		return NULL;
	}
	strncpy(str, text, len);
	str[len] = '\0';
	return str;
}

/**
 * @brief Destroy the parser
 **/
void html_parser_destroy(struct html_parser_t *parser)
{
	if (!parser)
	{
		return;
	}
	assert(parser->hp_pool);
	mem_pool_destroy(parser->hp_pool);
}

/**
 * @brief GETTER of parser::xml_compatible
 **/
int html_parser_get_xml_compatible(struct html_parser_t *parser)
{
	return parser->hp_xml_compatible;
}

/**
 * @brief SETTER of parser::xml_compatible
 **/
void html_parser_set_xml_compatible(struct html_parser_t *parser, int enable)
{
	parser->hp_xml_compatible = enable;
}

/**
 * @brief GETTER of parser::wml_compatible
 **/
int html_parser_get_wml_compatible(struct html_parser_t *parser)
{
	return parser->hp_wml_compatible;
}

/**
 * @brief SETTER of parser::wml_compatible
 **/
void html_parser_set_wml_compatible(struct html_parser_t *parser, int enable)
{
	parser->hp_wml_compatible = enable;
}

/**
 * @brief GETTER of parser::script_parsing
 **/
int html_parser_get_script_parsing(struct html_parser_t *parser)
{
	return parser->hp_script_parsing;
}

/**
 * @brief SETTER of parser::script_parsing
 **/
void html_parser_set_script_parsing(struct html_parser_t *parser, int enable)
{
	parser->hp_script_parsing = enable;
}

/**
 * @brief 将html网页解析成dom树
 **/
int html_parse(struct html_parser_t *parser, html_tree_t *tree, const char *page, size_t size, int ignore_space)
{
	int ret = -1;
	html_node_t *next = NULL;
	assert(parser);
	assert(tree);
	if (page == NULL)
	{
		return -1;
	}
	parser->hp_tree = tree;
	// shuangwei modify 20120405 修改复位顺序及复位函数
//    /*设置网页源代码读取器*/
//    html_tokenizer_reset(parser->hp_tokenizer, page, size);
//    /*设置分析策略*/
//    if (html_parser_reset(parser) != 0) {
//        return -1;
//    }
	if (html_parser_reset_all(parser) != 0)
	{
		return -1;
	}
	html_tokenizer_reset(parser->hp_tokenizer, page, size);
	// shuangwei modify 20120405

	parser->hp_ignore_space = ignore_space;
	while (1)
	{
		if (parser->hp_use_nest_tokenizer)
		{
			next = html_tokenize(parser->hp_nest_tokenizer, parser->hp_tree);
			if (!next)
			{
				parser->hp_use_nest_tokenizer = 0;
				continue;
			}
			next->html_tag.page_offset = -1;
		}
		else
		{
			next = html_tokenize(parser->hp_tokenizer, parser->hp_tree);
			if (!next)
			{
				break;
			}
		}
		if (parser->hp_handler(parser, next) != 0)
		{
			return -1;
		}
		if (next->html_tag.is_close_tag)
		{
			html_node_destroy(next);
		}
	}
	while (!SLIST_EMPTY(parser->hp_foster))
	{
		ret = do_merge_foster(parser, NULL);
		if (ret != 0)
		{
			return ret;
		}
	}
	assert(SLIST_EMPTY(parser->hp_foster));
	return 0;
}

static int html_parser_reset_all(struct html_parser_t *parser)
{
	struct stack_item_t *item = NULL;
	assert(parser);
	/* reset state variable */
	//parser的pool不断申请内存需要复位
	if (parser->hp_pool)
	{
		mem_pool_reset(parser->hp_pool);
		parser->hp_pool->mp_last = parser->hp_pool->mp_last + sizeof(html_parser_t);
	}

	parser->hp_handler = on_initial;
	if (parser->hp_xml_compatible)
	{
		parser->hp_handler = on_xml_root;
	}
	if (parser->hp_wml_compatible)
	{
		parser->hp_handler = on_wml_root;
	}
	parser->hp_last_handler = NULL;
	parser->hp_html = NULL;
	parser->hp_body = NULL;
	parser->hp_form = NULL;
	parser->hp_use_nest_tokenizer = 0;
	parser->hp_document_write = NULL;
	parser->hp_foster_stack = NULL;
	parser->hp_ignore_space = 0;
	parser->hp_tokenizer = html_tokenizer_create(parser->hp_pool);
	if (parser->hp_tokenizer == NULL)
	{
		html_parser_destroy(parser);
		return -1;
	}
	parser->hp_nest_tokenizer = html_tokenizer_create(parser->hp_pool);
	if (parser->hp_nest_tokenizer == NULL)
	{
		html_parser_destroy(parser);
		return -1;
	}

	/* create open elements stack */
	// parser->hp_stack = (struct html_stack_t*)palloc(pool, sizeof(struct html_stack_t));
	parser->hp_stack = (struct html_stack_t*) palloc(parser->hp_pool, sizeof(struct html_stack_t));
	if (parser->hp_stack == NULL)
	{
		html_parser_destroy(parser);
		return -1;
	}
	SLIST_INIT(parser->hp_stack);

	/* create foster parents stack */
	// parser->hp_foster = (struct html_foster_t*)palloc(pool, sizeof(struct html_foster_t));
	parser->hp_foster = (struct html_foster_t*) palloc(parser->hp_pool, sizeof(struct html_foster_t));
	if (parser->hp_foster == NULL)
	{
		html_parser_destroy(parser);
		return -1;
	}
	SLIST_INIT(parser->hp_foster);

	/* create active formatting list */
	//parser->hp_actfmt_list =  (struct html_stack_t*)palloc(pool, sizeof(struct html_stack_t));
	parser->hp_actfmt_list = (struct html_stack_t*) palloc(parser->hp_pool, sizeof(struct html_stack_t));
	if (parser->hp_actfmt_list == NULL)
	{
		html_parser_destroy(parser);
		return -1;
	}
	SLIST_INIT(parser->hp_actfmt_list);

	/* create slab for stack_item_t */
	parser->hp_stack_slab = slab_create(parser->hp_pool, sizeof(struct stack_item_t));
	if (parser->hp_stack_slab == NULL)
	{
		return -1;
	}
	parser->hp_stack_slab->slb_free = 0;
	/* create slab for foster_item_t */
	parser->hp_foster_slab = slab_create(parser->hp_pool, sizeof(struct foster_item_t));
	if (parser->hp_foster_slab == NULL)
	{
		return -1;
	}
	parser->hp_foster_slab->slb_free = 0;
	if (parser->hp_pool)
	{
		parser->hp_pool->mp_last = parser->hp_pool->mp_last + sizeof(html_tree_impl_t);
	}
	/* create new bottom for open elemens stack*/
	item = salloc_stack(parser->hp_stack_slab);
	if (item == NULL)
	{
		return -1;
	}
	item->si_node = &(parser->hp_tree->root);
	SLIST_INSERT_HEAD(parser->hp_stack, item, si_entries);
	//html_tree_impl_t

	return 0;
}
