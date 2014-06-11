/*
 * easou_html_doctype.cpp
 *
 * 识别逻辑：
 * 1. 如果有wml标签, 则是wap1.0
 *
 *  Created on: 2012-4-17
 *      Author: sandy
 */

#include "html_doctype.h"
#include "html_tree.h"
#include "html_dom.h"
#include "util/htmlparser/utils/log.h"

#include <string.h>

using namespace EA_COMMON;

/**
 * WAP1.0 特有标签
 */
static const html_tag_type_t wmltag[] =
{ TAG_WAP_WML, TAG_WAP_CARD, TAG_WAP_DO, TAG_WAP_GO, TAG_WAP_POSTFIELD };

/**
 * WEB特有标签
 */
static const html_tag_type_t html[] =
{ TAG_INS, TAG_LEGEND, TAG_MAP, TAG_NOSCRIPT, TAG_DEL, TAG_COLGROUP, TAG_BDO };

/**
 * html5特有标签
 static const char* html5[] = { "article", "aside", "details", "figcaption",
 "figure", "header", "hgroup", "section", "footer", "nav" };
 */

/**
 * 手机网页的url前缀
 */
static const char url_prefix[][20] =
{ "http://wap.", "http://m.", "http://3g." };
static const int url_prefix_num = sizeof(url_prefix) / sizeof(url_prefix[0]);

/**
 * DOCTYPE类型定义
 */
typedef enum _doc_types
{
	doctypes_unknown = 0, doctypes_wml = 1, doctypes_html = 2, doctypes_xhtml = 3, doctypes_xhtmlmobile = 4, doctypes_html5 = 5,
} doc_types;

/**
 * DOCTYPE关键字
 */
typedef struct _doctype_keyword
{
	char keyword[256];
	doc_types type;
} doctype_keyword;

/**
 * DOCTYPE关键字和类型对应数组
 */
static const doctype_keyword g_doctype_keyword[] =
{
{ "wml", doctypes_wml },
{ "HTML PUBLIC \"-//W3C//DTD HTML 4.01", doctypes_html },
{ "html PUBLIC \"-//W3C//DTD XHTML 1.", doctypes_xhtml },
{ "html PUBLIC \"-//W3C//DTD XHTML Basic", doctypes_xhtml },
{ "html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile", doctypes_xhtmlmobile },
{ "<!DOCTYPE html>", doctypes_html5 } };

static const int g_doctype_keyword_len = sizeof(g_doctype_keyword) / sizeof(doctype_keyword);

typedef struct _doctype_visit_data
{
	html_doctype doctype;
	doc_types tag_doctype;
	int wmlOrHtml;
	bool MobileOptimized;
} doctype_visit_data;

static void determine_visit_data(doctype_visit_data *visit_data, const char* url)
{
	// 1. 网页中含有wml标签的，认为是wap1.0网页
	if (visit_data->wmlOrHtml == 1)
	{
		visit_data->doctype = doctype_wml;
		return;
	}
	else if (visit_data->wmlOrHtml == 2)
	{
		visit_data->doctype = doctype_html4;
	}
	else
	{
		visit_data->doctype = doctype_unknown;
	}

	// 2. 根据DOCTYPE确定网页类型
	if (visit_data->tag_doctype == doctypes_wml)
	{
	}
	else if (visit_data->tag_doctype == doctypes_xhtmlmobile)
	{
		visit_data->doctype = doctype_xhtml_MP;
	}
	else if (visit_data->tag_doctype == doctypes_xhtml)
	{
		visit_data->doctype = doctype_xhtml;
	}
	else if (visit_data->tag_doctype == doctypes_html5)
	{
		visit_data->doctype = doctype_html5;
	}
	else if (visit_data->tag_doctype == doctypes_html)
	{
		visit_data->doctype = doctype_html4;
	}

	// 3. DOCTYPE为XHTML，且url含有WAP关键字的，认识是WAP2.0
	if (visit_data->doctype == doctype_xhtml && url)
	{
		for (int i = 0; i < url_prefix_num; i++)
		{
			if (strstr(url, url_prefix[i]))
			{
				visit_data->doctype = doctype_xhtml_MP;
			}
		}
	}

	if (visit_data->MobileOptimized)
	{
		visit_data->doctype = doctype_xhtml_MP;
	}

	// 4. 其它网页默认为html4网页
	if (visit_data->doctype == doctype_unknown)
	{
		visit_data->doctype = doctype_html4;
	}
}

static int start_visit_doctype(html_tag_t* html_tag, void* result, int flag)
{
	doctype_visit_data *tmp_data = (doctype_visit_data*) result;
	if (html_tag->tag_type == TAG_DOCTYPE)
	{
		if (html_tag->text)
		{
			for (int i = 0; i < g_doctype_keyword_len; i++)
			{
				if (strstr(html_tag->text, g_doctype_keyword[i].keyword))
				{
					tmp_data->tag_doctype = g_doctype_keyword[i].type;
				}
			}
		}
	}
	if (html_tag->tag_type == TAG_WAP_WML)
	{
		tmp_data->wmlOrHtml = 1;
	}
	if (html_tag->tag_type == TAG_HTML)
	{
		tmp_data->wmlOrHtml = 2;
	}
	if (html_tag->tag_type == TAG_META && html_tag->attribute)
	{
		html_attribute_t *attr = html_tag->attribute;
		while (attr)
		{
			if (attr->name && strcmp(attr->value, "MobileOptimized") == 0)
			{
				tmp_data->MobileOptimized = true;
			}
			attr = attr->next;
		}
	}
	return VISIT_NORMAL;
}

static int finish_visit_doctype(html_tag_t* html_tag, void* result)
{
	if (html_tag->tag_type == TAG_HEAD)
	{
		return VISIT_FINISH;
	}
	return VISIT_NORMAL;
}

int determine_doctype_from_tree(html_tree_t *html_tree, const char *url)
{

	doctype_visit_data tmp_data;
	tmp_data.wmlOrHtml = 0;
	tmp_data.tag_doctype = doctypes_unknown;
	tmp_data.MobileOptimized = false;

	int visit_ret = html_tree_visit(html_tree, start_visit_doctype, finish_visit_doctype, &tmp_data, 0);
	if (visit_ret == VISIT_ERROR)
	{
		return -1;
	}
	determine_visit_data(&tmp_data, url);
	html_tree->doctype = tmp_data.doctype;
	return 0;
}
