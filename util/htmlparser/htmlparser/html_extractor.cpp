/**
 * easou_html_extractor.cpp
 *
 *  Created on: 2012-1-9
 *      Author: xunwu
 **/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "easou_url.h"
#include "html_text_utils.h"
#include "easou_html_attr.h"
#include "easou_html_extractor.h"

using namespace EA_COMMON;

// come from easou_vhtml_parser.cpp
static const html_tag_type_t html_blockTagList[] =
{ TAG_BLOCKQUOTE, TAG_CENTER, TAG_DD, TAG_DIV, TAG_DL, TAG_DT, TAG_FORM, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_HR, TAG_IFRAME,
		TAG_LI, TAG_OL, TAG_P, TAG_PRE, TAG_TABLE, TAG_TR, TAG_UL,
		// added
		TAG_HTML, TAG_BODY, TAG_MARQUEE, TAG_TBODY, TAG_THEAD, TAG_TFOOT };

/*
 * 是否是块标签；0：块标签；其他：非块标签
 */
static int isBlockTagInHtml(html_tag_type_t tagtype)
{
	int size = sizeof(html_blockTagList) / sizeof(html_tag_type_t);
	for (int i = 0; i < size; i++)
	{
		if (html_blockTagList[i] == tagtype)
		{
			return 0;
		}
	}
	return 1;
}

/***************************************TAGTITLE EXTRACTOR*******************************************************/
typedef struct _title_t
{
	char *content;
	int available;
	int end;
	unsigned begin :1;
	unsigned wml_compatible :1;
} title_t;

static int start_visit_title(html_tag_t *tag, void *result, int flag)
{
	title_t *tit = NULL;
	char *text = NULL;
	tit = (title_t*) result;
	if (tag->tag_type == TAG_TITLE)
	{
		tit->begin = 1;
		return VISIT_NORMAL;
	}
	if (tit->begin && tag->tag_type == TAG_PURETEXT)
	{
		text = tag->text;
	}
	else if (tag->tag_type == TAG_WAP_CARD)
	{
		text = get_attribute_value(tag, ATTR_TITLE);
	}
	if (text)
	{
		tit->available = copy_html_text(tit->content, tit->available, tit->end, text);
	}
	return VISIT_NORMAL;
}

static int finish_visit_title(html_tag_t *tag, void *result)
{
	if (tag->tag_type == TAG_TITLE)
	{
		return VISIT_FINISH;
	}
	if (((title_t*) result)->wml_compatible && tag->tag_type == TAG_WAP_CARD && ((title_t*) result)->available > 1)
	{
		return VISIT_FINISH;
	}
	return VISIT_NORMAL;
}

/**
 * @brief 提取标题,支持wml
 * 返回实际提取的字符串长度
 **/
int html_tree_extract_title(html_tree_t *html_tree, char* title, int size)
{
	title_t tit;

	assert(title);
	assert(size > 0);

	title[0] = '\0';
	memset(&tit, 0, sizeof(tit));
	tit.begin = 0;
	tit.content = title;
	tit.available = 0;
	tit.end = size - 1;
	tit.wml_compatible = 1;
	html_tree_visit(html_tree, start_visit_title, finish_visit_title, &tit, 0);
	return tit.available;
}

/***************************************LINK EXTRACTOR*******************************************************/
typedef struct _link_data_t
{
	char base_domain[MAX_SITE_LEN];
	char base_port[MAX_PORT_LEN];
	char base_path[MAX_PATH_LEN];
	link_t *links;
	int available;
	int end;
	char *anchor;
	int anchor_available;
	int anchor_end;
	char in_anchor;
	char in_option;
	char need_space;
} link_data_t;

static char* get_meta_url(html_tag_t *tag)
{
	char *p = NULL;
	char *q = NULL;

	p = get_attribute_value(tag, ATTR_HTTPEQUIV);
	if (p == NULL || strcasecmp(p, "refresh") != 0)
	{
		return NULL;
	}
	p = get_attribute_value(tag, ATTR_CONTENT);
	if (p == NULL)
	{
		return NULL;
	}
	q = strchr(p, '=');
	if (q != NULL && q - 3 >= p && strncasecmp(q - 3, "url", 3) == 0)
	{
		return q + 1;
	}
	return NULL;
}

typedef struct protocol_t
{
	const char *name;
	int len;
};

static const struct protocol_t protocol_head[] =
{
{ "http://", 7 } };

static int url_has_protocol_head(const char *url)
{
	unsigned int i = 0;
	for (i = 0; i < sizeof(protocol_head) / sizeof(protocol_t); i++)
	{
		if (strncasecmp(url, protocol_head[i].name, protocol_head[i].len) == 0)
		{
			return 1;
		}
	}
	return 0;
}

static int url_recheck(char *url)
{
	// fix bug
	if (url_has_protocol_head(url))
	{
		return 0;
	}
	// check syntax of url, lower upper char in site name
	if (easou_check_url(url) == 0)
	{
		return 0;
	}
	return 1;
}
/*
 * unparse url
 */
static void combine_url_inner(char *url, char *domain, char *port, char *path)
{
	if (*port == '\0')
	{
		if (domain != NULL)
			snprintf(url, MAX_URL_LEN, "%s%s", domain, path);
		else
			snprintf(url, MAX_URL_LEN, "%s", path);
	}
	else
	{
		snprintf(url, MAX_URL_LEN, "%s:%s%s", domain, port, path);
	}
	//todo
//	html_derefrence_text(url, 0, MAX_URL_LEN-1, url);
}

/*
 * is network path
 */
static int is_net_path(const char *path)
{
	if (strncmp(path, "//", 2) == 0)
	{
		return 1;
	}
	return 0;
}
/*
 * is absolute path
 */
static int is_abs_path(const char *path)
{
	if (*path == '/')
	{
		return 1;
	}
	return 0;
}

/*
 * is relative path
 */
static int is_rel_path(const char *url)
{
	const char *p;
	if (easou_is_url(url))
		return 0;
	if (is_net_path(url))
		return 0;
	if (is_abs_path(url))
		return 0;
	p = strchr(url, ':');
	if (p != NULL && p - url <= 10)
		//10 is the length of the longest shemas javascript
		return 0;
	return 1;
}

int html_combine_url(char *result_url, const char *src, char *base_domain, char *base_path, char *base_port)
{
	//param check
	if (result_url == NULL || src == NULL || base_domain == NULL || base_path == NULL || base_port == NULL)
	{
		return -1;
	}

	char domain[MAX_SITE_LEN];
	char port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN];
	char relpath[MAX_PATH_LEN];
	char *p = NULL;
	int res = 1;

	assert(src != NULL);

	if (easou_is_url(src))
	{
		if (strlen(src) >= MAX_URL_LEN || !easou_parse_url(src, domain, port, path) || !easou_single_path(path))
		{
			return -1;
		}
		easou_normalize_path(path);
		combine_url_inner(result_url, domain, port, path);
	}
	else if (is_net_path(src))
	{
		if (strlen(src) >= MAX_PATH_LEN)
			return -1;
		snprintf(path, sizeof(path), "%s", src);
		if (!easou_single_path(path))
			return -1;
		easou_normalize_path(path);
		char *p = path;
		while (p && *p == '/')
			p++;
		if (p && *p != '\0' && p != path)
			memmove(path, p, strlen(p) + 1);
		port[0] = '\0';
		combine_url_inner(result_url, NULL, port, path);
	}
	else if (is_abs_path(src))
	{
		if (strlen(src) >= MAX_PATH_LEN)
			return -1;
		snprintf(path, sizeof(path), "%s", src);
		if (!easou_single_path(path))
			return -1;
		easou_normalize_path(path);
		combine_url_inner(result_url, base_domain, base_port, path);
	}
	else if (is_rel_path(src))
	{
		if (strlen(base_path) + strlen(src) >= MAX_PATH_LEN)
		{
			return -1;
		}

		if (*src != '?' && *src != ';' && *src != '#')
		{
			snprintf(relpath, sizeof(relpath), "%s", base_path);
			remove_path_file_name(relpath);
			snprintf(path, MAX_PATH_LEN, "%s%s", relpath, src);
		}
		else
		{
			snprintf(relpath, sizeof(relpath), "%s", base_path);
			char ch = src[0];
			if ((p = strchr(relpath, ch)) != NULL)
				*p = 0;
			if (src[0] != '#')
				snprintf(path, MAX_PATH_LEN, "%s%s", relpath, src);
			else if (strlen(src) > 1)
				snprintf(path, MAX_PATH_LEN, "%s", relpath);
			else
			{
				snprintf(path, MAX_PATH_LEN, "%s", relpath);
				res = 2;
			}
		}
		if (!easou_single_path(path))
		{
			return -1;
		}
		easou_normalize_path(path);
		combine_url_inner(result_url, base_domain, base_port, path);
	}
	else
	{
		return -1;
	}
	if (url_recheck(result_url) == false)
	{
		return -1;
	}

	return res;
}

/*
 * extract link from TAG_(A, AREA, IMG, LINK, FRAME, IFRMAE, EMBED)
 * use base_url in TAG_BASE to replace base_url for normalizing, if exist
 * and extract text from child tags of TAG_A as its anchor text
 */
static int start_visit_link(html_tag_t *tag, void *result, int flag)
{
	link_data_t *link = NULL;
	int ret = -1;
	char *base_url = NULL;
	char *src = NULL;

	link = (link_data_t *) result;

	if (link->available == link->end)
	{
		return VISIT_FINISH;
	}
	assert(link->available < link->end);

	//base url
	if (tag->tag_type == TAG_BASE)
	{
		char domain[MAX_SITE_LEN] = { '\0' };
		char port[MAX_PORT_LEN] = { '\0' };
		char path[MAX_PATH_LEN] = { '\0' };
		base_url = get_attribute_value(tag, ATTR_HREF);
		if (base_url == NULL || !easou_is_url(base_url) || strlen(base_url) >= MAX_URL_LEN || !easou_parse_url(base_url, domain, port, path)
				|| !easou_single_path(path))
		{
			return VISIT_NORMAL;
		}
		ret = easou_parse_url(base_url, link->base_domain, link->base_port, link->base_path);
		assert(ret == 1);
		ret = easou_single_path(link->base_path);
		assert(ret == 1);
		easou_normalize_path(link->base_path);
		remove_path_file_name(link->base_path);
		return VISIT_NORMAL;
	}

	//anchor text
	if (tag->tag_type == TAG_PURETEXT && (link->in_anchor == 1 || link->in_option == 1))
	{
		link->anchor_available = copy_html_text(link->anchor, link->anchor_available, link->anchor_end, tag->text);
		return VISIT_NORMAL;
	}

	//normal link
	if (tag->tag_type == TAG_IMG || tag->tag_type == TAG_EMBED || tag->tag_type == TAG_FRAME || tag->tag_type == TAG_IFRAME)
	{
		src = get_attribute_value(tag, ATTR_SRC);
	}
	else if (tag->tag_type == TAG_LINK || tag->tag_type == TAG_AREA || tag->tag_type == TAG_A)
	{
		src = get_attribute_value(tag, ATTR_HREF);
	}
	else if (tag->tag_type == TAG_META)
	{
		src = get_meta_url(tag);
	}
	else if (tag->tag_type == TAG_OPTION)
	{
		src = get_attribute_value(tag, ATTR_VALUE);
		if (src != NULL && strncasecmp(src, "http://", 7) != 0)
		{
			src = NULL;
		}
	}
	else
	{
		src = NULL;
	}

	//no any link
	if (src == NULL)
	{
		return VISIT_NORMAL;
	}

	//extract link
	if (html_combine_url(link->links[link->available].url, src, link->base_domain, link->base_path, link->base_port) < 0)
	{
		return VISIT_NORMAL;
	}
	link->links[link->available].text[0] = '\0';

	//prepare anchor text
	if (tag->tag_type == TAG_A)
	{
		link->anchor = link->links[link->available].text;
		link->anchor_available = 0;
		link->anchor_end = UL_MAX_TEXT_LEN - 1;
		link->in_anchor = 1;
	}
	else if (tag->tag_type == TAG_OPTION)
	{
		link->anchor = link->links[link->available].text;
		link->anchor_available = 0;
		link->anchor_end = UL_MAX_TEXT_LEN - 1;
		link->in_option = 1;
	}
	link->available++;
	return VISIT_NORMAL;
}

/*
 * finish visit TAG_A or TAG_OPTION
 */
static int finish_visit_link(html_tag_t *tag, void *result)
{
	link_data_t *lnk = NULL;

	lnk = (link_data_t*) result;
	if (tag->tag_type == TAG_A)
	{
		lnk->in_anchor = 0;
	}
	else if (tag->tag_type == TAG_OPTION)
	{
		lnk->in_option = 0;
	}
	return VISIT_NORMAL;
}

int html_tree_extract_link(html_node_list_t* list, char* baseUrl, link_t* link, int& num)
{
	link_data_t lnk;
	char *p = NULL;

	assert(baseUrl != NULL);
	assert(link != NULL);
	assert(num > 0);

	if (strlen(baseUrl) >= MAX_URL_LEN)
	{
		return -1;
	}
	lnk.end = num;
	lnk.available = 0;
	lnk.links = link;
	lnk.need_space = 0;
	lnk.in_anchor = 0;
	lnk.in_option = 0;
	if (!easou_parse_url(baseUrl, lnk.base_domain, lnk.base_port, lnk.base_path))
	{
		return -1;
	}

	if ((p = strchr(lnk.base_path, '#')) != NULL)
		*p = 0;

	while (list)
	{
		if (list->html_node)
		{
			if (!html_node_visit(list->html_node, start_visit_link, finish_visit_link, &lnk, 0))
			{
				return -1;
			}
		}
		list = list->next;
	}

	assert(lnk.available <= lnk.end);
	num = lnk.available;
	return lnk.available;
}

/**
 * @brief 提取链接
 * */
int html_tree_extract_link(html_tree_t *html_tree, char* baseUrl, link_t* link, int& num)
{
	link_data_t lnk;
	char *p = NULL;

	assert(baseUrl != NULL);
	assert(link != NULL);
	assert(num > 0);

	if (strlen(baseUrl) >= MAX_URL_LEN)
	{
		return -1;
	}
	lnk.end = num;
	lnk.available = 0;
	lnk.links = link;
	lnk.need_space = 0;
	lnk.in_anchor = 0;
	lnk.in_option = 0;
	if (!easou_parse_url(baseUrl, lnk.base_domain, lnk.base_port, lnk.base_path))
	{
		return -1;
	}

	if ((p = strchr(lnk.base_path, '#')) != NULL)
		*p = 0;

	if (!html_tree_visit(html_tree, start_visit_link, finish_visit_link, &lnk, 0))
	{
		return -1;
	}
	assert(lnk.available <= lnk.end);
	num = lnk.available;
	return lnk.available;
}

/***************************************ABSTRACT EXTRACTOR*******************************************************/
/* extract content from meta */
static int trim_tag_from_text(char *dstbuf, int curpos, int bufsize, const char *srcbuf)
{
	const char * psrc = srcbuf;
	int i = 0;
	while (*psrc)
	{
		if (curpos == bufsize)
		{
			break;
		}
		if (*psrc == '<')
		{
			i = 1;
			while (*(psrc + i) != 0 && *(psrc + i) > 0 && *(psrc + i) != '>')
			{
				i++;
			}
			if (*(psrc + i) == '>')
			{
				psrc += i + 1;
				continue;
			}
			if (*(psrc + i) == '\0')
				break;
		}
		dstbuf[curpos++] = *psrc++;
	}
	dstbuf[curpos] = 0;
	return curpos;
}

typedef struct _abstract_t
{
	char *content;
	int available;
	int end;
} abstract_t;

static int start_visit_abstract(html_tag_t *tag, void *result, int flag)
{
	abstract_t * abst = NULL;
	char *text = NULL;

	abst = (abstract_t *) result;

	if (tag->tag_type == TAG_META)
	{
		text = get_attribute_value(tag, ATTR_NAME);
		if (!text)
		{
			text = get_attribute_value(tag, ATTR_HTTPEQUIV);
		}
		if (text && strncasecmp(text, "description", 11) == 0)
		{
			text = get_attribute_value(tag, ATTR_CONTENT);
			if (text)
			{
				copy_html_text(abst->content, abst->available, abst->end, text);
				abst->available = trim_tag_from_text(abst->content, abst->available, abst->end, abst->content + abst->available);
				return VISIT_FINISH;
			}
		}
	}
	return VISIT_NORMAL;
}

static int start_visit_keywords(html_tag_t *tag, void *result, int flag)
{
	abstract_t * abst = NULL;
	char *text = NULL;

	abst = (abstract_t *) result;

	if (tag->tag_type == TAG_META)
	{
		text = get_attribute_value(tag, ATTR_NAME);

		if (text && strstr(text, "keyword") != NULL)
		{
			text = get_attribute_value(tag, ATTR_CONTENT);
			if (text)
			{
				copy_html_text(abst->content, abst->available, abst->end, text);
				abst->available = trim_tag_from_text(abst->content, abst->available, abst->end, abst->content + abst->available);
				return VISIT_FINISH;
			}
		}
	}
	return VISIT_NORMAL;
}

/**
 * 返回实际提取字符串长度
 */
int html_tree_extract_abstract(html_tree_t *tree, char *abstract, int size, int merge)
{
	abstract_t cnt;

	assert(abstract);
	assert(size > 0);

	abstract[0] = '\0';
	memset(&cnt, 0, sizeof(cnt));
	cnt.content = abstract;
	cnt.available = 0;
	cnt.end = size - 1;
	html_tree_visit(tree, start_visit_abstract, NULL, &cnt, merge);
	return cnt.available;
}

/**
 * 返回实际提取字符串长度
 */
int html_tree_extract_keywords(html_tree_t *tree, char *keywords, int size, int merge)
{
	abstract_t cnt;

	assert(keywords);
	assert(size > 0);

	keywords[0] = '\0';
	memset(&cnt, 0, sizeof(cnt));
	cnt.content = keywords;
	cnt.available = 0;
	cnt.end = size - 1;
	html_tree_visit(tree, start_visit_keywords, NULL, &cnt, merge);
	return cnt.available;
}

static int finish_visit_csslink(html_tag_t *tag, void *result)
{
//	if (tag && tag->tag_type == TAG_HEAD)
//	{
//		return VISIT_FINISH;
//	}
	return VISIT_NORMAL;
}

static int start_visit_csslink(html_tag_t *tag, void *result, int flag)
{
	link_data_t *link = NULL;
	int ret = -1;
	char *base_url = NULL;
	char *src = NULL;

	link = (link_data_t *) result;

	if (link->available == link->end)
	{
		return VISIT_FINISH;
	}
	assert(link->available < link->end);

	//base url
	if (tag->tag_type == TAG_BASE)
	{
		char domain[MAX_SITE_LEN] = { '\0' };
		char port[MAX_PORT_LEN] = { '\0' };
		char path[MAX_PATH_LEN] = { '\0' };
		base_url = get_attribute_value(tag, ATTR_HREF);
		if (base_url == NULL || !easou_is_url(base_url) || strlen(base_url) >= MAX_URL_LEN || !easou_parse_url(base_url, domain, port, path)
				|| !easou_single_path(path))
		{
			return VISIT_NORMAL;
		}
		ret = easou_parse_url(base_url, link->base_domain, link->base_port, link->base_path);
		assert(ret == 1);
		ret = easou_single_path(link->base_path);
		assert(ret == 1);
		easou_normalize_path(link->base_path);
		remove_path_file_name(link->base_path);
		return VISIT_NORMAL;
	}

	//normal link
	if (tag->tag_type == TAG_LINK)
	{
		char *prel = NULL;
		char *ptype = NULL;
		prel = get_attribute_value(tag, "rel");
		ptype = get_attribute_value(tag, "type");
		if (prel && strcasecmp(prel, "stylesheet") == 0)
		{
			src = get_attribute_value(tag, ATTR_HREF);
		}
	}
    if(tag->tag_type==TAG_STYLE){
    	char styleinfo[UL_MAX_URL_LEN]={0};
    	char *prealinfo=styleinfo;
    	char *pstyle=tag->text;

    	if(pstyle){
    	//	printf("find style ,str=%s\n",pstyle);
    		while (pstyle&&easou_isspace(*pstyle))
    		{
    		    		pstyle++;
    		}
    		if(strncmp(pstyle,"@import",7)==0){
    			//printf("find style import,str=%s\n",pstyle);
    			pstyle=pstyle+7;
    			while (pstyle && (*pstyle == ' '||*pstyle == '	'))
    			{
    			    pstyle++;
    			}
    			if(strncmp(pstyle,"url(",4)==0){
    				pstyle=pstyle+4;
    			}
    			if (pstyle && (*pstyle == '"'||*pstyle == '\''))
    			{
    			    			    pstyle++;
    			}
    			int charnum=0;
    			while(pstyle&&((*pstyle!='\0')&&(*pstyle!='"')&&(*pstyle != '\''))&&charnum++<UL_MAX_URL_LEN-1){
    				*prealinfo++= *pstyle++;
    			}
    			if(strlen(styleinfo)>0){
    				if (html_combine_url(link->links[link->available].url, styleinfo, link->base_domain, link->base_path, link->base_port) < 0)
    				{
    					return VISIT_SKIP_CHILD;
    				}
    				 //prepare anchor text
    				link->available++;
    				return VISIT_SKIP_CHILD;
    			}

    		}
    	}


    }
	//no any link
	if (src == NULL)
	{
		return VISIT_NORMAL;
	}
	while (src && *src == ' ')
	{
		src++;
	}
	//extract link
	if (html_combine_url(link->links[link->available].url, src, link->base_domain, link->base_path, link->base_port) < 0)
	{
		return VISIT_NORMAL;
	}

	//prepare anchor text
	link->available++;
	return VISIT_NORMAL;
}
/**
 * @brief 提取链接
 * */
int html_tree_extract_csslink(html_tree_t *html_tree, const char* baseUrl, link_t* link, int& num)
{
	link_data_t lnk;
	char *p = NULL;

	assert(baseUrl != NULL);
	assert(link != NULL);
	assert(num > 0);

	if (strlen(baseUrl) >= MAX_URL_LEN)
	{
		return -1;
	}
	lnk.end = num;
	lnk.available = 0;
	lnk.links = link;
	lnk.need_space = 0;
	lnk.in_anchor = 0;
	lnk.in_option = 0;
	if (!easou_parse_url(baseUrl, lnk.base_domain, lnk.base_port, lnk.base_path))
	{
		return -1;
	}

	if ((p = strchr(lnk.base_path, '#')) != NULL)
		*p = 0;
	//remove_path_file_name(lnk.base_path);

	if (!html_tree_visit(html_tree, start_visit_csslink, NULL, &lnk, 0))
	{
		return -1;
	}
	assert(lnk.available <= lnk.end);
	return lnk.available;
}

static int start_visit_node_content(html_tag_t *html_tag, void *result, int flag)
{
	title_t *tit = NULL;
	char *src = NULL;
	tit = (title_t*) result;
	if (tit->available >= tit->end)
	{
		return VISIT_FINISH;
	}
	if (html_tag->tag_type == TAG_PURETEXT)
	{
		src = html_tag->text;
	}
	else if (html_tag->tag_type == TAG_TD)
	{
		if (tit->available > 0)
		{
			if (tit->content[tit->available - 1] == ' ' || tit->content[tit->available - 1] == '\n'
					|| tit->content[tit->available - 1] == '\t')
			{
			}
			else
			{
				if (tit->available < tit->end - 1)
				{
					tit->content[tit->available++] = ' ';
					tit->content[tit->available] = 0;
				}
			}
		}
	}
	else if (isBlockTagInHtml(html_tag->tag_type) == 0)
	{
		if (tit->available > 0)
		{
			if (tit->content[tit->available - 1] == '\n')
			{
			}
			else
			{
				if (tit->available < tit->end - 1)
				{
					tit->content[tit->available++] = '\n';
					tit->content[tit->available] = 0;
				}
			}
		}
	}
	else
	{
		src = NULL;
	}
	if (src != NULL)
	{
		tit->available = copy_html_text(tit->content, tit->available, tit->end, src);
	}
	return VISIT_NORMAL;
}
/**
 * @brief 提取某一节点及其下的内容
 **/
int html_node_extract_content(html_node_t *html_node, char* content, int size)
{
	title_t tit;

	assert(content);
	assert(size > 0);

	content[0] = '\0';
	content[size - 1] = '\0';
	memset(&tit, 0, sizeof(tit));
	tit.begin = 0;
	tit.content = content;
	tit.available = 0;
	tit.end = size - 1;
	html_node_visit(html_node, start_visit_node_content, NULL, &tit, 0);
	return tit.available;
}

static int start_visit_for_base_url(html_tag_t *html_tag, void *result, int flag)
{
	char domain[MAX_SITE_LEN];
	char port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN];
	if (html_tag->tag_type == TAG_BASE)
	{
		char *base_url = get_attribute_value(html_tag, ATTR_HREF);
		if (NULL == base_url || !easou_is_url(base_url) || strlen(base_url) >= UL_MAX_URL_LEN
				|| !easou_parse_url(base_url, domain, port, path) || !easou_single_path(path))
		{
			return VISIT_FINISH;
		}
		snprintf((char *) result, MAX_URL_LEN, "%s", base_url);
		return VISIT_FINISH;
	}
	return VISIT_NORMAL;
}

static int finish_visit_for_base_url(html_tag_t *html_tag, void *result)
{
	if (html_tag->tag_type == TAG_HEAD)
		return VISIT_FINISH;
	return VISIT_NORMAL;
}

//获取页面内base tag所指定的URL
int get_base_url(char *base_url, html_tree_t *html_tree)
{
	int ret = html_tree_visit(html_tree, &start_visit_for_base_url, &finish_visit_for_base_url, base_url, 0);
	return ret;
}
