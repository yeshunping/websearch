/*
 * easou_html_tree.cpp
 *
 *  Created on: 2011-11-8
 *      Author: xunwu
 */
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "easou_url.h"
#include "html_text_utils.h"
#include "easou_html_attr.h"
#include "easou_html_parser.h"
#include "easou_html_tree.h"
#include "easou_html_doctype.h"
#include"debuginfo.h"

using namespace EA_COMMON;

/**
 * @brief 创建树
 **/
html_tree_t* html_tree_create(int max_page_len)
{
	struct html_parser_t *parser = NULL;
	struct html_tree_impl_t *tree = NULL;
	parser = html_parser_create();
	if (parser == NULL)
	{
		return NULL;
	}
	tree = (struct html_tree_impl_t*) html_parser_palloc(parser, sizeof(*tree));
	if (tree == NULL)
	{
		return NULL;
	}
	memset(tree, 0, sizeof(*tree));
	tree->ht_parser = parser;
	tree->ht_tree.root.html_tag.tag_type = TAG_ROOT;
	tree->ht_tree.root.html_tag.page_offset = -1;
	tree->ht_tree.root.owner = (html_tree_t*) tree;
	return &tree->ht_tree;
}

//todo
void html_tree_clean(html_tree_t *html_tree)
{
//	html_tree_del(html_tree);
//	html_tree = NULL;
//	html_tree = html_tree_create(128*1024);
	html_tree->body = NULL;
	html_tree->head = NULL;
	html_tree->html = NULL;
	struct html_tree_impl_t *self = (struct html_tree_impl_t*) html_tree;
	memset(&self->ht_tree, 0, sizeof(html_tree_t));
	self->ht_tree.root.html_tag.tag_type = TAG_ROOT;
	self->ht_tree.root.html_tag.page_offset = -1;
	if (self->ht_pool)
	{
//    	printf("clean-----self->ht_pool->mp_chain:%lx,	self->ht_pool->mp_large:%lx,	self->ht_pool->mp_slab:%lx\n",
//    			&(self->ht_pool->mp_chain), &(self->ht_pool->mp_large), self->ht_pool->mp_slab);
		mem_pool_destroy(self->ht_pool);
		self->ht_pool = NULL;
	}
}
//todo

/**
 * @brief 销毁树
 **/
void html_tree_del(html_tree_t *tree)
{
	struct html_tree_impl_t *self = NULL;
	if (tree == NULL)
	{
		return;
	}
	self = (struct html_tree_impl_t*) tree;
	if (self->ht_pool)
	{
		mem_pool_destroy(self->ht_pool);
		self->ht_pool = NULL;
	}
	html_parser_destroy(self->ht_parser);
	tree = NULL;
	self = NULL;
}

/**
 * @brief 设置是否兼容xml
 **/
void html_tree_set_xml_compatible(html_tree_t *tree, int enable)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	html_parser_set_xml_compatible(self->ht_parser, enable);
}

/**
 * @brief 判断是否兼容xml
 **/
int html_tree_is_xml_compatible(html_tree_t *tree)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	return html_parser_get_xml_compatible(self->ht_parser);
}

/**
 * @brief 设置是否兼容wml
 **/
void html_tree_set_wml_compatible(html_tree_t *tree, int enable)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	html_parser_set_wml_compatible(self->ht_parser, enable);
}

/**
 * @brief 判断是否兼容wml
 **/
int html_tree_is_wml_compatible(html_tree_t *tree)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	return html_parser_get_wml_compatible(self->ht_parser);
}

/**
 * @brief 设置是否解析js
 **/
void html_tree_set_script_parsing(html_tree_t *tree, int enable)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	html_parser_set_script_parsing(self->ht_parser, enable);
}

/**
 * @brief 判断是否解析js
 **/
int html_tree_is_script_parsing(html_tree_t *tree)
{
	struct html_tree_impl_t *self = NULL;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	return html_parser_get_script_parsing(self->ht_parser);
}

/**
 * @brief 将html网页解析成dom树
 * @param [in/out] html_tree   : html_tree_t*	dom树结构
 * @param [in] page   : char*	页面源代码
 * @param [in] page_len   : int	页面源代码的长度
 * @param [in] ignore_space   : bool	是否忽略页面中的空格
 * @return  int
 * @retval  0:解析失败;1:解析成功.
 * @author xunwu
 * @date 2011/08/02
 **/
int html_tree_parse(html_tree_t *tree, char *html, int size, bool ignore_space)
{
	struct html_tree_impl_t *self = NULL;
	int ret = -1;
	assert(tree);
	self = (struct html_tree_impl_t*) tree;
	// if (html_tree_reset(self) != 0) { //shuangwei 20120330 modify reset don't destroy space
	if (html_tree_reset_no_destroy(self) != 0)
	{
		return 0;
	}
	ret = html_parse(self->ht_parser, tree, html, size, ignore_space);
	markDomTreeSubType(tree);
	return ret == 0 ? 1 : 0;
}

/**
 * @brief 遍历树
 **/
int html_tree_visit(html_tree_t *html_tree, start_visit_t start_visit, finish_visit_t finish_visit, void *result, int flag)
{
	return (html_node_visit(&html_tree->root, start_visit, finish_visit, result, flag) == VISIT_ERROR) ? 0 : 1;
}

/**
 * @brief 遍历节点
 **/
int html_node_visit(html_node_t *html_node, start_visit_t start_visit, finish_visit_t finish_visit, void *result, int flag)
{
	int ret = VISIT_NORMAL;
	int lret = VISIT_NORMAL;
	html_node_t *current = NULL;

	current = html_node;
	while (current)
	{
		if (start_visit)
		{
			lret = start_visit(&current->html_tag, result, flag);
			if (lret == VISIT_ERROR || lret == VISIT_FINISH)
			{
				ret = lret;
				return ret;
			}
		}
		if (current->child && lret != VISIT_SKIP_CHILD)
		{
			current = current->child;
			continue;
		}
		if (finish_visit && lret != VISIT_SKIP_CHILD)
		{
			lret = finish_visit(&current->html_tag, result);
			if (lret == VISIT_ERROR || lret == VISIT_FINISH)
			{
				ret = lret;
				return ret;
			}
			assert(lret == VISIT_NORMAL);
		}
		if (current == html_node)
		{
			return ret;
		}
		if (current->next)
		{
			current = current->next;
			continue;
		}
		while (1)
		{
			current = current->parent;
			if (!current)
			{
				break;
			}
			if (finish_visit)
			{
				lret = finish_visit(&current->html_tag, result);
				if (lret == VISIT_ERROR || lret == VISIT_FINISH)
				{
					ret = lret;
					return ret;
				}
				assert(lret == VISIT_NORMAL);
			}
			if (current == html_node)
			{
				return ret;
			}

			if (current->next)
			{
				current = current->next;
				break;
			}
		}
	}
	return ret;
}

struct descinfo
{
	char *description;
	int size;
};

static int start_visit_description(html_tag_t *tag, void *result, int flag)
{
	descinfo *desc = (descinfo*) result;
	if (tag->tag_type == TAG_META && 0 == strcmp(get_attribute_value(tag, ATTR_NAME), "description"))
	{
		char* content = get_attribute_value(tag, ATTR_CONTENT);
		if (content)
		{
			strncpy(desc->description, content, desc->size);
			return VISIT_FINISH;
		}
	}
	return VISIT_NORMAL;
}

/**
 * @brief 提取description
 **/
int html_tree_extract_description(html_tree_t *html_tree, char* description, int size)
{
	assert(description);
	assert(size > 0);

	descinfo desc;
	desc.description = description;
	desc.size = size;
	desc.description[0] = '\0';
	return html_tree_visit(html_tree, start_visit_description, NULL, &desc, 0);
}

/**
 * @brief 重置树
 **/
int html_tree_reset_no_destroy(struct html_tree_impl_t *self)
{
	memset(&self->ht_tree, 0, sizeof(html_tree_t));
	self->ht_tree.root.html_tag.tag_type = TAG_ROOT;
	self->ht_tree.root.html_tag.page_offset = -1;
	self->ht_tree.root.owner = (html_tree_t*) self;

	//shuangwei modify,reset don't free space
//    if (self->ht_pool) {
//    	mem_pool_destroy(self->ht_pool);
//    }
//    self->ht_pool = mem_pool_create(NULL, POOLSZ);

	if (self->ht_pool)
	{
		mem_pool_reset(self->ht_pool);
	}
	else
	{
		self->ht_pool = mem_pool_create(NULL, POOLSZ);
		if (g_EASOU_DEBUG == DEBUG_MEMERY)
		{
			char info[1000] =
			{ 0 };
			sprintf(info, "ht_pool = %p", (self->ht_pool));
			easouprintf(info);
		}
	}
	//shuangwei modify ,over
	if (NULL == self->ht_pool)
	{
		return -1;
	}
	self->ht_tag_code = 0;

	self->ht_node_slab = slab_create(self->ht_pool, sizeof(html_node_t));
//    if(EASOE_DEBUG){
//    			char info[1000]={0};
//    					sprintf(info,"ht_node_slab = %#x",self->ht_node_slab);
//    					easouprintf(info);
//    }
//    if(self->ht_node_slab == NULL){
//    	 self->ht_node_slab = slab_create(self->ht_pool,sizeof(html_node_t));
//    }

	if (self->ht_node_slab == NULL)
	{
		return -1;
	}
	return 0;
}

int determine_doctype(html_tree_t *html_tree, const char* url)
{
	return determine_doctype_from_tree(html_tree, url);
}

void printNode(html_node_t *html_node)
{
	if (html_node)
	{
		html_attribute_t *attribute;
		if (html_node->html_tag.tag_name)
		{
			myprintf("<%s", html_node->html_tag.tag_name);
		}
		else
		{
			if (html_node->html_tag.tag_type == TAG_ROOT)
			{
				myprintf("<ROOT");
			}
			else
			{
				myprintf("<%d", html_node->html_tag.tag_type);
			}

		}
		for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
		{
			myprintf( " %s", attribute->name);
			if (attribute->value != NULL)
			{
				myprintf("=\"%s\"", attribute->value);
			}
		}
		if (html_node->html_tag.tag_type == TAG_PURETEXT)
		{
			myprintf( ";text=%s#;", html_node->html_tag.text);
		}
		myprintf( ">\n");
	}
}

void PrintNode(html_node_t *html_node, int level)
{
	html_attribute_t *attribute;
	html_node_t *child;

	if (level == 0)
	{
		myprintf( " ");
	}
	for (int i = 0; i < level - 1; i++)
	{
		myprintf( " ");
	}

	if (html_node->html_tag.tag_type == TAG_DOCTYPE)
	{
		myprintf("<!DOCTYPE %s>\n", html_node->html_tag.text);
	}
	else if (html_node->html_tag.tag_type == TAG_COMMENT)
	{
		myprintf("<!-- %s -->\n", html_node->html_tag.text);
	}
	else if (html_node->html_tag.tag_type == TAG_PURETEXT)
	{
		myprintf("%s\n", html_node->html_tag.text);
	}
	else
	{

		if (html_node->html_tag.tag_name)
		{
			myprintf("<%s", html_node->html_tag.tag_name);
		}
		else
		{
			if (html_node->html_tag.tag_type == TAG_ROOT)
			{
				myprintf("<ROOT");
			}
			else
			{
				myprintf("<%d", html_node->html_tag.tag_type);
			}

		}
		for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
		{
			myprintf( " %s", attribute->name);
			if (attribute->value != NULL)
			{
				myprintf("=\"%s\"", attribute->value);
			}
		}
		if (html_node->html_tag.is_self_closed)
		{
			myprintf("/");
		}
		myprintf( ">(childnodetype=%x,subnodetype=%x)\n", html_node->childnodetype, html_node->subnodetype);

	}

	for (child = html_node->child; child != NULL; child = child->next)
	{
		PrintNode(child, level + 1);
	}
	if (html_node->html_tag.tag_type != TAG_COMMENT && html_node->html_tag.tag_type != TAG_DOCTYPE && html_node->html_tag.tag_type != TAG_PURETEXT && !html_node->html_tag.is_self_closed)
	{
		for (int i = 0; i < level - 1; i++)
		{
			myprintf("  ");
		}
		if (html_node->html_tag.tag_type == TAG_ROOT)
		{
			myprintf("</ROOT>\n");
		}
		else
		{
			myprintf("</%s>\n", html_node->html_tag.tag_name);
		}
	}
}

void printTree(html_tree_t *html_tree)
{
	if (html_tree)
	{
		myprintf("the attr of tree is %x\n", html_tree->treeAttr);
		PrintNode(&(html_tree->root), 0);
	}
}

int finish_mark_node_type(html_tag_t *tag, void *result)
{
	if (tag)
	{
		html_node_t *node = (html_node_t *) tag;
		for (html_node_t * child = node->child; child; child = child->next)
		{
			char * alt = NULL;
			char * src = NULL;
			switch (child->html_tag.tag_type)
			{
			case TAG_P:
				MARK_DOMTREE_P_TAG(node->childnodetype);
				break;
			case TAG_DIV:
				MARK_DOMTREE_DIV_TAG(node->childnodetype);
				break;
			case TAG_TABLE:
				MARK_DOMTREE_TABLE_TAG(node->childnodetype);
				break;
			case TAG_H1:
			case TAG_H2:
			case TAG_H3:
			case TAG_H4:
			case TAG_H5:
			case TAG_H6:
				MARK_DOMTREE_H_TAG(node->childnodetype);
				break;
			case TAG_UL:
			case TAG_OL:
				MARK_DOMTREE_LIST_TAG(node->childnodetype);
				break;
			case TAG_FORM:
				MARK_DOMTREE_FORM_TAG(node->childnodetype);
				break;
			case TAG_IMG:

				alt = html_node_get_attribute_value(child, "alt");
				src = html_node_get_attribute_value(child, "src");
				if (src && alt)
				{
					char * slappos = strrchr(src, '/');
					char * pointpos = strrchr(src, '.');
					bool run = false;
					if (slappos && pointpos && (pointpos - slappos) > 0)
					{
						int isgif = strncasecmp(pointpos + 1, "gif", strlen("gif"));
						if (isgif)
						{
							run = true;
						}
					}
					if (strlen(alt) > 1 && run)
					{
						MARK_DOMTREE_IMG_TAG(node->childnodetype);
					}
				}
				else
				{

				}
				break;
			default:
				break;
			}
			node->subnodetype = node->subnodetype | child->subnodetype;
		}
		node->subnodetype = node->subnodetype | node->childnodetype;
	}
	return VISIT_NORMAL;
}

/**
 * 标注dom树的每个节点含有的子节点类型
 * html_tree：in
 * 结果：-1：参数错误；0：成功；>0:其他错误
 *
 */
int markDomTreeSubType(html_tree_t *html_tree)
{
	if (html_tree == NULL)
	{
		return -1;
	}
	if (IS_DOMTREE_SUBTYPE(html_tree->treeAttr))
	{
		return 0;
	}
	else
	{
		int flag = 0;
		int result = html_tree_visit(html_tree, NULL, finish_mark_node_type, (void *) NULL, flag);
		if (result == VISIT_NORMAL)
		{
			MARK_DOMTREE_SUBTYPE(html_tree->treeAttr);
		}
		return result;
	}
}

int printTreeSrc(html_tree_t *html_tree, char *srcodebuf, int buflen)
{
	if (html_tree && srcodebuf && buflen > 0)
	{
		printDomS doms;
		doms.availpos = 0;
		doms.buflen = buflen;
		doms.psrccode = srcodebuf;
		int result = PrintNodeSrc(&(html_tree->root), &doms, 0);
		if (result < 0)
		{
			return result;
		}
		else
		{
			return doms.availpos;
		}
	}
	else
	{
		return -1;
	}
}
int PrintNodeSrc(html_node_t *html_node, printDomS *pPrintSrc, int level)
{

	int result = 0;
	html_attribute_t *attribute;
	html_node_t *child;
	if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
	{
		return -1;
	}
//	if (level == 0)
//	{
//		pPrintSrc->availpos+=snprintf(pPrintSrc->psrccode+pPrintSrc->availpos,pPrintSrc->buflen-pPrintSrc->availpos, " ");
//	}
//	for (int i = 0; i < level - 1; i++)
//	{
//		pPrintSrc->availpos+=snprintf(pPrintSrc->psrccode+pPrintSrc->availpos,pPrintSrc->buflen-pPrintSrc->availpos," ");
//	}

	if (html_node->html_tag.tag_type == TAG_DOCTYPE)
	{
		pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "<!DOCTYPE %s>", html_node->html_tag.text);
		if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
		{
			return -1;
		}
	}
	else if (html_node->html_tag.tag_type == TAG_COMMENT)
	{
		pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "<!-- %s -->", html_node->html_tag.text);
		if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
		{
			return -1;
		}
	}
	else if (html_node->html_tag.tag_type == TAG_PURETEXT)
	{
		pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "%s", html_node->html_tag.text);
		if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
		{
			return -1;
		}
	}
	else
	{

		if (html_node->html_tag.tag_name)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "<%s", html_node->html_tag.tag_name);
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
		}

		for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, " %s", attribute->name);
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
			if (attribute->value != NULL)
			{
				pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "=\"%s\"", attribute->value);
				if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
				{
					return -1;
				}
			}
		}
		if (html_node->html_tag.is_self_closed)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "/");
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
		}
		if (html_node->html_tag.tag_type != TAG_ROOT && html_node->html_tag.tag_name)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, ">");
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
		}

	}

	for (child = html_node->child; child != NULL; child = child->next)
	{
		result = PrintNodeSrc(child, pPrintSrc, level + 1);
		if (result < 0)
		{
			return result;
		}
	}
	if (html_node->html_tag.tag_type != TAG_COMMENT && html_node->html_tag.tag_type != TAG_DOCTYPE && html_node->html_tag.tag_type != TAG_PURETEXT && !html_node->html_tag.is_self_closed)
	{
//		for (int i = 0; i < level - 1; i++)
//		{
//			pPrintSrc->availpos+=snprintf(pPrintSrc->psrccode+pPrintSrc->availpos,pPrintSrc->buflen-pPrintSrc->availpos,"  ");
//		}
		if (html_node->html_tag.tag_type == TAG_STYLE)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "%s</style>", html_node->html_tag.text ? html_node->html_tag.text : "");
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
		}
		else if (html_node->html_tag.tag_type == TAG_SCRIPT)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "%s</script>", html_node->html_tag.text ? html_node->html_tag.text : "");
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
		}
		else if (html_node->html_tag.tag_name)
		{
			pPrintSrc->availpos += snprintf(pPrintSrc->psrccode + pPrintSrc->availpos, pPrintSrc->buflen - pPrintSrc->availpos, "</%s>", html_node->html_tag.tag_name);
			if (!pPrintSrc || pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
			{
				return -1;
			}
		}
	}
	if (pPrintSrc->availpos + 10 >= pPrintSrc->buflen)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
