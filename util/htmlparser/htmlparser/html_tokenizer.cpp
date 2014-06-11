/**
 * @file html_tokenizer.cpp

 * @date 2011/08/02
 * @version 1.0
 * @brief html源代码读取器
 *
 **/

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "queue.h"
#include "html_pool.h"
#include "html_dtd.h"
#include "html_dom.h"
#include "html_attr.h"
#include "html_node.h"
#include "html_tokenizer.h"
#include "util/htmlparser/utils/string_util.h"

#define SCANNER_ASSERT() \
    assert(tk); \
    assert(doc);

/**公共操作，assert，取当前字符*/
#define SCANNER_COMMON(ch) \
    int (ch) = '\0'; \
    SCANNER_ASSERT() \
    (ch) = (tk)->ht_current[0];

/**
 * @brief 判断是否是空格符
 **/
#define IS_WS(ch) (g_whitespace_map[(unsigned char)(ch)])
/**
 * @brief 判断是否是拉丁字符
 **/
#define IS_LATIN(ch) (g_latin_map[(unsigned char)(ch)])

/**
 *	@brief 生成一个注释节点
 */
static int emit_comment(html_tokenizer_t *tk, html_tree_t *doc, const char *end)
{
	char *text = NULL;
	int len = 0;
	SCANNER_ASSERT();
	len = end - tk->ht_current;
	if (len == 0)
	{
		text = (char*) "";
	}
	else
	{
		text = html_tree_strndup(doc, tk->ht_current, len);
		if (text == NULL)
		{
			return -1;
		}
	}
	tk->ht_node = html_tree_create_comment(doc, text);
	if (tk->ht_node == NULL)
	{
		return -1;
	}
	tk->ht_node->html_tag.page_offset = tk->ht_current - 4 - tk->ht_source;
	return 0;
}

/**
 *	@brief 生成一个doctype节点
 */
static int emit_doctype(html_tokenizer_t *tk, html_tree_t *doc, const char *end)
{
	char *text = NULL;
	int len = 0;
	SCANNER_ASSERT();
	len = end - tk->ht_current;
	if (len == 0)
	{
		text = (char*) "";
	}
	else
	{
		text = html_tree_strndup(doc, tk->ht_current, len);
		if (text == NULL)
		{
			return -1;
		}
	}
	tk->ht_node = html_tree_create_doctype(doc, text);
	if (tk->ht_node == NULL)
	{
		return -1;
	}
	tk->ht_node->html_tag.page_offset = tk->ht_current - 9 - tk->ht_source;
	return 0;
}

/**
 *	@brief 生成一个text节点
 */
static int emit_text(html_tokenizer_t *tk, html_tree_t *doc, const char *end)
{
	char *text = NULL;
	html_node_t *node = NULL;
	int i = 0;
	int len = 0;
	SCANNER_ASSERT();
	len = end - tk->ht_begin;
	if (len > 0)
	{
		text = html_tree_strndup(doc, tk->ht_begin, len);
		if (text == NULL)
		{
			return -1;
		}
	}
	else
	{
		text = (char*) "";
	}
	node = html_tree_create_text_node(doc, text);
	if (node == NULL)
	{
		return -1;
	}

	node->html_tag.nodelength = len;
	node->html_tag.textlength = len; //shuangwei add 20120529
	node->html_tag.tag_type = TAG_WHITESPACE;
	node->html_tag.page_offset = tk->ht_begin - tk->ht_source;
	for (i = 0; i < len; i++)
	{
		if (!IS_WS(text[i]))
		{
			node->html_tag.tag_type = TAG_PURETEXT;
			break;
		}
	}
	tk->ht_node = node;
	return 0;
}

/**
 *	@brief 填充属性的name
 */
static int fill_attr_name(html_tokenizer_t *tk, html_tree_t *doc, const char *end)
{
	char *text = NULL;
	html_attr_type_t type = ATTR_UNKNOWN;
	int len = 0;
	int i = 0;
	SCANNER_ASSERT();
	assert(tk->ht_attr);
	len = end - tk->ht_attr->name;
	type = get_attr_type(tk->ht_attr->name, len);

	tk->ht_attr->type = type;
	if (type != ATTR_UNKNOWN)
	{
		tk->ht_attr->name = get_attr_name(type);
	}
	else
	{
		text = html_tree_strndup(doc, tk->ht_attr->name, len);
		if (text == NULL)
		{
			return -1;
		}
		for (i = 0; i < len; i++)
		{
			text[i] = (char) tolower(text[i]);
		}
		tk->ht_attr->name = text;
	}

	return 0;
}

/**
 *	@brief 填充属性的value
 */
static int fill_attr_value(html_tokenizer_t *tk, html_tree_t *doc, const char *end)
{
	char *text = NULL;
	SCANNER_ASSERT();
	assert(tk->ht_attr);

	text = html_tree_strndup(doc, tk->ht_attr->value, end - tk->ht_attr->value);
	if (text == NULL)
	{
		return -1;
	}
	tk->ht_attr->valuelength = end - tk->ht_attr->value; //shuangwei add 20120529
	tk->ht_attr->value = text;

	return 0;
}

/**
 *	@brief 在字符串中找到第一个目标字符，如果找不到，直至到达字符串末尾或者begin==0
 */
static inline const char* strchr_range(const char *begin, const char *end, int ch)
{
	while (*begin && begin < end)
	{
		if (*begin == ch)
		{
			return begin;
		}
		begin++;
	}
	return end;
}

/**
 *	@brief 跳过空白符，如果找不到空白符，直至到达字符串末尾或者begin==0
 */
static inline const char* skip_ws(const char *src, const char *end)
{
	while (*src && src < end)
	{
		if (!IS_WS(*src))
		{
			return src;
		}
		src++;
	}
	return end;
}

/**
 *	@brief 跳过字符，直至遇到特定字符，如果找不到空白符，直至到达字符串末尾或者begin==0
 */
static inline const char* skip_to(const char *src, const char *end, char *map)
{
	while (*src && src < end)
	{
		if (map[(unsigned char) *src])
		{
			return src;
		}
		src++;
	}
	return end;
}

/**tokenizer遍历网页源文件的几种状态 */
static int scan_tag_open(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_data(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_bogus_comment(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_rcdata(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_doctype(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_comment(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_self_closing_start_tag(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_attribute_value(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_after_attribute_name(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_attribute_name(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_before_attribute_name(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_tag_name(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_end_tag_open(html_tokenizer_t* tk, html_tree_t* doc);
static int scan_markup_decl_open(html_tokenizer_t* tk, html_tree_t* doc);

/**
 * @brief HTML Tokenizer 遍历到可能是标签的位置，吃进一个'<'
 **/
static int scan_tag_open(html_tokenizer_t* tk, html_tree_t* doc)
{
	SCANNER_COMMON(ch);
	if (tk->ht_current >= tk->ht_end || ch == '\0')
	{
		tk->ht_state = scan_data;
		return scan_data(tk, doc);
	}
	switch (ch)
	{
	case '!': /**注释？Doc？*/
		tk->ht_current++;
		return scan_markup_decl_open(tk, doc);
	case '/': /**结束标签*/
		tk->ht_current++;
		return scan_end_tag_open(tk, doc);
	case '?': /**一般是<?xml>，也可以认作是“假注释”*/
		return scan_bogus_comment(tk, doc);
	default:
		if (IS_LATIN(ch))
		{ /**拉丁字符*/
			/**当前字符离网页开始的距离大于1，即第一个'<'之前还有字符，需要生成text node*/
			if (tk->ht_current - tk->ht_begin > 1)
			{
				if (emit_text(tk, doc, tk->ht_current - 1) != 0)
				{
					return -1;
				}
				tk->ht_state = scan_tag_open;
				return 0;
			}
			/**创建一个节点*/
			tk->ht_node = html_tree_create_element_by_tag(doc, TAG_UNKNOWN);
			if (tk->ht_node == NULL)
			{
				return -1;
			}
			tk->ht_node->html_tag.page_offset = tk->ht_current - 1 - tk->ht_source;
			tk->ht_node->html_tag.tag_name = (char*) tk->ht_current;
			tk->ht_current++;
			/**开始遍历标签的name*/
			return scan_tag_name(tk, doc);
		}
		else
		{/**不是上述几种字符，直接将之前的'<'当作普通字符来处理（空格在此处亦是如此处理）*/
			tk->ht_state = scan_data;
			return scan_data(tk, doc);
		}
	}
}

/**
 * @brief HTML Tokenizer 遍历网页的开始状态，也是输出一个节点后的默认状态
 **/
static int scan_data(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *ch = NULL;
	assert(tk);
	assert(doc);
	/**找到第一个'<'*/
	ch = strchr_range(tk->ht_current, tk->ht_end, '<');
	/**返回的是结束指针*/
	if (ch == tk->ht_end)
	{
		/**当前节点和结束节点之间还有字符，可以组成一个text节点*/
		if (tk->ht_end - tk->ht_current > 0)
		{
			if (emit_text(tk, doc, tk->ht_end) != 0)
			{
				return -1;
			}
			/**当前遍历到结束点*/
			tk->ht_current = tk->ht_end;
			return 0;
		}
		/**当前节点和结束节点重合了，出现该情况一般是空网页造成的*/
		return 1;
	}
	else
	{
		/**当前遍历点指向下一个字符，并且转到下一个状态：找到一个可能是标签的点*/
		tk->ht_current = ch + 1;
		return scan_tag_open(tk, doc);
	}
}

/**
 * @brief HTML Tokenizer 遍历假注释的状态 '<!' or '<?' 但是得当作注释来处理
 **/
static int scan_bogus_comment(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *gt = NULL;
	SCANNER_ASSERT();

	gt = strchr_range(tk->ht_current, tk->ht_end, '>');
	if (emit_comment(tk, doc, gt) != 0)
	{
		return -1;
	}
	tk->ht_node->html_tag.page_offset += 3; /* bogus comment start as '<!' or '<?' not '<!--' */
	if (gt == tk->ht_end)
	{
		tk->ht_current = tk->ht_end;
	}
	else
	{
		tk->ht_current = gt + 1;
	}
	tk->ht_state = scan_data;
	return 0;
}

/*
 * RCDATA state will parse text until an appropriate end tag,
 * a.k.a <xyz>...</xyz> pattern
 * Find this tag, emit #text and return DATA state.
 */
/**
 * @brief HTML Tokenizer Scanner rcdata  相当于自定义标签
 **/
static int scan_rcdata(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *ch = NULL;
	int len = 0;
	SCANNER_ASSERT();

	if (!tk->ht_opening)
	{
		return -1;
	}
	len = strlen(tk->ht_opening->html_tag.tag_name);
	ch = tk->ht_current;
	while (1)
	{
		ch = strchr_range(ch, tk->ht_end, '<');
		if (ch == tk->ht_end)
		{
			tk->ht_current = tk->ht_end;
			if (emit_text(tk, doc, tk->ht_current) != 0)
			{
				return -1;
			}
			tk->ht_state = scan_data;
			return 0;
		}
		if (tk->ht_end - ch > len + 2 && ch[1] == '/')
		{
			/**找到正在等待关闭的标签的匹配标签*/
			if (strncasecmp(ch + 2, tk->ht_opening->html_tag.tag_name, len) == 0)
			{
				tk->ht_current = ch; /* record the text end boundary */
				ch += 2 + len;
				if (ch[0] == '>')
				{
					if (emit_text(tk, doc, tk->ht_current))
					{
						return -1;
					}
					tk->ht_current = ch + 1;
					tk->ht_state = scan_data;
					return 0;
				}
				else if (IS_WS(ch[0]) || ch[0] == '/')
				{
					ch = strchr_range(ch, tk->ht_end, '>');
					if (ch == tk->ht_end)
					{
						if (emit_text(tk, doc, tk->ht_end) != 0)
						{
							return -1;
						}
						tk->ht_current = tk->ht_end;
					}
					else
					{
						if (emit_text(tk, doc, tk->ht_current) != 0)
						{
							return -1;
						}
						tk->ht_current = ch + 1;
					}
					tk->ht_state = scan_data;
					return 0;
				}
			}
			else
			{
				ch += 2;
			}
		}
		else
		{
			ch += 1;
		}
	}
	return -1;
}

/**
 * @brief HTML Tokenizer 扫描到doctype节点
 **/
static int scan_doctype(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *gt = NULL;
	SCANNER_ASSERT();
	gt = strchr_range(tk->ht_current, tk->ht_end, '>');
	if (emit_doctype(tk, doc, gt) != 0)
	{
		return -1;
	}
	if (gt == tk->ht_end)
	{
		tk->ht_current = tk->ht_end;
	}
	else
	{
		tk->ht_current = gt + 1;
	}
	tk->ht_state = scan_data;
	return 0;
}

/*
 * A COMMENT generally follows the pattern <!--comment-->,
 * '-->' can also be '--!>' or '--(whitespace)>',
 * or to the end of input stream
 */
/**
 * @brief HTML Tokenizer扫描到注释的状态 <!--comment-->, -->' 可以是 '--!>' or '-- >
 **/
static int scan_comment(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *ch = NULL;
	const char *minus = NULL;
	const char *gt = NULL;
	SCANNER_ASSERT();

	/* tk->ht_current points to the first character after <!-- */
	ch = tk->ht_current;
	while ((minus = strchr_range(ch, tk->ht_end, '-')) != tk->ht_end)
	{
		gt = strchr_range(minus, tk->ht_end, '>');
		if (gt == tk->ht_end)
		{
			break;
		}
		else if (minus[1] == '-')
		{
			if (minus + 2 == gt)
			{
				/* --> */
				if (emit_comment(tk, doc, minus) != 0)
				{
					return -1;
				}
				tk->ht_current = gt + 1;
				tk->ht_state = scan_data;
				return 0;
			}
			else if (minus + 3 == gt)
			{
				if (minus[2] == '!' || IS_WS(minus[2]))
				{
					/* --!> or --(whitespace)> */
					if (emit_comment(tk, doc, minus) != 0)
					{
						return -1;
					}
					tk->ht_current = gt + 1;
					tk->ht_state = scan_data;
					return 0;
				}
				else
				{
					/* --(other)>, consume these as comment, go after gt */
					ch = minus + 1;
					continue;
				}
			}
			else
			{
				/* --(.{2,})>, go forward one minus */
				ch = minus + 1;
				continue;
			}
		}
		else
		{
			/* -(^-)(.*)>, go forward */
			ch = minus + 2;
			continue;
		}
	}
	ch = tk->ht_end; /* ch is eof */
	if (ch != tk->ht_current)
	{
		if (*(ch - 1) == '-')
		{
			ch -= 2;
			if (*ch == '-' && ch >= tk->ht_current)
			{
				/* pattern 1 */
				gt = ch;
			}
			else
			{
				/* pattern 2 */
				gt = ch + 1;
			}
		}
		else
		{
			/* pattern 3 */
			gt = ch;
		}
	}
	else
	{
		/* pattern 4 */
		gt = ch;
	}
	if (emit_comment(tk, doc, gt) != 0)
	{
		return -1;
	}
	tk->ht_current = ch;
	tk->ht_state = scan_data;
	return 0;
}

/**
 * @brief HTML Tokenizer 扫描到自关闭标签的结尾处
 **/
static int scan_self_closing_start_tag(html_tokenizer_t* tk, html_tree_t* doc)
{
	SCANNER_COMMON(ch);
	if (tk->ht_current >= tk->ht_end || ch == '\0')
	{
		return 1;
	}
	/**只有"/>"组合才能判定处于自关闭标签中*/
	if (ch == '>')
	{
		assert(tk->ht_node);
		tk->ht_node->html_tag.is_self_closed = 1;
		if (tk->ht_node->html_tag.page_offset >= 0)
		{
			tk->ht_node->html_tag.nodelength = tk->ht_current + 1 - tk->ht_source;
		}

		tk->ht_current++;
		tk->ht_state = scan_data;
		return 0;
	}
	else
	{/**否则进入等待attribute的状态*/
		return scan_before_attribute_name(tk, doc);
	}
}

/**
 * @brief HTML Tokenizer 遍历到属性的value处
 **/
static int scan_attribute_value(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *ch = NULL;
	char chr = '\0';
	SCANNER_ASSERT();

	tk->ht_current = skip_ws(tk->ht_current, tk->ht_end);
	/**万年不变的定律，到末尾了就结束*/
	if (tk->ht_current == tk->ht_end)
	{
		return 1;
	}

	chr = tk->ht_current[0];
	/**如果是引号，则直接跳到对称符号出*/
	if (chr == '"' || chr == '\'')
	{
		//shuangwei modify,skip the " or ' in the value
		ch = strchr_range(tk->ht_current + 1, tk->ht_end, chr);
		if (ch == tk->ht_end)
		{
			return 1;
		}
//		const char * beginpot=tk->ht_current + 1;
//		//const char *pgt=strchr_range(beginpot,tk->ht_end,'>');
//
//		do{
//			ch = strchr_range(beginpot, tk->ht_end, chr);
//
//			if (ch == tk->ht_end) {
//			 return 1;
//			}
//			beginpot=ch+1;
//		}while(((ch+1) < tk->ht_end)&&!(IS_WS(*(ch+1))||*(ch+1) == '>'||*(ch+1) == '/')||((ch+2) < tk->ht_end&&(*(ch+2))=='>'));
//		//shuangwei modify,skip the " or ' in the value
		assert(tk->ht_attr);
		tk->ht_attr->value = (char*) tk->ht_current + 1;
		/**填充属性的value*/
		if (fill_attr_value(tk, doc, ch) != 0)
		{
			return -1;
		}
		assert(tk->ht_node);
		html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
		tk->ht_attr = NULL;
		tk->ht_current = ch + 1;

		if (tk->ht_current >= tk->ht_end)
		{
			return 1;
		}
		chr = tk->ht_current[0];

		if (IS_WS(chr))
		{
			tk->ht_current++;
			return scan_before_attribute_name(tk, doc);
		}
		else if (chr == '>')
		{
			assert(tk->ht_node);
			tk->ht_state = scan_data;
			tk->ht_current++;
			return 0;
		}
		else if (chr == '/')
		{
			tk->ht_current++;
			return scan_self_closing_start_tag(tk, doc);
		}
		else
		{
			return scan_before_attribute_name(tk, doc);
		}
	}
	else if (chr == '>')
	{
		assert(tk->ht_node);
		assert(tk->ht_attr);
		html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
		tk->ht_current++;
		tk->ht_state = scan_data;
		return 0;
	}
	else
	{
		tk->ht_attr->value = (char*) tk->ht_current;
		ch = skip_to(tk->ht_current, tk->ht_end, g_attribute_value_uq_map);
		if (ch == tk->ht_end)
		{
			return 1;
		}
		assert(tk->ht_attr);
		if (fill_attr_value(tk, doc, ch) != 0)
		{
			return -1;
		}
		assert(tk->ht_node);
		html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
		tk->ht_attr = NULL;
		tk->ht_current = ch + 1;
		chr = ch[0];
		if (IS_WS(chr))
		{
			return scan_before_attribute_name(tk, doc);
		}
		else
		{
			assert(chr == '>');
			tk->ht_state = scan_data;
			return 0;
		}
	}
}

/**
 * @brief HTML Tokenizer Scanner after_attribute_name
 **/
static int scan_after_attribute_name(html_tokenizer_t* tk, html_tree_t* doc)
{
	char ch = '\0';
	SCANNER_ASSERT();

	tk->ht_current = skip_ws(tk->ht_current, tk->ht_end);
	if (tk->ht_current == tk->ht_end)
	{
		return 1;
	}

	ch = tk->ht_current[0];
	tk->ht_current++;
	if (ch == '=')
	{
		return scan_attribute_value(tk, doc);
	}
	else if (ch == '>')
	{
		assert(tk->ht_node);
		assert(tk->ht_attr);
		html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
		tk->ht_state = scan_data;
		return 0;
	}
	else if (ch == '/')
	{
		assert(tk->ht_node);
		assert(tk->ht_attr);
		html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
		return scan_self_closing_start_tag(tk, doc);
	}
	else
	{
		assert(tk->ht_node);
		assert(tk->ht_attr);
		html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
		tk->ht_attr = html_tree_create_attribute_by_tag(doc, ATTR_UNKNOWN);
		if (tk->ht_attr == NULL)
		{
			return -1;
		}
		tk->ht_attr->name = tk->ht_current - 1;
		return scan_attribute_name(tk, doc);
	}
}

/**
 * @brief HTML Tokenizer 扫描attribute的name的状态
 **/
static int scan_attribute_name(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *ch = NULL;
	char chr = '\0';
	SCANNER_ASSERT();
	/**到  whitespace / = > 停止*/
	ch = skip_to(tk->ht_current, tk->ht_end, g_attribute_name_map);
	if (ch == tk->ht_end)
	{
		return 1;
	}
	else
	{
		/**完善attribute的名称*/
		assert(tk->ht_attr);
		if (fill_attr_name(tk, doc, ch) != 0)
		{
			return -1;
		}
		tk->ht_current = ch + 1;
		chr = ch[0];
		if (IS_WS(chr))
		{ /**空格，只能说明属性名称遍历完毕*/
			return scan_after_attribute_name(tk, doc);
		}
		else if (chr == '=')
		{ /**'='说明是attribute=value的模式*/
			return scan_attribute_value(tk, doc);
		}
		else if (chr == '>')
		{ /**'>'，说明该属性遍历结束，可以添加到节点上*/
			html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
			tk->ht_state = scan_data;
			return 0;
		}
		else
		{
			assert(chr == '/');
			/**同'>'一样，但是同时说明了该节点是一个自关闭节点*/
			html_node_set_attribute_by_name(tk->ht_node, tk->ht_attr);
			return scan_self_closing_start_tag(tk, doc);
		}
	}
}

/**
 * @brief HTML Tokenizer 处于attribute名称之前，准备接收attribute名称的状态
 **/
static int scan_before_attribute_name(html_tokenizer_t* tk, html_tree_t* doc)
{
	char ch = '\0';
	SCANNER_ASSERT();
	/**跳过空格*/
	tk->ht_current = skip_ws(tk->ht_current, tk->ht_end);
	if (tk->ht_current == tk->ht_end)
	{
		return 1;
	}
	ch = tk->ht_current[0];
	/**标签名称后面紧跟的是'/'，说明有可能是一个自关闭标签*/
	if (ch == '/')
	{
		tk->ht_current++;
		return scan_self_closing_start_tag(tk, doc);
	}
	else if (ch == '>')
	{
		tk->ht_current++;
		tk->ht_state = scan_data;
		return 0;
	}
	else
	{
		/**创建一个unknow属性*/
		tk->ht_attr = html_tree_create_attribute_by_tag(doc, ATTR_UNKNOWN);
		tk->ht_attr->name = tk->ht_current;
		tk->ht_current++;
		return scan_attribute_name(tk, doc);
	}
}

/**
 * @brief HTML Tokenizer 开始遍历标签的name
 **/
static int scan_tag_name(html_tokenizer_t* tk, html_tree_t* doc)
{
	const char *ch = NULL;
	html_tag_type_t type = TAG_UNKNOWN;
	char *text = NULL;
	int len = 0;
	int i = 0;
	char chr = '\0';
	SCANNER_ASSERT();
	/**遍历到指定字符处*/
	ch = skip_to(tk->ht_current, tk->ht_end, g_tag_name_map);
	if (ch == tk->ht_end)
	{
		return 1;
	}
	else
	{
		assert(tk->ht_node);
		len = ch - tk->ht_node->html_tag.tag_name;
		//todo
		/**获取标签类型*/
		/**缺少第一个map*/
		type = get_tag_type(tk->ht_node->html_tag.tag_name, len);
		//todo
		tk->ht_node->html_tag.tag_type = type;
		/**获取标签name*/
		if (type != TAG_UNKNOWN)
		{
			tk->ht_node->html_tag.tag_name = get_tag_name(type);
		}
		else
		{
			text = html_tree_strndup(doc, tk->ht_node->html_tag.tag_name, len);
			if (text == NULL)
			{
				return -1;
			}
			for (i = 0; i < len; i++)
			{/**初始化为小写*/
				text[i] = tolower(text[i]);
			}
			tk->ht_node->html_tag.tag_name = text;
		}
		tk->ht_current = ch + 1;
		chr = ch[0];
		if (IS_WS(chr))
		{/**标签名称后面紧跟的是空格，则说明该标签还有属性*/
			return scan_before_attribute_name(tk, doc);
		}
		else if (chr == '>')
		{ /**标签名称后面紧跟的是'>'，说明该标签的内容已经结束*/
			if (tk->ht_node->html_tag.is_close_tag && tk->ht_node->html_tag.page_offset >= 0)
			{
				tk->ht_node->html_tag.page_offset = ch + 1 - tk->ht_source;
			}
			tk->ht_state = scan_data;
			return 0;
		}
		else
		{ /**标签名称后面紧跟的是'/'，说明有可能是一个自关闭标签*/
			assert(chr == '/');
			return scan_self_closing_start_tag(tk, doc);
		}
	}
}

/**
 * @brief HTML Tokenizer 扫描到标签结束符
 **/
static int scan_end_tag_open(html_tokenizer_t* tk, html_tree_t* doc)
{
	SCANNER_COMMON(ch);

	if (tk->ht_current >= tk->ht_end || ch == '\0')
	{
		if (tk->ht_current - tk->ht_begin > 0)
		{
			if (emit_text(tk, doc, tk->ht_current) != 0)
			{
				return -1;
			};
			tk->ht_state = scan_end_tag_open;
			return 0;
		}
		return scan_data(tk, doc);
	}
	else if (IS_LATIN(ch))
	{/**拉丁字符，即出现</a>之类的情况，但是需要判断<之前是否有字符*/
		if (tk->ht_current - tk->ht_begin > 2)
		{
			if (emit_text(tk, doc, tk->ht_current - 2) != 0)
			{
				return -1;
			}
			tk->ht_state = scan_end_tag_open;
			return 0;
		}
		tk->ht_node = html_tree_create_element_by_tag(doc, TAG_UNKNOWN);
		if (tk->ht_node == NULL)
		{
			return -1;
		}
		tk->ht_node->html_tag.page_offset = tk->ht_current - 2 - tk->ht_source;
		tk->ht_node->html_tag.tag_name = (char*) tk->ht_current;
		tk->ht_node->html_tag.is_close_tag = 1;
		return scan_tag_name(tk, doc);
	}
	else if (ch == '>')
	{/**</> 直接忽略*/
		tk->ht_current++;
		return scan_data(tk, doc);
	}
	else
	{
		tk->ht_current--; /* preserve the first character */
		return scan_bogus_comment(tk, doc);
	}
}

/**
 * @brief HTML Tokenizer 除拉丁标签外的其他标签
 **/
static int scan_markup_decl_open(html_tokenizer_t* tk, html_tree_t* doc)
{
	SCANNER_ASSERT();
	/**<!-- 注释*/
	if (tk->ht_end - tk->ht_current >= 2 && strncmp(tk->ht_current, "--", 2) == 0)
	{
		if (tk->ht_current - tk->ht_begin > 2)
		{
			if (emit_text(tk, doc, tk->ht_current - 2) != 0)
			{
			};
			tk->ht_state = scan_markup_decl_open;
			return 0;
		}
		tk->ht_current += 2;
		return scan_comment(tk, doc);
	}
	else if (tk->ht_end - tk->ht_current >= 7 && strncasecmp(tk->ht_current, "DOCTYPE", 7) == 0)
	{ /**<!DOCTYPE */
		/**文本节点永远都是在确认下一个节点之后“补”的*/
		if (tk->ht_current - tk->ht_begin > 2)
		{
			if (emit_text(tk, doc, tk->ht_current - 2) != 0)
			{
				return -1;
			};
			tk->ht_state = scan_markup_decl_open;
			return 0;
		}
		tk->ht_current += 7;
		return scan_doctype(tk, doc);
	}
	else
	{
		if (tk->ht_current - tk->ht_begin > 2)
		{
			if (emit_text(tk, doc, tk->ht_current - 2) != 0)
			{
				return -1;
			};
			tk->ht_state = scan_markup_decl_open;
			return 0;
		}
		tk->ht_current--; /* preserve the '!' from '<!' */
		return scan_bogus_comment(tk, doc);
	}
}

html_tokenizer_t* html_tokenizer_create(struct mem_pool_t *pool)
{
	html_tokenizer_t *tokenizer = NULL;
	assert(pool);
	tokenizer = (html_tokenizer_t*) palloc(pool, sizeof(*tokenizer));
	if (tokenizer == NULL)
	{
		return NULL;
	}
	memset(tokenizer, 0, sizeof(*tokenizer));
	return tokenizer;
}

void html_tokenizer_reset(html_tokenizer_t *tokenizer, const char *html, size_t size)
{
	assert(tokenizer);
	tokenizer->ht_source = html;
	tokenizer->ht_begin = html;
	tokenizer->ht_current = html;
	tokenizer->ht_end = tokenizer->ht_source + size;
	tokenizer->ht_state = scan_data;
	tokenizer->ht_node = NULL;
	tokenizer->ht_attr = NULL;
	tokenizer->ht_opening = NULL;
}

/**
 * @brief 遍历页面源代码
 **/
html_node_t* html_tokenize(html_tokenizer_t *tokenizer, html_tree_t *doc)
{
	html_node_t *node = NULL;
	assert(tokenizer);
	assert(doc);
	while (1)
	{
		/**遍历结果不为0，均为出错*/
		if (tokenizer->ht_state(tokenizer, doc) != 0)
		{
			return NULL;
		}
		node = tokenizer->ht_node;
		assert(node);
		tokenizer->ht_begin = tokenizer->ht_current;
		tokenizer->ht_node = NULL;
		tokenizer->ht_attr = NULL;
		if (!node->html_tag.is_close_tag && node->html_tag.tag_name)
		{
			tokenizer->ht_opening = node;
		}
		return node;
	}
	return NULL;
}

void html_tokenizer_switch_to_rcdata(html_tokenizer_t *tokenizer)
{
	assert(tokenizer);
	tokenizer->ht_state = scan_rcdata;
}
