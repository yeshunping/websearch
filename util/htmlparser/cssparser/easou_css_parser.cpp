/**
 * easou_css_parser.cpp
 * Description: CSS解析
 *  Created on: 2011-06-20
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "easou_html_attr.h"
#include "easou_html_tree.h"
#include "easou_css_parser.h"
#include "easou_css_nodepool.h"
#include "easou_css_dtd.h"
#include "easou_string.h"
#include "easou_debug.h"

using namespace EA_COMMON;

/**
 * @brief CSS
 */
#define PROP_FIRST_ALPHA(PROP_NAME)	(PROP_NAME[0]=='-'?PROP_NAME[1]:PROP_NAME[0])

/**
 * @brief 清除css信息
 * @param [in/out] css   : easou_css_t*
 * @return  void 
 * @author xunwu
 * @date 2011/06/20
 **/
void css_clean(easou_css_t *css)
{
	css->all_ruleset_list = NULL;
	css->css_inner.str_heap.p_heap_avail = css->css_inner.str_heap.p_heap;
	css_nodepool_reset(&(css->css_inner.nodepool));
	hashmap_clean(css->index.class_map);
	for (int i = 0; i < TAG_TYPE_NUM_LIMIT; i++)
		css->index.type_index[i] = NULL;
}

/**
 * @brief 创建一个css解析结构
 * @param [in] max_css_text_size
 * @return  easou_css_t*
 * @author xunwu
 * @date 2011/06/21
 **/
easou_css_t *css_create(int max_css_text_size)
{
	easou_css_t *css = NULL;
	easou_css_inner_t *p_inner = NULL;

	if (max_css_text_size <= 0)
	{
	    // FIXME:跟函数的定义不符合，应该返回-1。被调用函数不能打这种补丁。
		return css_create();
	}
	/* alloc easou_css_t */
	css = (easou_css_t*) calloc(sizeof(easou_css_t), 1);
	if (NULL == css)
	{
		Fatal((char*) "%s:%s:malloc easou_css_t error!", __FILE__, __FUNCTION__);
		goto FAIL_CC;
	}
	/* alloc string heap */
	p_inner = &(css->css_inner);
	p_inner->str_heap.p_heap = (char *) calloc(max_css_text_size, 1);
	if (NULL == p_inner->str_heap.p_heap)
	{
		Fatal((char*) "%s:%s:malloc str_heap error!", __FILE__, __FUNCTION__);
		goto FAIL_CC;
	}
	p_inner->str_heap.heap_size = max_css_text_size;

	/* alloc nodepool */
	if (css_nodepool_init(&(p_inner->nodepool), max_css_text_size) < 0)
	{
		goto FAIL_CC;
	}

	css->index.class_map = hashmap_create();
	if (css->index.class_map == NULL)
		goto FAIL_CC;


	/* init */
	css->all_ruleset_list = NULL;
	css->css_inner.str_heap.p_heap_avail = css->css_inner.str_heap.p_heap;
	return css;

	FAIL_CC: if (css)
	{
		css_destroy(css);
		css = NULL;
	}
	return NULL;
}

/**
 * @brief 销毁css
 * @param [in/out] css   : easou_css_t*
 * @author xunwu
 * @date 2011/06/20
 **/
void css_destroy(easou_css_t *css)
{
	css_nodepool_destroy(&(css->css_inner.nodepool));
	if (css->css_inner.str_heap.p_heap != NULL)
	{
		free(css->css_inner.str_heap.p_heap);
		css->css_inner.str_heap.p_heap = NULL;
	}
	if (css->index.class_map != NULL)
	{
		hashmap_destroy(css->index.class_map);
		css->index.class_map = NULL;
	}
	free(css);
}

/**
 * @brief 创建一个css解析结构
 * @param [in] max_css_text_size
 * @return  easou_css_t*
 * @author xunwu
 * @date 2011/06/21
 **/
easou_css_t *css_create()
{
	return css_create(CSS_DEFAULT_TEXT_SIZE);
}

int get_new_token(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap, easou_css_t *css)
{
	scan_ignore(css_scan, css);
	css_scan->p_curr_token = str_heap->p_heap_avail;
	css_scan->p_curr_token_len = 0;
	if (*(css_scan->p_next) == '\0')
	{
		return -1;
	}
	if (strncmp(css_scan->p_next, "url(", 4) == 0)
	{
		memcpy(str_heap->p_heap_avail, css_scan->p_next, 4);
		str_heap->p_heap_avail += 4;
		css_scan->p_next += 4;
		scan_ignore(css_scan, css);
		if (*(css_scan->p_next) == '\'' || *(css_scan->p_next) == '"')
		{
			scan_string(css_scan, str_heap);
		}
		else
		{
			scan_url(css_scan, str_heap);
		}
		scan_ignore(css_scan, css);
		if (*(css_scan->p_next) == ')')
		{
			*(str_heap->p_heap_avail++) = *(css_scan->p_next++);
			if (is_css_sep_chr_state(css_scan->state, *(css_scan->p_next)) == true)
			{
				css_scan->p_curr_token_len = str_heap->p_heap_avail - css_scan->p_curr_token - 1;
				*(str_heap->p_heap_avail++) = '\0';
				return CSS_TOKEN_URI;
			}
		}
	}

	switch (*(css_scan->p_next))
	{
	case '\'':
	case '"':
		scan_string(css_scan, str_heap);
		if (is_css_sep_chr_state(css_scan->state, *(css_scan->p_next)) == true)
		{
			*(str_heap->p_heap_avail++) = '\0';
			css_scan->p_curr_token_len = str_heap->p_heap_avail - css_scan->p_curr_token - 1;
			return CSS_TOKEN_STRING;
		}
		// FIXME(error):break
		// break;
	default:
		scan_normal(css_scan, str_heap);
		*(str_heap->p_heap_avail++) = '\0';
		css_scan->p_curr_token_len = str_heap->p_heap_avail - css_scan->p_curr_token - 1;
		return CSS_TOKEN_NORMAL;
	}
}

/**
 * @brief	分析属性选择子
 *	assert no spaces in the attr string.
 * @param [in/out] simple_selector   : css_simple_selector_t*
 * @param [in/out] pstr   : char*
 * @param [in/out] css   : easou_css_t*
 * @return  char*
 * @retval   
 * @see 
 * @author xunwu
 * @date 2011/06/20
 **/
static char* parse_attr_selector(easou_css_simple_selector_t *simple_selector, char *pstr, easou_css_t *css)
{
	easou_css_attr_selector_t *attr_selector = (easou_css_attr_selector_t *) css_get_from_nodepool(&(css->css_inner.nodepool),
			sizeof(easou_css_attr_selector_t));
	/* extract the attr string out */
	char *ptend = pstr;
	ptend = strchr(ptend, ']');
	if (ptend == NULL)
	{
	    // FIXME(warn):strlen
		ptend = pstr + strlen(pstr);
		return ptend;
	}
	*ptend = '\0';
	/* extract attr name and value*/
	char *pt = pstr;
	attr_selector->attr.name = pt;
	pt = strchr(pt, '=');
	if (pt == NULL)
	{
		attr_selector->attr.value = NULL;
		attr_selector->type = CSS_ATTR_SELECT_MATCH_NOVALUE;
	}
	else
	{
		switch (*(pt - 1))
		{
		case '~':
			attr_selector->type = CSS_ATTR_SELECT_MATCH_ANY;
			*(pt - 1) = '\0';
			break;
		case '|':
			attr_selector->type = CSS_ATTR_SELECT_MATCH_BEGIN;
			*(pt - 1) = '\0';
			break;
		default:
			attr_selector->type = CSS_ATTR_SELECT_MATCH_EXACTLY;
			*pt = '\0';
			break;
		}

		if (*(pt + 1) == '"')
			attr_selector->attr.value = pt + 2;
		else
			attr_selector->attr.value = pt + 1;
		char *val_end = strchr(attr_selector->attr.value, '"');
		if (val_end != NULL)
			*val_end = '\0';
	}
	/* hang the attrib on the simple_selector*/
	attr_selector->next = simple_selector->attr_selector;
	simple_selector->attr_selector = attr_selector;

	return ptend + 1;
}

/**
 * @brief	分析普通选择子
 *	simple_selector : element_name? [ HASH | class | attrib | pseudo ]* S*
 * @author xunwu
 * @date 2011/06/20
 **/
static void parse_simple_selector(easou_css_simple_selector_t *simple_selector, easou_css_t *css)
{
	char *pstr = simple_selector->name;

	int type = 0;
	while (*pstr != '\0')
	{
		switch (*pstr)
		{
		case ':':
			*pstr = '\0';
			simple_selector->pseudo_selector = ++pstr;
			type |= 1;
			break;
		case '[':
			*pstr = '\0';
			pstr = parse_attr_selector(simple_selector, ++pstr, css);
			type |= 2;
			break;
		case '.':
			*pstr = '\0';
			simple_selector->class_selector = ++pstr;
			type |= 4;
			break;
		case '#':
			*pstr = '\0';
			simple_selector->id_selector = ++pstr;
			type |= 8;
			break;
		default:
			pstr++;
			break;
		}
	}

#ifdef DEBUG_INFO
	char buf[256];
	int ret = snprintf(buf, 256, "simple_selector_%d", type);
	buf[ret] = 0;
#endif

	if (simple_selector->name[0] == '\0' || (simple_selector->name[0] == '*' && simple_selector->name[1] == '\0'))
	{
		simple_selector->tag_type = CSS_UNIVERSAL_SELECTOR;
		if (type == 4)
		{
			simple_selector->type = SIMPLE_SELECTOR_CLASS;
			counter_add("simple_selector_class", 1);
		}
		else if (type == 8)
		{
			simple_selector->type = SIMPLE_SELECTOR_ID;
			counter_add("simple_selector_id", 1);
		}
		else if (type == 2)
		{
			simple_selector->type = SIMPLE_SELECTOR_ATTR;
			counter_add("simple_selector_attr", 1);
		}
		else if (type == 0)
		{
			simple_selector->type = SIMPLE_SELECTOR_ALL;
			counter_add("simple_selector_all", 1);
		}
		else
		{
			simple_selector->type = SIMPLE_SELECTOR_OTHER;
			counter_add("simple_selector_other", 1);
			counter_add(buf, 1);
		}
	}
	else
	{
		simple_selector->tag_type = get_tag_type(simple_selector->name, strlen(simple_selector->name));
		if (type == 4)
		{
			simple_selector->type = SIMPLE_SELECTOR_TAGCLASS;
			counter_add("simple_selector_tagclass", 1);
		}
		else if (type == 8)
		{
			simple_selector->type = SIMPLE_SELECTOR_TAGID;
			counter_add("simple_selector_tagid", 1);
		}
		else if (type == 0)
		{
			simple_selector->type = SIMPLE_SELECTOR_TAG;
			counter_add("simple_selector_tag", 1);
		}
		else if (type == 2)
		{
			simple_selector->type = SIMPLE_SELECTOR_TAGATTR;
			counter_add("simple_selector_tagattr", 1);
		}
		else
		{
			simple_selector->type = SIMPLE_SELECTOR_OTHER;
			counter_add("simple_selector_other", 1);
			counter_add(buf, 1);
		}
	}
	if (type & 1 == 1)
		simple_selector->type = SIMPLE_SELECTOR_PSEUDO;
}

/**
 * @brief	清空选择子
 * @author xunwu
 * @date 2011/06/20
 **/
static void selector_clean(easou_css_selector_t *selector)
{
	selector->pre_selector = NULL;
	selector->combinator = CSS_NON_COMBINATOR;
	selector->simple_selector.name = NULL;
	selector->simple_selector.attr_selector = NULL;
	selector->simple_selector.class_selector = NULL;
	selector->simple_selector.id_selector = NULL;
	selector->simple_selector.pseudo_selector = NULL;
}

/**
 * @brief 普通选择子是否合法
 * @return  bool 
 * @retval   
 * @see 
 * @author xunwu
 * @date 2011/06/20
 **/
static bool is_illegal_simple_selector(const easou_css_simple_selector_t *sslt)
{
	if (sslt->tag_type == TAG_UNKNOWN)
	{
		return true;
	}
	if (sslt->id_selector && sslt->id_selector[0] == '\0')
	{
		return true;
	}
	if (sslt->class_selector && sslt->class_selector[0] == '\0')
	{
		return true;
	}
	if (sslt->pseudo_selector && sslt->pseudo_selector[0] == '\0')
	{
		return true;
	}
	return false;
}

/**
 * @brief 创建选择子
 * @param [in/out] css_scan   : css_scan_t*
 * @param [in/out] css   : easou_css_t*
 * @return  css_selector_t* 
 * @author xunwu
 * @date 2011/06/20
 **/
easou_css_selector_t *make_selector(easou_css_scan_t *css_scan, easou_css_t *css)
{
	easou_css_selector_t *selector = NULL;
	easou_css_selector_combinator_t curr_combinator = CSS_NON_COMBINATOR;
	easou_css_str_heap_t *str_heap = &(css->css_inner.str_heap);
	easou_css_nodepool_t *pool = &(css->css_inner.nodepool);
	while (1)
	{
		/* scan token */
		if (get_new_token(css_scan, str_heap, css) < 0)
		{
			goto FAIL_MS;
		}
		if (is_css_sep_chr_state(css_scan->state, *(css_scan->p_next)) == false)
		{
			goto FAIL_MS;
		}
		if (css_scan->p_curr_token[0] == '\0')
		{
			break;
		}
		/* get memory */
		easou_css_selector_t *new_selector = (easou_css_selector_t *) css_get_from_nodepool(pool, sizeof(easou_css_selector_t));
		if (NULL == new_selector)
		{
			Fatal((char*) "%s:%s:get from nodepool error!", __FILE__, __FUNCTION__);
			goto FAIL_MS;
		}
		/* make new selector */
		selector_clean(new_selector);
		new_selector->pre_selector = selector;
		new_selector->combinator = curr_combinator;
		new_selector->simple_selector.name = css_scan->p_curr_token;
		if (css_scan->p_curr_token_len >= 256)
		{
			css_scan->p_next = skip_current_selector(css_scan->p_next);
			goto FAIL_MS;
		}
		parse_simple_selector(&(new_selector->simple_selector), css);
		if (is_illegal_simple_selector(&(new_selector->simple_selector)))
		{
			css_scan->p_next = skip_current_selector(css_scan->p_next);
			goto FAIL_MS;
		}
		/* combinator */
		scan_ignore(css_scan, css);
		if (*(css_scan->p_next) == '+')
		{
			curr_combinator = CSS_ADJACENT_COMBINATOR;
			css_scan->p_next++;
			scan_ignore(css_scan, css);
		}
		else if (*(css_scan->p_next) == '>')
		{
			curr_combinator = CSS_CHILD_COMBINATOR;
			css_scan->p_next++;
			scan_ignore(css_scan, css);
		}
		else
		{
			curr_combinator = CSS_DESCEND_COMBINATOR;
		}
		selector = new_selector;
		/* end of selector? */
		if (*(css_scan->p_next) == '{' || *(css_scan->p_next) == ',' || *(css_scan->p_next) == '\0')
			break;
	}

	return selector;
	FAIL_MS: return NULL;
}

/**
 * @brief
 * @param [in/out] name   : const char*
 * @return  css_prop_type_t 
 * @retval   
 * @see 
 * @author xunwu
 * @date 2011/06/20
 **/
easou_css_prop_type_t css_seek_prop_type(const char *name)
{
	char first_alpha = '\0';
	if (name != NULL && name[0] != '\0')
	{
		if (name[0] == '-')
			first_alpha = name[1];
		else
			first_alpha = name[0];
		int begin_index = css_prop_first_char_map[(unsigned char) first_alpha];
		if (begin_index < 0)
			return CSS_PROP_UNKNOWN;
		for (unsigned int i = begin_index; i < CSS_PROPERTY_NUM && PROP_FIRST_ALPHA(css_property_name_array[i]) == first_alpha; i++)
		{
			if (strcmp(name, css_property_name_array[i]) == 0)
			{
				return prop_typeinfo_array[i].type;
			}
		}
	}
	return CSS_PROP_UNKNOWN;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/20
 **/
bool is_useful_css_prop(easou_css_prop_type_t prop)
{
	if (prop_typeinfo_array[prop].is_geo_prop || prop_typeinfo_array[prop].is_font_prop)
	{
		return true;
	}
	if(css_other_usefull_prop[prop])
	{
		return true;
	}
	return false;
}

/**
 * @brief 得到属性的值
 * @author xunwu
 * @date 2011/06/20
 **/
static char *get_prop_value(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap, easou_css_t *css)
{
	char *cur_val = NULL;
	css_scan->state = CSS_STAT_SCAN_PROP_VALUE;
	do
	{
		if (get_new_token(css_scan, str_heap, css) < 0)
			return NULL;
		if (is_css_sep_chr_state(css_scan->state, *(css_scan->p_next)) == false)
			return NULL;
		if (cur_val == NULL)
			cur_val = css_scan->p_curr_token;
		else if (css_scan->p_curr_token != NULL)
		{
			sprintf(cur_val, "%s %s", cur_val, css_scan->p_curr_token);
		}
		scan_ignore(css_scan, css);
	} while (*(css_scan->p_next) != ';' && *(css_scan->p_next) != '}' && *(css_scan->p_next) != '\0');
	return cur_val;
}

/**
 * @brief	得到属性的name
 * @author xunwu
 * @date 2011/06/20
 **/
static char *get_prop_name(easou_css_scan_t *css_scan, easou_css_str_heap_t *str_heap, easou_css_t *css)
{
	char *cur_name = NULL;
	css_scan->state = CSS_STAT_SCAN_PROP_NAME;
	if (get_new_token(css_scan, str_heap, css) < 0)
		return NULL;
	if (is_css_sep_chr_state(css_scan->state, *(css_scan->p_next)) == false)
		return NULL;
	cur_name = css_scan->p_curr_token;

	return cur_name;
}

/**
 * @brief	新建属性
 * @author xunwu
 * @date 2011/06/20
 **/
static easou_css_property_t *create_new_prop(easou_css_nodepool_t *pool, char *prop_name, easou_css_prop_type_t prop_type, char *prop_value)
{
	easou_css_property_t *new_prop = (easou_css_property_t *) css_get_from_nodepool(pool, sizeof(easou_css_property_t));
	if (new_prop == NULL)
	{
		Fatal((char*) "%s:%s:get from nodepool error!", __FILE__, __FUNCTION__);
		return NULL;
	}
	new_prop->name = prop_name;
	new_prop->type = prop_type;
	new_prop->value = prop_value;
	return new_prop;
}

/**
 * @brief	创建属性列表
 * @author xunwu
 * @date 2011/06/20
 **/
static easou_css_property_t *make_property_list(easou_css_scan_t *css_scan, easou_css_t *css)
{
	easou_css_property_t *css_prop_list = NULL;
	easou_css_str_heap_t *str_heap = &(css->css_inner.str_heap);
	easou_css_nodepool_t *pool = &(css->css_inner.nodepool);
	while (1)
	{
		char *cur_name = NULL;
		char *cur_value = NULL;
		/* scan prop name */
		cur_name = get_prop_name(css_scan, str_heap, css);
		if (cur_name == NULL)
			break;
		easou_trans2lower(cur_name, cur_name);
		easou_css_prop_type_t prop_type = css_seek_prop_type(cur_name);
		scan_ignore(css_scan, css);
		if (*(css_scan->p_next) != ':' || !is_useful_css_prop(prop_type))
		{
			css_scan->p_next = skip_current_prop(css_scan->p_next);
		}
		else
		{
			/* scan prop value */
			css_scan->p_next++;
			cur_value = get_prop_value(css_scan, str_heap, css);
			if (cur_value == NULL)
				break;
			easou_trans2lower(cur_value, cur_value);
			/* create a new property */
			if (cur_name != NULL && cur_name[0] != '\0' && cur_value != NULL && cur_value[0] != '\0')
			{
				easou_css_property_t *new_prop = create_new_prop(pool, cur_name, prop_type, cur_value);
				if (new_prop == NULL)
					break;
				new_prop->next = css_prop_list;
				css_prop_list = new_prop;
			}
		}

		if (*(css_scan->p_next) == ';')
		{
			css_scan->p_next++;
			scan_ignore(css_scan, css);
		}
		if (*(css_scan->p_next) == '}')
		{
			break;
		}
	}
	return css_prop_list;
}

/**
 * @brief	打印选择子
 * @author xunwu
 * @date 2011/06/20
 **/
static void print_selector(easou_css_selector_t *selector, FILE *fout, int *combine_num, easou_css_ruleset_t *ruleset)
{
	*combine_num = *combine_num + 1;
	static const char *attr_type_str[] =
	{ "attr_select_match_novalue", "attr_select_match_exactly", "attr_select_match_any", "attr_select_match_begin", "attr_select_other" };

	if (selector->pre_selector != NULL)
	{
		print_selector(selector->pre_selector, fout, combine_num, ruleset);
	}
	if (selector->pre_selector == NULL)
		fprintf(fout, "id:%d ", ruleset->id);
	switch (selector->combinator)
	{
	case CSS_NON_COMBINATOR:
		fprintf(fout, "[combinator:NON]");
		break;
	case CSS_DESCEND_COMBINATOR:
		fprintf(fout, "[combinator:DESCEND]");
		counter_add("combine_descend", 1);
		break;
	case CSS_CHILD_COMBINATOR:
		fprintf(fout, "[combinator:CHLD]");
		counter_add("combine_child", 1);
		break;
	case CSS_ADJACENT_COMBINATOR:
		fprintf(fout, "[combinator:ADJACENT]");
		counter_add("combine_adjacent", 1);
		break;
	}

	int type = 0;

	html_tag_type_t tt = selector->simple_selector.tag_type;
	if (tt >= 0 && tt < 104)
	{
		fprintf(fout, "(tag_type:%s)", get_tag_name(tt));

		char buf[256];
		int ret = snprintf(buf, 256, "tagtype_%s", get_tag_name(tt));
		buf[ret] = 0;
		counter_add(buf, 1);
		type |= 1;
	}
	else if (tt == -1)
	{
		fprintf(fout, "(tag_type:UNIV)");
		counter_add("tagtype_univ", 1);
	}
	else
	{
		fprintf(fout, "(tag_type:%d)", tt);
		counter_add("tagtype_unknow", 1);
		type |= 1;
	}

	if (selector->simple_selector.class_selector)
	{
		fprintf(fout, "(class:%s)", selector->simple_selector.class_selector);
		type |= 2;
	}
	if (selector->simple_selector.id_selector)
	{
		fprintf(fout, "(id:%s)", selector->simple_selector.id_selector);
		type |= 4;
	}
	if (selector->simple_selector.pseudo_selector)
	{
		fprintf(fout, "(pseudo selector:%s)", selector->simple_selector.pseudo_selector);
		type |= 8;
	}

	if (selector->simple_selector.attr_selector)
	{
		counter_add("simple_selector_attr", 1);
		type |= 16;
	}
	if (type == 1)
	{
		counter_add("select_type_tagtype", 1);
	}
	if (type == 2)
	{
		counter_add("select_type_class", 1);
		if (&ruleset->selector->simple_selector == &selector->simple_selector)
		{
			counter_add("select_type_class[leaf]", 1);
		}
	}
	else if (type == 4)
	{
		counter_add("select_type_id", 1);
	}
	else if (type == 3)
	{
		counter_add("select_type_class&tag", 1);
	}
	else if (type == 5)
	{
		counter_add("select_type_id&tag", 1);
	}
	else if (type == 6)
	{
		counter_add("select_type_id&class", 1);
	}
	else if (type == 7)
	{
		counter_add("select_type_tag&id&class", 1);
	}

	for (easou_css_attr_selector_t *attr = selector->simple_selector.attr_selector; attr; attr = attr->next)
	{
		fprintf(fout, "(name:%s|value:%s|type:%s)", attr->attr.name, attr->attr.value, attr_type_str[attr->type]);
	}
}

/**
 * @brief	打印属性列表
 * @author xunwu
 * @date 2011/06/20
 **/
static void print_prop_list(easou_css_property_t *prop_list, FILE *fout)
{
	easou_css_property_t *prop = prop_list;
	fprintf(fout, "\n{");
	while (prop != NULL)
	{
		if (prop->type == CSS_PROP_UNKNOWN)
		{
			fprintf(fout, "(type:PROP_UNKNOWN)=%s;", prop->value);
		}
		else
		{
			fprintf(fout, "(type:%s)=%s;", css_property_name_array[prop->type], prop->value);
		}
		prop = prop->next;
	}
	fprintf(fout, "}\n");
}

/**
 * @brief	以选择子为主键创建规则列表
 * @author xunwu
 * @date 2011/06/20
 **/
static easou_css_ruleset_t *make_ruleset_list_only_with_selector(easou_css_scan_t *css_scan, easou_css_t *css)
{
	easou_css_ruleset_t *ruleset = NULL;
	easou_css_ruleset_t *ruleset_list = NULL;
	easou_css_nodepool_t *pool = &(css->css_inner.nodepool);
	css_scan->state = CSS_STAT_SCAN_SELECTOR;
	while (1)
	{
		easou_css_selector_t *selector = make_selector(css_scan, css);
		if (selector == NULL)
			goto NEXT;

		/* make a new ruleset and hang on the list */
		ruleset = (easou_css_ruleset_t *) css_get_from_nodepool(pool, sizeof(easou_css_ruleset_t));
		if (ruleset == NULL)
		{
			Fatal((char*) "%s:%s:get from nodepool error!", __FILE__, __FUNCTION__);
			break;
		}
		ruleset->selector = selector;
		ruleset->all_property_list = NULL;
		ruleset->font_prop_begin = NULL;
		ruleset->font_prop_end = NULL;
		ruleset->geo_prop_begin = NULL;
		ruleset->geo_prop_end = NULL;
		ruleset->other_prop_begin = NULL;
		ruleset->other_prop_end = NULL;
		ruleset->next = ruleset_list;
		ruleset_list = ruleset;

		NEXT:
		if (*(css_scan->p_next) == '\0')
		{
			break;
		}
		else if (*(css_scan->p_next) != ',')
		{
			break;
		}
		css_scan->p_next++;
	}
	return ruleset_list;
}

/**
 * @brief	打印CSS.
 * @author xunwu
 * @date 2011/06/20
 **/
void print_css(easou_css_t *css, FILE *fout)
{
	for (easou_css_ruleset_t *ruleset = css->all_ruleset_list; ruleset != NULL; ruleset = ruleset->next)
	{
		int combine_num = 0;
		print_selector(ruleset->selector, fout, &combine_num, ruleset);
		print_prop_list(ruleset->all_property_list, fout);

		char info[256];
		int ret = snprintf(info, 256, "selector_%d_combine", combine_num);
		info[ret] = 0;
		counter_add(info, 1);
		fprintf(fout, "\n");
	}

	hashmap_iter_begin(css->index.class_map);
	void* data;
	while((data = hashmap_iter_next(css->index.class_map)) != NULL)
	{
		easou_css_index_node_t* index_node = (easou_css_index_node_t*)data;
		counter_add("select_type_class[hash_check]", 1);
		while(index_node->next != NULL)
		{
			index_node = index_node->next;
			counter_add("select_type_class[hash_check]", 1);
		}
	}
}

/**
 * @brief	判断是否是font属性
 * @author xunwu
 * @date 2011/06/20
 **/
// FIXME:这个可以改成inline
bool is_font_property(easou_css_property_t *prop)
{
	if (prop->type != CSS_PROP_UNKNOWN)
		return prop_typeinfo_array[prop->type].is_font_prop;
	else
		return false;
}

/**
 * @brief	判断是否是geo属性
 * @author xunwu
 * @date 2011/06/20
 **/
// FIXME:这个可以改成inline
bool is_geo_property(easou_css_property_t *prop)
{
	if (prop->type != CSS_PROP_UNKNOWN)
		return prop_typeinfo_array[prop->type].is_geo_prop;
	else
		return false;
}

/**
 * @brief	规则中添加属性
 * @param [in/out] ruleset   : css_ruleset_t*
 * @param [in/out] prop_list   : css_property_t*
 * @author xunwu
 * @date 2011/06/20
 **/
static void ruleset_add_props(easou_css_ruleset_t *ruleset, easou_css_property_t *prop_list)
{
	easou_css_property_t *next_prop = NULL;
	/**每一个属性都添加到队列头部*/
	for (easou_css_property_t *prop = prop_list; prop != NULL; prop = next_prop)
	{
		next_prop = prop->next;
		if (is_font_property(prop))
		{
			prop->next = ruleset->font_prop_begin;
			ruleset->font_prop_begin = prop;
			if (ruleset->font_prop_end == NULL)
			{
				ruleset->font_prop_end = prop;
			}
		}
		else if (is_geo_property(prop))
		{
			prop->next = ruleset->geo_prop_begin;
			ruleset->geo_prop_begin = prop;
			if (ruleset->geo_prop_end == NULL)
			{
				ruleset->geo_prop_end = prop;
			}
		}
		else
		{
			prop->next = ruleset->other_prop_begin;
			ruleset->other_prop_begin = prop;
			if (ruleset->other_prop_end == NULL)
			{
				ruleset->other_prop_end = prop;
			}
		}
	}
	/* 可以考虑将这部分操作移到循环里面 */
	easou_css_property_t *cur_end = NULL;
	if (ruleset->font_prop_begin != NULL)
	{
		ruleset->all_property_list = ruleset->font_prop_begin;
		cur_end = ruleset->font_prop_end;
	}
	if (ruleset->geo_prop_begin != NULL)
	{
		if (cur_end == NULL)
		{
			ruleset->all_property_list = ruleset->geo_prop_begin;
		}
		else
		{
			cur_end->next = ruleset->geo_prop_begin;
		}
		cur_end = ruleset->geo_prop_end;
	}
	if (ruleset->other_prop_begin != NULL)
	{
		if (cur_end == NULL)
		{
			ruleset->all_property_list = ruleset->other_prop_begin;
		}
		else
		{
			cur_end->next = ruleset->other_prop_begin;
		}
	}
}

/**
 * @brief	拷贝属性
 * @author xunwu
 * @date 2011/06/20
 **/
static void ruleset_prop_copy(easou_css_ruleset_t *des_ruleset, easou_css_ruleset_t *src_ruleset)
{
	des_ruleset->all_property_list = src_ruleset->all_property_list;
	des_ruleset->font_prop_begin = src_ruleset->font_prop_begin;
	des_ruleset->font_prop_end = src_ruleset->font_prop_end;
	des_ruleset->geo_prop_begin = src_ruleset->geo_prop_begin;
	des_ruleset->geo_prop_end = src_ruleset->geo_prop_end;
	des_ruleset->other_prop_begin = src_ruleset->other_prop_begin;
	des_ruleset->other_prop_end = src_ruleset->other_prop_end;
}

static inline int get_selector_string(easou_css_simple_selector_t* s, char* buf, int size)
{
	int ret;
	if (s->type == SIMPLE_SELECTOR_CLASS)
		ret = snprintf(buf, size, ".%s", s->class_selector);
	else if (s->type == SIMPLE_SELECTOR_TAG)
		ret = snprintf(buf, size, "%s", s->name);
	else if (s->type == SIMPLE_SELECTOR_ID)
		ret = snprintf(buf, size, "#%s", s->id_selector);
	else if (s->type == SIMPLE_SELECTOR_TAGCLASS)
		ret = snprintf(buf, size, "%s.%s", s->name, s->class_selector);
	else if (s->type == SIMPLE_SELECTOR_TAGID)
		ret = snprintf(buf, size, "%s#%s", s->name, s->id_selector);
	else if (s->type == SIMPLE_SELECTOR_ALL)
		ret = snprintf(buf, size, " ");
	else
		ret = snprintf(buf, size, "%s_%s_%s", s->tag_type == -1 ? "uni" : s->name, s->class_selector == NULL ? "null" : s->class_selector, s->id_selector == NULL ? "null" : s->id_selector);
	if (ret <= 0)
	{
		buf[0] = 0;
		return -1;
	}
	buf[ret] = 0;
	return ret;
}

bool css_create_hash_index(easou_css_t *css, hashmap_t* hm)
{
	char selector_str[1024];
	easou_css_nodepool_t *npool = &(css->css_inner.nodepool);
	for (easou_css_ruleset_t *rs = css->all_ruleset_list; rs != NULL; rs = rs->next)
	{
		easou_css_selector_t *selector = rs->selector;
		short start = 1;
		for (; selector; selector = selector->pre_selector, start--)
			;

		short pos = 0;
		int len = 0;
		selector = rs->selector;
		for (; selector; selector = selector->pre_selector, pos--)
		{
			if (selector->simple_selector.type == SIMPLE_SELECTOR_PSEUDO)
				break;
			len = get_selector_string(&selector->simple_selector, selector_str, 256);
			if (len <= 0)
				break;

			simple_selector_index_node_t *simple = (simple_selector_index_node_t*) css_get_from_nodepool(npool, sizeof(simple_selector_index_node_t));
			simple->pos = pos;
			simple->start_pos = start;
			simple->selector_id = rs->id;
			simple->ruleset = rs;

			void *value = hashmap_get(hm, selector_str);
			if (value == NULL)
			{
				simple->next = NULL;
				char* hash_key = (char*) css_get_from_nodepool(npool, len + 1);
				memcpy(hash_key, selector_str, len);
				hash_key[len] = 0;
				hashmap_put(hm, hash_key, simple);
			}
			else
			{
				simple->next = (simple_selector_index_node_t*) value;
				hashmap_mod(hm, selector_str, simple);
			}
		}
	}
	return true;
}

int css_parse_no_index(easou_css_t *css, const char *css_text, const char *css_url, bool is_continue, bool test_import)
{
	if (!is_continue)
	{
		if (adjust_str_heap(css, strlen(css_text)) < 0)
		{
			return -1;
		}
	}
	easou_css_scan_t css_scan;
	css_scan.p_next = css_text;
	css_scan.p_curr_token = NULL;
	css_scan.p_curr_token_len = 0;
	css_scan.type = CSS_TOKEN_UNKNOWN;
	css_scan.css_url = css_url;
	css_scan.test_import = test_import;
	if (!is_continue)
	{
		css_clean(css);
	}

	while (1)
	{
		/**创建规则集，但是只有选择子，可能会有多个选择子*/
		easou_css_ruleset_t *ruleset_list = make_ruleset_list_only_with_selector(&css_scan, css);
		if (*(css_scan.p_next) == '\0')
		{
			break;
		}
		if (*(css_scan.p_next) == '{')
		{
			css_scan.p_next++;
		}
		else
		{
			css_scan.p_next = skip_block(css_scan.p_next);
		}
		/**创建属性集*/
		easou_css_property_t *prop_list = make_property_list(&css_scan, css);
		/**属性集和规则集都有效，则可以将属性集移植到规则集上面*/
		if (prop_list != NULL && ruleset_list != NULL)
		{
			easou_css_ruleset_t *ruleset_list_tail = ruleset_list;
			easou_css_ruleset_t *first_ruleset = ruleset_list;
			ruleset_add_props(first_ruleset, prop_list);
			/**将这一段css代码中不同片段（多个选择子）也串成一个链表，这次是添加到队列尾部*/
			for (easou_css_ruleset_t *ruleset = first_ruleset->next; ruleset != NULL; ruleset = ruleset->next)
			{
				ruleset_prop_copy(ruleset, first_ruleset);
				ruleset_list_tail = ruleset;
			}
			/**添加到队列头部，操作简便是一方面，查找时正适合临近原则*/
			ruleset_list_tail->next = css->all_ruleset_list;
			css->all_ruleset_list = ruleset_list;
		}
		scan_ignore(&css_scan, css);

		if (*(css_scan.p_next) == '\0')
		{
			break;
		}
		if (*(css_scan.p_next) == '}')
		{
			css_scan.p_next++;
		}
		else
		{ /**无选择子的代码，忽略掉*/
			css_scan.p_next = skip_block(css_scan.p_next);
		}
	}

	if (css->all_ruleset_list != NULL)
	{
		return 1;
	}
	else
	{
		return -1; // parse fail
	}
}

int css_parse(easou_css_t *css, const char *css_text, const char *css_url, bool is_continue, bool test_import)
{
	//去掉UTF－8文件的BOM头
	if (strncmp(css_text, "\357\273\277", 3) == 0)
	{
		css_text += 3;
	}
	if (css_parse_no_index(css, css_text, css_url, is_continue, test_import) < 0)
	{
		return -1;
	}
	if(is_continue)
	{
		return 1;
	}
	return 1;
}
