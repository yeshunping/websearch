/***************************************************************************
 *
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link.cpp,v 1.0 2012/09/01 pageparse Exp $
 *
 **************************************************************************/

#include "html_text_utils.h"
#include "easou_mark_parser.h"
#include "easou_mark_sem.h"
#include "easou_css_parser.h"
#include "easou_html_attr.h"
#include "easou_url.h"
#include "easou_string.h"
#include "PageType.h"

#include "easou_link_timematch.h"
#include "easou_link.h"
#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_link_area.h"
#include "easou_html_extractor.h"
#include "easou_mark_func.h"

#include "pcre.h"
#include <ctype.h>
#include <math.h>
#include <pthread.h>

#include<stack>
using namespace std;

#include<string.h>

#include "log.h"
using namespace EA_COMMON;

#define PAGE_NOFOLLOW 0x1
#define PAGE_NULL 0x0

typedef struct _vlink_data_t
{
	const char *base_url;
	char *base_domain;
	char *base_port;
	char *base_path;
	vlink_t *vlink;
	int available;
	int end;
	char *anchor;
	int anchor_available;
	int anchor_end;
	char in_anchor;
	char in_option;
	char need_space;
	char flag;
	char nofollow;
	int route;
} vlink_data_t;

static int is_rel_path(char *url);
static int is_abs_path(char *path);
static int is_net_path(char *path);
static int is_url(const char *url);
static char *get_meta_url(html_tag_t *html_tag);
static int isGoodLink(vlink_t *vlink, base_info_t *base_info);
static int isOutLink(vlink_t *vlink, const char* base_domain);
static void get_anchor_from_alt(vlink_data_t *vlink_data, int index, char flag);
static int extract_text_link_in_vnode(vlink_data_t *link_data, html_vnode_t *vnode);

/**
 * @brief clean vlink inner property
 **/
static void vlink_inner_clean(vlink_info_t *vinfo)
{
	vinfo->xpos = -1;
	vinfo->ypos = -1;
	vinfo->width = -1;
	vinfo->height = -1;
	vinfo->tag_type = TAG_UNKNOWN;
	vinfo->node = NULL;
	vinfo->vnode = NULL;
	vinfo->is_goodlink = 0;
	vinfo->is_outlink = 0;
	vinfo->link_set = -1;
	vinfo->html_area = NULL;
	vinfo->route = 0;
	vinfo->area_left_link_count = 0;
	vinfo->anchor_from_alt = 0;
	vinfo->text_len = 0;
}

/**
 * @brief clean vlink 
 **/
static void vlink_clean(vlink_t *vlink)
{
	vlink->url[0] = '\0';
	vlink->text[0] = '\0';
	vlink->position = PAGE_UNDEFINED;
	vlink->linkFunc = 0;
	vlink->nofollow = 0;
	vlink->group = -1;
	vlink->tag_code = -1;
	vlink_inner_clean(&vlink->inner);
}

static void vlink_assign_vnode_property(vlink_t *vlink, html_vnode_t *vnode)
{
	// the following will assign the vnode info to vlink
	vlink->inner.xpos = vnode->xpos;
	vlink->inner.ypos = vnode->ypos;
	vlink->inner.width = vnode->wx;
	vlink->inner.height = vnode->hx;
	vlink->inner.tag_type = vnode->hpNode->html_tag.tag_type;

	vlink->inner.node = vnode->hpNode;
	vlink->inner.vnode = vnode;
}

/**
 * @brief 对url合法性再次进行核查
 * @retval  合法返回true，否则false.
 **/
static bool vlink_url_recheck(const char *url)
{
	// fix bug
	if (is_url_has_protocol_head(url))
		return false;
	//check syntax of url, lower upper char in site name 
	if (easou_check_url((char *) url) == 0)
	{
		return false;
	}

	return true;
}

/**
 * @brief 对style标签内的@import链接进行提取.
 * @retval  -1:error;>=0:提取的链接数量.
 **/
static int extract_vlink_in_style(vlink_data_t *link_data, html_vnode_t *vnode)
{
	html_tag_t *html_tag = &vnode->hpNode->html_tag;
	assert(TAG_STYLE == html_tag->tag_type);

	vlink_t *vlink = link_data->vlink + link_data->available;
	int leftnum = link_data->end - link_data->available;
	int link_num = 0;
	if (html_tag->text)
	{
		/**
		 * 提取css链接但不标注，统一放在链接标注时进行.
		 */
		link_num = css_extract_vlink(html_tag->text, link_data->base_url, vlink, leftnum, false);
		if (-1 == link_num)
		{
			return -1;
		}

		// assign properties
		for (int i = 0; i < link_num; i++)
		{
			vlink_t *curr_vlink = link_data->vlink + link_data->available;
			vlink_assign_vnode_property(curr_vlink, vnode);
			curr_vlink->inner.route = link_data->route;
			link_data->available++;
		}
	}

	return link_num;
}

static int vlink_combine_url(char *result_url, char &nofollow, char *src, char *base_domain, char *base_path, char *base_port, const char *base_url)
{
	char domain[UL_MAX_SITE_LEN];
	char port[UL_MAX_PORT_LEN];
	char path[UL_MAX_PATH_LEN];
	char relpath[UL_MAX_PATH_LEN];
	char *p = NULL;

	assert(src != NULL);

	if (is_url(src))
	{
		if (strlen(src) >= UL_MAX_URL_LEN || !easou_parse_url_ex(src, domain, UL_MAX_SITE_LEN, port, UL_MAX_PORT_LEN, path, UL_MAX_PATH_LEN) || !easou_single_path_inner(path))
		{
			return -1;
		}
		easou_normalize_path(path);
		easou_combine_url_inner(result_url, domain, port, path);
	}
	else if (is_net_path(src))
	{
		if (strlen(src) >= UL_MAX_PATH_LEN)
			return -1;
		strcpy(path, src);
		if (!easou_single_path_inner(path))
			return -1;
		easou_normalize_path(path);
		char *p = path;
		while (p && *p == '/')
			p++;
		if (p && *p != '\0')
			strcpy(path, p);
		port[0] = '\0';
		easou_combine_url_inner(result_url, NULL, port, path);
	}
	else if (is_abs_path(src))
	{
		if (strlen(src) >= UL_MAX_PATH_LEN)
			return -1;
		strcpy(path, src);
		if (!easou_single_path_inner(path))
			return -1;
		easou_normalize_path(path);
		easou_combine_url_inner(result_url, base_domain, base_port, path);
	}
	else if (is_rel_path(src))
	{
		if (strlen(base_path) + strlen(src) >= UL_MAX_PATH_LEN)
		{
			return -1;
		}

		if (*src != '?' && *src != ';' && *src != '#')
		{
			strcpy(relpath, base_path);
			remove_path_file_name(relpath);
			snprintf(path, UL_MAX_PATH_LEN, "%s%s", relpath, src);
		}
		else
		{
			strcpy(relpath, base_path);
			char ch = src[0];
			if ((p = strchr(relpath, ch)) != NULL)
				*p = 0;
			if (src[0] != '#')
				snprintf(path, UL_MAX_PATH_LEN, "%s%s", relpath, src);
			else if (strlen(src) > 1)
				snprintf(path, UL_MAX_PATH_LEN, "%s", relpath);
			else
			{ // src is a single '#'
			  // return -1;
				snprintf(path, UL_MAX_PATH_LEN, "%s", relpath);
				nofollow = 1;
			}
		}
		if (!easou_single_path_inner(path))
		{
			return -1;
		}
		easou_normalize_path(path);
		easou_combine_url_inner(result_url, base_domain, base_port, path);
	}
	else
	{
		return -1;
	}

	if (vlink_url_recheck(result_url) == false)
	{
		return -1;
	}

	//add
	char tmp[UL_MAX_SITE_LEN] ={ 0 };
	if (url_protocol_head_len(src) == 8)
		strncpy(tmp, "https://", 8);
	else if (url_protocol_head_len(src) == 7)
		strncpy(tmp, "http://", 7);
	else
	{
		if (url_protocol_head_len(base_url) == 8)
			strncpy(tmp, "https://", 8);
		else if (url_protocol_head_len(base_url) == 7)
			strncpy(tmp, "http://", 7);
	}
	strcat(tmp, result_url);
	result_url[0] = '\0';
	strcpy(result_url, tmp);

	return 1;
}

/**
 * extract vlink from TAG_(A, AREA, IMG, LINK, FRAME, IFRMAE, EMBED)
 * use base_url in TAG_BASE to replace base_url for normalizing, if exist
 * and extract text from child tags of TAG_A as its anchor text
 */
static int start_visit_for_vlink(html_vnode_t *vnode, void *result)
{
	int ret;
	char domain[UL_MAX_SITE_LEN];
	char port[UL_MAX_PORT_LEN];
	char path[UL_MAX_PATH_LEN];
	char *base_url;
	char *src;
	char nofollow = 0;

	vlink_data_t *link_data = (vlink_data_t *) result;

	if (link_data->available == link_data->end)
	{
		return VISIT_FINISH;
	}
	assert(link_data->available < link_data->end);

	html_tag_t *html_tag = &(vnode->hpNode->html_tag);
	link_data->route += html_tag->tag_type;
	//base url
	if (html_tag->tag_type == TAG_BASE)
	{
		base_url = get_attribute_value(html_tag, "href");
		if (base_url == NULL || !is_url(base_url) || strlen(base_url) >= UL_MAX_URL_LEN || !easou_parse_url_ex(base_url, domain, UL_MAX_SITE_LEN, port, UL_MAX_PORT_LEN, path, UL_MAX_PATH_LEN) || !easou_single_path(path))
		{
			return VISIT_NORMAL;
		}
		link_data->base_url = base_url;
		ret = easou_parse_url_ex(base_url, link_data->base_domain, UL_MAX_SITE_LEN, link_data->base_port, UL_MAX_PORT_LEN, link_data->base_path, UL_MAX_PATH_LEN);
		assert(ret == 1);
		ret = easou_single_path(link_data->base_path);
		assert(ret == 1);
		easou_normalize_path(link_data->base_path);
		remove_path_file_name(link_data->base_path);
		return VISIT_NORMAL;
	}

	if (html_tag->tag_type == TAG_PURETEXT)
	{
		//anchor text
		if (link_data->in_anchor == 1 || link_data->in_option == 1)
		{
			if (link_data->flag == EXTRACT_MERGE)
			{
				copy_html_text(link_data->anchor, link_data->anchor_available, link_data->anchor_end, html_tag->text);
				link_data->anchor_available = strlen(link_data->anchor);
			}
			else if (link_data->flag == EXTRACT_NOMERGE)
			{
				html_deref_to_gb18030_str(html_tag->text, link_data->anchor + link_data->anchor_available, strlen(html_tag->text), link_data->anchor_end - link_data->anchor_available);
				link_data->anchor_available = strlen(link_data->anchor);
			}
			return VISIT_NORMAL;
		}
		if (link_data->available > 0)
		{
			link_data->vlink[link_data->available - 1].inner.text_len += get_valid_text_len(vnode);
		}

		// for compatibility of old-version vhtmlparser
		// text link
		if (vnode->isValid && !vnode->inLink && is_in_sem_area(vnode->hp_area, AREA_SEM_CENTRAL))
		{
			ret = extract_text_link_in_vnode(link_data, vnode);
			if (ret < 0)
				return VISIT_ERROR;
		}
	}

	// for css @import link
	if (html_tag->tag_type == TAG_STYLE)
	{
		if (-1 == extract_vlink_in_style(link_data, vnode))
		{
			return VISIT_ERROR;
		}
	}
	//normal link
	if (html_tag->tag_type == TAG_IMG || html_tag->tag_type == TAG_EMBED || html_tag->tag_type == TAG_FRAME || html_tag->tag_type == TAG_IFRAME)
	{
		src = get_attribute_value(html_tag, "src");
	}
	else if (html_tag->tag_type == TAG_LINK || html_tag->tag_type == TAG_AREA || html_tag->tag_type == TAG_A)
	{
		src = get_attribute_value(html_tag, "href");
	}
	else if (html_tag->tag_type == TAG_META)
	{
		src = get_meta_url(html_tag);
	}
	else if (html_tag->tag_type == TAG_OPTION)
	{
		src = get_attribute_value(html_tag, "value");
		if (src != NULL && is_url_has_protocol_head(src) == 0)
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
	if (vlink_combine_url(link_data->vlink[link_data->available].url, nofollow, src, link_data->base_domain, link_data->base_path, link_data->base_port, link_data->base_url) == -1)
	{
		return VISIT_NORMAL;
	}

	link_data->vlink[link_data->available].text[0] = '\0';

	//prepare anchor text
	if (html_tag->tag_type == TAG_A)
	{
		link_data->anchor = link_data->vlink[link_data->available].text;
		link_data->anchor_available = 0;
		link_data->anchor_end = UL_MAX_TEXT_LEN - 1;
		link_data->in_anchor = 1;
	}
	else if (html_tag->tag_type == TAG_OPTION)
	{
		link_data->anchor = link_data->vlink[link_data->available].text;
		link_data->anchor_available = 0;
		link_data->anchor_end = UL_MAX_TEXT_LEN - 1;
		link_data->in_option = 1;
	}

	// nofollow
	char *rel = get_attribute_value(html_tag, "rel");
	if (rel && html_tag->tag_type == TAG_A)
	{
		if (strcasecmp(rel, "nofollow") == 0 || strcasecmp(rel, "external nofollow") == 0)
		{
			nofollow = 1;
		}
	}

	// the following will assign the vnode info to vlink
	vlink_assign_vnode_property(&(link_data->vlink[link_data->available]), vnode);

	link_data->vlink[link_data->available].inner.route = link_data->route;

	// initialize
	link_data->vlink[link_data->available].position = PAGE_UNDEFINED;
	link_data->vlink[link_data->available].linkFunc = VLINK_UNDEFINED;
	link_data->vlink[link_data->available].nofollow = nofollow;
	link_data->vlink[link_data->available].group = -1;
	link_data->vlink[link_data->available].inner.text_len = 0;
	link_data->vlink[link_data->available].tag_code = vnode->hpNode->html_tag.tag_code;

	link_data->available++;
	return VISIT_NORMAL;
}

/**
 * finish visit TAG_A or TAG_OPTION
 */
static int finish_visit_for_vlink(html_vnode_t *vnode, void *result)
{
	html_tag_t *html_tag = &(vnode->hpNode->html_tag);

	vlink_data_t *link_data;

	link_data = (vlink_data_t *) result;
	link_data->route -= html_tag->tag_type;
	if (html_tag->tag_type == TAG_A)
	{
		link_data->in_anchor = 0;
	}
	else if (html_tag->tag_type == TAG_OPTION)
	{
		link_data->in_option = 0;
	}
	else if (html_tag->tag_type == TAG_META)
	{
		char *content = get_attribute_value(html_tag, "content");
		if (content && strcasecmp(content, "nofollow") == 0)
			link_data->nofollow = 1;
	}
	return VISIT_NORMAL;
}

/**
 * extract vlink from html vnode
 */
static int html_vnode_extract_vlink(html_vnode_t *vnode, char *base_url, base_info_t *base_info, vlink_t *vlink, int num, char flag, char & extra_info)
{
	vlink_data_t link_data;
	char *p = NULL;

	assert(base_url != NULL);
	assert(vlink != NULL);

	link_data.end = num;
	link_data.available = 0;
	link_data.vlink = vlink;
	link_data.need_space = 0;
	link_data.in_anchor = 0;
	link_data.in_option = 0;
	link_data.flag = flag;
	link_data.nofollow = 0;
	link_data.base_url = base_url;
	link_data.base_domain = base_info->base_domain;
	link_data.base_port = base_info->base_port;
	link_data.base_path = base_info->base_path;
	link_data.route = 0;

	if ((p = strchr(link_data.base_path, '#')) != NULL)
		*p = 0;
	// iterate for every top vnode of this area
	start_visit_for_vlink(vnode, &link_data);

	assert(link_data.available <= link_data.end);

	// the following will mark the position and func of every vlink of this area

	// mark position, good-link
	for (int i = 0; i < link_data.available; i++)
	{
		vlink[i].inner.is_goodlink = isGoodLink(vlink + i, base_info);
		vlink[i].inner.is_outlink = isOutLink(vlink + i, base_info->base_domain);
		vlink[i].inner.html_area = NULL;
		vlink[i].inner.anchor_from_alt = 0;
		vlink[i].inner.area_left_link_count = link_data.available - i - 1;
		get_anchor_from_alt(&link_data, i, link_data.flag);
	}

	// nofollow
	if (link_data.nofollow > 0)
		extra_info |= PAGE_NOFOLLOW;

	return link_data.available;
}
/*
 * extract vlink from html area
 */
int html_area_extract_vlink(html_vtree_t *vtree, html_area_t *html_area, char *base_url, base_info_t *base_info, vlink_t *vlink, int num, char flag, char & extra_info)
{
	vlink_data_t link_data;
	char *p = NULL;

	assert(base_url != NULL);
	assert(vlink != NULL);
	assert(num > 0);

	link_data.end = num;
	link_data.available = 0;
	link_data.vlink = vlink;
	link_data.need_space = 0;
	link_data.in_anchor = 0;
	link_data.in_option = 0;
	link_data.flag = flag;
	link_data.nofollow = 0;
	link_data.base_url = base_url;
	link_data.base_domain = base_info->base_domain;
	link_data.base_port = base_info->base_port;
	link_data.base_path = base_info->base_path;
	link_data.route = 0;

	if ((p = strchr(link_data.base_path, '#')) != NULL)
		*p = 0;

	html_vnode_t *vnode = html_area->begin;
	int ret = VISIT_NORMAL;
	// iterate for every top vnode of this area
	while (1)
	{
		if (vnode->hpNode->html_tag.tag_type != TAG_PURETEXT)
			ret = html_vnode_visit(vnode, start_visit_for_vlink, finish_visit_for_vlink, &link_data);

		if (ret == VISIT_ERROR)
		{
			return -1; // failed
		}
		if (vnode == html_area->end)
			break;
		if (link_data.available >= link_data.end)
			break; // have extract enough vlink

		vnode = vnode->nextNode;
		link_data.need_space = 0;
		link_data.in_anchor = 0;
		link_data.in_option = 0;
	}

	assert(link_data.available <= link_data.end);

	// the following will mark the position and func of every vlink of this area

	// mark position, good-link
	for (int i = 0; i < link_data.available; i++)
	{
		vlink[i].position = html_area->abspos_mark;
		vlink[i].inner.is_goodlink = isGoodLink(vlink + i, base_info);
		vlink[i].inner.is_outlink = isOutLink(vlink + i, base_info->base_domain);
		vlink[i].inner.html_area = html_area;
		vlink[i].inner.anchor_from_alt = 0;
		vlink[i].inner.area_left_link_count = link_data.available - i - 1;
		get_anchor_from_alt(&link_data, i, link_data.flag);
	}

	// nofollow
	if (link_data.nofollow > 0)
		extra_info |= PAGE_NOFOLLOW;

	return link_data.available;
}

static int start_visit_for_imglink(html_tag_t *html_tag, void *result, int flag)
{
	vlink_data_t *vlink_data = (vlink_data_t *) result;
	char *value = NULL;

	if (html_tag->tag_type == TAG_IMG)
	{
		value = get_attribute_value(html_tag, "alt");
		if (value != NULL)
		{
			if (vlink_data->anchor_available != 0 && vlink_data->anchor_available < vlink_data->anchor_end)
			{
				vlink_data->anchor[vlink_data->anchor_available] = ';';
				vlink_data->anchor_available++;
			}
			if (flag == 1)
			{
				copy_html_text(vlink_data->anchor, vlink_data->anchor_available, vlink_data->anchor_end, value);
				vlink_data->anchor_available = strlen(vlink_data->anchor);
			}
			else
			{
				html_deref_to_gb18030_str(value, vlink_data->anchor + vlink_data->anchor_available, strlen(value), vlink_data->anchor_end - vlink_data->anchor_available);
				vlink_data->anchor_available = strlen(vlink_data->anchor);
			}
		}
	}
	return VISIT_NORMAL;
}

static void get_anchor_from_alt(vlink_data_t *vlink_data, int index, char flag)
{
	int ret;
	char *anchor = NULL;

	// check the tag is <A>
	if (vlink_data->vlink[index].inner.node->html_tag.tag_type != TAG_A)
		return;

	// check the anchor is NULL or space.
	anchor = vlink_data->vlink[index].text;
	while (*anchor == ' ')
		anchor++;
	if (*anchor != '\0')
		return;

	// check <img> tags in subtree
	vlink_data->anchor = vlink_data->vlink[index].text;
	vlink_data->anchor_available = 0;
	vlink_data->anchor_end = UL_MAX_TEXT_LEN - 1;
	if (flag == EXTRACT_MERGE)
		ret = html_node_visit(vlink_data->vlink[index].inner.node, start_visit_for_imglink, NULL, vlink_data, 1);
	else
		ret = html_node_visit(vlink_data->vlink[index].inner.node, start_visit_for_imglink, NULL, vlink_data, 0);
	if (ret == 0)
		vlink_data->vlink[index].text[0] = '\0';
	else
	{
		anchor = vlink_data->vlink[index].text;
		while (*anchor == ' ' || *anchor == ';')
			anchor++;
		if (*anchor != '\0')
			vlink_data->vlink[index].inner.anchor_from_alt = 1;
		else
			vlink_data->vlink[index].text[0] = '\0';
	}

	return;
}

//有效链接判断（并非简单外链判断，会考虑tag_type及anchor等信息）
static int isGoodLink(vlink_t *vlink, base_info_t *base_info)
{
	char *url = vlink->url;
	char *anchor = vlink->text;
	char s[UL_MAX_SITE_LEN];
	char t[UL_MAX_SITE_LEN];

	if (strlen(anchor) < 1)
		return 0;
	if (url[0] == '\0')
		return 0;
	if (vlink->inner.tag_type != TAG_A)
		return 0;

	if (easou_get_site(url, s) == NULL)
		return 0;
	if (easou_fetch_trunk(s, t, UL_MAX_SITE_LEN) != 1)
		return 0;
	easou_trans2lower(t, t);

	if (easou_fetch_trunk(base_info->base_domain, s, UL_MAX_SITE_LEN) != 1)
		return 1;
	easou_trans2lower(s, s);

	if (strcmp(s, t) != 0)
		return 1;
	else
		return 0;
}

//简单外链判断
static int isOutLink(vlink_t *vlink, const char* base_domain)
{
	char *url = vlink->url;
	char s[UL_MAX_SITE_LEN];
	char t[UL_MAX_SITE_LEN];

	if (url[0] == '\0')
		return 0;
	if (easou_get_site(url, s) == NULL)
		return 0;
	if (easou_fetch_trunk(s, t, UL_MAX_SITE_LEN) != 1)
		return 0;
	easou_trans2lower(t, t);

	if (easou_fetch_trunk(base_domain, s, UL_MAX_SITE_LEN) != 1)
		return 1;
	easou_trans2lower(s, s);

	if (strcmp(s, t) != 0)
		return 1;
	else
		return 0;
}

/**
 * @brief 对连接分组
 */
static void group_link(vlink_t *vlink, int link_count, group_t *pgroup)
{
	pgroup->group_num = 0;
	int group_id = 0;
	int start_i = 0;
	while (start_i < link_count - 1)
	{
		int end_i = start_i + 1;
		while (end_i < link_count)
		{
			if (vlink[end_i].inner.route != vlink[start_i].inner.route)
				break;
			end_i++;
		}
		if (end_i - start_i > 1 && pgroup->group_num < MAX_GROUP_NUM)
		{
			group_link_t *pgl = &pgroup->groups[pgroup->group_num];
			memset(pgl, 0, sizeof(group_link_t));
			//group
			for (int i = start_i; i < end_i; i++)
			{
				if (i < end_i - 1)
					pgl->text_len += vlink[i].inner.text_len;
				pgl->anchor_len += strlen(vlink[i].text);
				pgl->homepage_count += is_homepage(vlink[i].url);
				pgl->link_count++;
				pgl->outer_count += vlink[i].inner.is_goodlink ? 1 : 0;
				pgl->inner_count += vlink[i].inner.is_goodlink ? 0 : 1;
				vlink[i].group = group_id;
			}
			pgroup->group_num++;
			group_id++;
		}
		start_i = end_i;
	}
}

/**
 * @brief 标记链接类型
 * @param [in] root   : html_vnode_t*	root结点
 * @param [in] html_area   : html_area_t*	网页分块信息
 * @param [in] areaCount   : int		网页块数量
 * @param [in] base_url   : char*		网页的url
 * @param [in/out] vlink   : vlink_t*		网页的link集合
 * @param [in] link_count   : int		网页link的数量
 * @param [in] pagetype   : unsigned int	网页类型
 * @param [in] marktype_flag   : unsigned int	需要标记的链接类型
 * @param [in] pvres   : vhp_res_t*		链接资源
 * @return  void
 **/
void html_vtree_mark_vlink(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink, int link_count, unsigned int pagetype, unsigned int marktype_flag, lt_res_t *pvres)
{
	lt_args_t vargs;
	memset(&vargs, 0, sizeof(lt_args_t));
	vargs.root = vtree->root;
	vargs.atree = atree;
	vargs.vtree = vtree;
	vargs.vlink = vlink;
	vargs.link_count = link_count;
	vargs.pagetype = pagetype;
	vargs.url = base_url;

	group_link(vlink, link_count, &pvres->group);

	//检查传入的链接标记和生成的资源链接标志之间是否对应
	if (marktype_flag != pvres->flag)
	{
		Warn("marktype_flag not agreed, input:%d, res:%d\n", marktype_flag, pvres->flag);
	}

	if (marktype_flag & VLINK_NOFOLLOW)
	{
		mark_linktype_nofollow(&vargs, pvres);
	}

	//add
	if (marktype_flag & VLINK_IFRAME)
	{
		mark_linktype_iframe(&vargs, pvres);
	}

	if (marktype_flag & VLINK_INVALID)
	{
		mark_linktype_invalid_and_control(&vargs, pvres);
	}
	if (marktype_flag & VLINK_NAV)
	{
		mark_linktype_nav(&vargs, pvres);
	}

	if (marktype_flag & VLINK_FRIEND)
	{
		mark_linktype_friend(&vargs, pvres);
	}

	if (marktype_flag & VLINK_MYPOS)
	{
		mark_linktype_mypos(&vargs, pvres);
	}

	if (marktype_flag & VLINK_HIDDEN)
	{
		mark_linktype_hidden(&vargs, pvres);
	}

	if (marktype_flag & VLINK_COPYRIGHT)
	{
		mark_linktype_copyright(&vargs, pvres);
	}

	if (marktype_flag & VLINK_FRIEND_EX)
	{
		mark_linktype_friendex(&vargs, pvres);
	}

	if (marktype_flag & VLINK_SELFHELP)
	{
		mark_linktype_selfhelp(&vargs, pvres);
	}

	if (marktype_flag & VLINK_CSS)
	{
		mark_linktype_css(&vargs, pvres);
	}

	if (marktype_flag & VLINK_IMAGE)
	{
		mark_linktype_image(&vargs, pvres);
	}

	if (marktype_flag & VLINK_IMG_SRC)
	{
		mark_linktype_img_src(&vargs, pvres);
	}

	if (marktype_flag & VLINK_EMBED_SRC)
	{
		mark_linktype_embed_src(&vargs, pvres);
	}

	if (marktype_flag & VLINK_BLOG_MAIN)
	{
		mark_linktype_blogmain(&vargs, pvres);
	}

	if (marktype_flag & VLINK_QUOTATION)
	{
		mark_linktype_quotation(&vargs, pvres);
	}

	if (marktype_flag & VLINK_BLOGRE)
	{
		mark_linktype_blogre(&vargs, pvres);
	}

	if (marktype_flag & VLINK_BBSRE)
	{
		mark_linktype_bbsre(&vargs, pvres);
	}

	if (marktype_flag & VLINK_CONT_INTERLUDE)
	{
		mark_linktype_cont_interlude(&vargs, pvres);
	}

	if (marktype_flag & VLINK_TEXT_LINK)
	{
		mark_linktype_text(&vargs, pvres);
	}

	if (marktype_flag & VLINK_FRIEND)
	{
		mark_linktype_friend_by_group(&vargs, pvres);
	}
}

//获取孩子中的图片结点 以当前节点为子树的所有节点均进行检查 flag=0-sub_img flag=1-near_img
static html_vnode_t *get_sub_img_vnode(html_vnode_t *vnode, int flag)
{
	stack<html_vnode_t *> stack_node; //定义栈

	if (vnode == NULL || vnode->firstChild == NULL)
		return NULL;
	if (vnode->hpNode->html_tag.tag_type == TAG_IMG)
		return vnode;

	html_vnode_t *node = vnode->firstChild;

	while (node != NULL || !stack_node.empty())
	{
		while (node != NULL)
		{
			if (node->hpNode->html_tag.tag_type == TAG_IMG)
			{
				return node;
			}
			if (flag == 0 || node->hpNode->html_tag.tag_type != TAG_DIV) //near_img--slip div
			{
			stack_node.push(node);
			node = node->firstChild;
		}
			else
			{
				node = node->nextNode;
			}

		}
		if (!stack_node.empty())
		{
			node = stack_node.top();
			stack_node.pop();
			node = node->nextNode;
		}
	}
	return NULL;
}

static html_vnode_t* get_near_img_vnode(html_vnode_t *vnode)
{
	html_vnode_t *node = vnode;
	if (node == NULL)
		return NULL;

	for (int i = 1; i < 5 && node->upperNode; i++)
	{
		if (node->hpNode->html_tag.tag_type != TAG_DIV)
		{
			html_vnode_t *pat_node = node->upperNode;
			html_vnode_t *img_node = get_sub_img_vnode(pat_node, 1);
			if (img_node != NULL && img_node->upperNode->hpNode->html_tag.tag_type != TAG_A)
				return img_node;
			else
				node = node->upperNode;
		}
		else
			return NULL;
	}
	return NULL;
}

char* get_url_value(html_tag_t *tag)
{
//返回正确的url值
	int flag = 0; //无src时为0；有src时为1－－其他属性值无jpg结尾的为2；有src且其他属性值有jpg结尾的为3
	html_attribute_t *attr = NULL;
	const char *postfix = ".jpg"; //后缀是jpg
	const char *postfix1 = "http://"; //没有后缀 针对youku
//针对没有src属性的情况，检查该标签的每个属性，其值是否存在以jpg、png结尾的

	//strstr(attr->value, postfix)== NULL --jpg ending

	for (attr = tag->attribute; attr; attr = attr->next) //判断有无src
	{
		if (attr->type == ATTR_SRC)
		{
			flag = 0;
		}
		else
		{
			flag = 1;
			break;
		}
	}

	if (flag == 1) //细分flag为1的情况
	{
		for (attr = tag->attribute; attr; attr = attr->next)
		{
			if (attr->type != ATTR_SRC)
			{
				if (strstr(attr->value, postfix) == NULL && strstr(attr->value, postfix1) == NULL)
				{
					flag = 2;
				}
				else
				{
					flag = 3;
					break;
				}
			}
		}

	}

	for (attr = tag->attribute; attr; attr = attr->next)
	{
		//没有src属性的情况，img_url为后缀是jpg的值
		if (flag == 0 && strstr(attr->value, postfix) != NULL)
			return attr->value;
		//存在src，img_url的值是src属性值
		if (flag == 2)
			return get_attribute_value(tag, ATTR_SRC);
		//存在src，img_url的值非src属性值
		if (flag == 3 && (attr->type != ATTR_SRC) && (strstr(attr->value, postfix) != NULL || strstr(attr->value, postfix1) != NULL))
			return attr->value;
	}

	/*if (flag == 0) //没有src属性的情况，img_url为后缀是jpg的值
	{
		for (attr = tag->attribute; attr; attr = attr->next)
		{
			if (strstr(attr->value, postfix) != NULL)
				return attr->value;
		}
	}
	 if (flag == 2) //存在src，img_url的值是src属性值
		return get_attribute_value(tag, ATTR_SRC);

	 if (flag == 3) //存在src，img_url的值非src属性值
	{
		for (attr = tag->attribute; attr; attr = attr->next)
		{
			if ((attr->type != ATTR_SRC) && (strstr(attr->value, postfix) != NULL || strstr(attr->value, postfix1) != NULL))
				return attr->value;
		}
	 }*/

	return NULL;
}

//正确提取图片的链接信息，可能存在无src属性，也可能存在src但其图片的正确链接地址存于其他属性如alt、_src等
int extract_img_infos(const char *url, int url_len, area_tree_t* atree, vlink_t *vlinks, int num, img_info_t *imgs, int size)
{
	int img_num = 0;

	//pic_in_content
	const area_list_t *alist = get_func_mark_result(atree, AREA_FUNC_ARTICLE_CONTENT);
	if (alist)
	{
		for (area_list_node_t* area = alist->head; area; area = area->next)
		{
			if (area->area == NULL)
				continue;
			for (html_vnode_t *v_node = area->area->begin; v_node && v_node->hx > 0 && v_node->wx > 0 && img_num < size; v_node = v_node->nextNode)
			{
				html_vnode_t *img_vnode = get_sub_img_vnode(v_node, 0);
				if (img_vnode != NULL && img_vnode->trust >= 0)
				{
					const char* img_url = get_url_value(&img_vnode->hpNode->html_tag);
					if (img_url == NULL || strstr(img_url, ".gif") != NULL)
						continue;
					int img_url_len = strlen(img_url);
					if (img_url_len >= UL_MAX_URL_LEN)
						continue;
					imgs[img_num].type = IMG_IN_CONTENT;
					memcpy(imgs[img_num].img_url, img_url, img_url_len);
					imgs[img_num].img_url[img_url_len] = 0;
					imgs[img_num].img_hx = img_vnode->hx;
					imgs[img_num].img_wx = img_vnode->wx;
					imgs[img_num].trust = img_vnode->trust;
					imgs[img_num].owner = NULL;
					img_num++;
					continue;

					if (v_node == area->area->end)
						break;
				}

			}
			if (area == alist->tail)
				break;
		}
	}

	for (int i = 0; i < num; i++)
	{
		if (img_num == size)
			return img_num;

		vlink_t *vlink = &vlinks[i];

		if (vlink->inner.node == NULL || vlink->inner.node->child == NULL) //去掉无孩子节点的标签
			continue;

		html_vnode_t *img_vnode = get_sub_img_vnode(vlink->inner.vnode, 0); //pic_in_anchor

		if (img_vnode != NULL)
		{
			const char* img_url = get_url_value(&img_vnode->hpNode->html_tag);
			if (img_url == NULL)
				continue;
			int img_url_len = strlen(img_url);
			if (img_url_len >= UL_MAX_URL_LEN)
				continue;
			//img_info_t* img = &imgs[img_num++];
			imgs[img_num].type = IMG_IN_ANCHOR;
			memcpy(imgs[img_num].img_url, img_url, img_url_len);
			imgs[img_num].img_url[img_url_len] = 0;
			imgs[img_num].img_hx = img_vnode->hx;
			imgs[img_num].img_wx = img_vnode->wx;
			imgs[img_num].trust = img_vnode->trust;
			imgs[img_num].owner = vlink;
			img_num++;
		}
		else
		{
			img_vnode = get_near_img_vnode(vlink->inner.vnode);
			if (img_vnode == NULL)
				continue;
			const char* img_url = get_url_value(&img_vnode->hpNode->html_tag);
			if (img_url == NULL)
				continue;

			//防止pic_in_anchor与pic_near_anchor重复
			int repeate = 0;
			for (int k = 0; k < img_num; k++)
			{
				if ((strcmp(img_url, imgs[k].img_url) == 0) && imgs[k].type == IMG_IN_ANCHOR)
				{
					repeate = 1;
					break;
				}
			}
			if (repeate == 1)
				continue;

			int img_url_len = strlen(img_url);
			if (img_url_len >= UL_MAX_URL_LEN)
				continue;

			if (img_vnode->xpos != vlink->inner.xpos) //位置匹配
				continue;
			imgs[img_num].type = IMG_NEAR_ANCHOR;
			memcpy(imgs[img_num].img_url, img_url, img_url_len);
			imgs[img_num].img_url[img_url_len] = 0;
			imgs[img_num].img_hx = img_vnode->hx;
			imgs[img_num].img_wx = img_vnode->wx;
			imgs[img_num].trust = img_vnode->trust;
			imgs[img_num].owner = vlink;
			img_num++;
		}
	}
	return img_num;
}

static const int MAX_AREA_ROOT_NUM = 255;

//对分块获取相关的root结点
static int get_area_root_vnode(html_area_t *area, html_vnode_t *vnodes[], int size)
{
	int num = 0;
	html_vnode_t *vnode = area->begin->upperNode;
	while (vnode && num < size)
	{
		vnodes[num++] = vnode;
		vnode = vnode->upperNode;
	}
	return num;
}

//去重vnode
static int dedup_vnode(html_vnode_t *vnodes[], int num)
{
	if (num >= MAX_AREA_ROOT_NUM)
		return -1;

	int dup[MAX_AREA_ROOT_NUM];
	memset(dup, 0, sizeof(dup));

	for (int i = 0; i < num; i++)
	{
		if (dup[i])
			continue;

		for (int j = i + 1; j < num; j++)
		{
			if (vnodes[i] == vnodes[j])
			{
				dup[j] = 1;
			}
		}
	}
	int real_num = 0;
	for (int i = 0; i < num; i++)
	{
		if (!dup[i])
		{
			vnodes[real_num++] = vnodes[i];
		}
	}
	return real_num;
}

/**
 * extract vlink from html page
 * Input : root, the root of html_vtree which parsed OK;
 * 	   html_area, the partitioned area set;
 * 	   areaCount, the count of html areas in this web page;
 * 	   base_url, the base url of this web page;
 * 	   maxnum, the max available space count of vlinks
 * Ouput : vlink, the space stored the extracted vlinks
 * Return: the num of vlinks extracted actually
 * 
 * Pre-Require : the html areas HAVE partitioned and marked position
 */
static int html_vtree_extract_vlink_inner(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink, int maxnum, int flag)
{
	assert(base_url);
	assert(maxnum > 0);
	int extracted_count = 0;
	char extra_info = 0;
	int ret_num = 0;

	if (strlen(base_url) >= UL_MAX_URL_LEN)
	{
		Error("Too long base url:%s", base_url);
		return -1;
	}
//提取链接类型
	base_info_t base_info;
	base_info.base_domain[0] = '\0';
	base_info.base_port[0] = '\0';
	base_info.base_path[0] = '\0';

	if (!easou_parse_url_ex(base_url, base_info.base_domain, UL_MAX_SITE_LEN, base_info.base_port, UL_MAX_PORT_LEN, base_info.base_path, UL_MAX_PATH_LEN))
	{
		Error("Error base url:%s", base_url);
		return -1;
	}
	html_area_t *subArea = NULL;
	if (atree->root->subArea)
		subArea = atree->root->subArea;
	else
		subArea = atree->root;

//获取每个子分块的root结点，防止在遍历subArea时，无法覆盖到这些节点，而少链接
	html_vnode_t *root_vnodes[MAX_AREA_ROOT_NUM];
	int root_vnode_num = 0;

	for (; subArea; subArea = subArea->nextArea)
	{
//获取每个子分块的root结点，防止在遍历subArea时，无法覆盖到这些节点，而少链接
		root_vnode_num += get_area_root_vnode(subArea, root_vnodes + root_vnode_num, MAX_AREA_ROOT_NUM - root_vnode_num);

		if (extracted_count >= maxnum)
			break;
		ret_num = html_area_extract_vlink(vtree, subArea, base_url, &base_info, vlink + extracted_count, maxnum - extracted_count, flag, extra_info);
		if (ret_num < 0)
			break;
		extracted_count += ret_num;
	}

//从每个area的root结点中补充链接
	root_vnode_num = dedup_vnode(root_vnodes, root_vnode_num);
	for (int i = 0; i < root_vnode_num; i++)
	{
		ret_num = html_vnode_extract_vlink(root_vnodes[i], base_url, &base_info, vlink + extracted_count, maxnum - extracted_count, flag, extra_info);
		if (ret_num < 0)
			break;
		extracted_count += ret_num;
	}

	if (extra_info & PAGE_NOFOLLOW)
	{
		for (int i = 0; i < extracted_count; i++)
			vlink[i].nofollow = 1;
	}

//提取外部css的链接
	int linknum = 250;
	link_t links[250];					//保存链接信息
	int reallinknum = html_tree_extract_csslink(vtree->hpTree, base_url, links, linknum);
	int cssadded = 0;
	for (int i = 0; i < reallinknum; i++)
	{
		if (maxnum - extracted_count - i <= 0)
		{
			break;
		}
		memset(&(vlink[extracted_count + i]), 0, sizeof(vlink_t));
		vlink[extracted_count + i].linkFunc = VLINK_CSS;
		if (url_protocol_head_len(base_url) == 7)
			memcpy(vlink[extracted_count + i].url, "http://", 7);
		else if (url_protocol_head_len(base_url) == 8)
			memcpy(vlink[extracted_count + i].url, "https://", 8);

		//memcpy(vlink[extracted_count + i].url, links[i].url, strlen(links[i].url));
		strncat(vlink[extracted_count + i].url, links[i].url, strlen(links[i].url));
		cssadded++;
	}

	return extracted_count + cssadded;
}

/**
 * extract vlink from html page
 * Input : root, the root of html_vtree which parsed OK;
 * 	   html_area, the partitioned area set;
 * 	   areaCount, the count of html areas in this web page;
 * 	   base_url, the base url of this web page;
 * 	   maxnum, the max available space count of vlinks
 * Ouput : vlink, the space stored the extracted vlinks
 * Return: the num of vlinks extracted actually
 * 
 * Pre-Require : the html areas HAVE partitioned and marked position
 */
int html_vtree_extract_vlink(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink, int maxnum)
{
	return html_vtree_extract_vlink_inner(vtree, atree, base_url, vlink, maxnum, EXTRACT_MERGE);
}

int html_vtree_extract_vlink_nomerge(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink, int maxnum)
{
	return html_vtree_extract_vlink_inner(vtree, atree, base_url, vlink, maxnum, EXTRACT_NOMERGE);
}

/*
 * get content refresh url in meta
 */
static char *get_meta_url(html_tag_t *html_tag)
{
	char *p, *q;

	p = get_attribute_value(html_tag, "http-equiv");
	if (p == NULL || strcasecmp(p, "refresh") != 0)
	{
		return NULL;
	}
	p = get_attribute_value(html_tag, "content");
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

/* 
 * is absolute url
 */
static int is_url(const char *url)
{
	return is_url_has_protocol_head(url);
}

/*
 * is network path
 */
static int is_net_path(char *path)
{
	if (strncmp(path, "//", 2) == 0)
		return 1;
	return 0;
}
/*
 * is absolute path
 */
static int is_abs_path(char *path)
{
	if (*path == '/')
		return 1;
	return 0;
}

/*
 * is relative path
 */
static int is_rel_path(char *url)
{
	char *p;
	if (is_url(url))
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

lt_res_t *linktype_res_create(unsigned int vhp_mark_type)
{
	const char *where = "vhplink_res_create()";
	lt_res_t *pvres = NULL;

	pvres = (lt_res_t *) calloc(1, sizeof(lt_res_t));
	if (pvres == NULL)
	{
		Error("calloc pvres error. %s", where);
		linktype_res_del(pvres);
		return NULL;
	}

	if (vhp_mark_type & VLINK_BLOG_MAIN)
	{
		pvres->res_blogmain = blogmain_res_create();
		if (pvres->res_blogmain == NULL)
		{
			Error("blogmain_res_create error. %s", where);
			linktype_res_del(pvres);
			return NULL;
		}
	}

	if (vhp_mark_type & VLINK_CONT_INTERLUDE)
	{
		pvres->res_cont_interlude = cont_interlude_res_create();
		if (pvres->res_cont_interlude == NULL)
		{
			Error("cont_interlude res create error. %s", where);
			linktype_res_del(pvres);
			return NULL;
		}
	}

	pvres->ptime_match = timematch_pack_create();
	if (pvres->ptime_match == NULL)
	{
		Error("timematch pack create error");
		linktype_res_del(pvres);
		return NULL;
	}

	pvres->area_info = create_area_info();
	if (pvres->area_info == NULL)
	{
		linktype_res_del(pvres);
		Error("create_vhplk_area_info error.");
		return NULL;
	}

	pvres->tag_code_size = 65536;
	pvres->tag_code_len = (int *) calloc(pvres->tag_code_size, sizeof(int));
	if (pvres->tag_code_len == NULL)
	{
		linktype_res_del(pvres);
		Error("calloc tag code len error.");
		return NULL;
	}

	/* TODO
	 pvres->post_list = bbs_post_list_alloc();
	 if (pvres->post_list == NULL)
	 {
	 vhp_res_del(pvres);
	 Error("bbs post list alloc error");
	 return NULL;
	 }
	 */

	pvres->flag = vhp_mark_type;
	return pvres;
}

void linktype_res_reset(lt_res_t *pvres)
{
	memset(pvres->tag_code_len, 0, sizeof(int) * pvres->tag_code_size);
	bbs_post_list_reset(pvres->post_list);
	pvres->group.group_num = 0;
}

void linktype_res_del(lt_res_t * pvres)
{
	if (pvres)
	{
		blogmain_res_del(pvres->res_blogmain);
		cont_interlude_res_del(pvres->res_cont_interlude);
		timematch_pack_del(pvres->ptime_match);
		del_area_info(pvres->area_info);
		free(pvres->tag_code_len);
		bbs_post_list_destroy(pvres->post_list);
		free(pvres);
	}
	return;
}

static int parse_base_url(base_info_t *base_info, const char *base_url)
{
	base_info->base_domain[0] = '\0';
	base_info->base_port[0] = '\0';
	base_info->base_path[0] = '\0';

	if (!easou_parse_url_ex(base_url, base_info->base_domain, UL_MAX_SITE_LEN, base_info->base_port, UL_MAX_PORT_LEN, base_info->base_path, UL_MAX_PATH_LEN))
	{
		Error("Error base url:%s", base_url);
		return -1;
	}

	return 1;
}

/* TODO
 static int add_css_vlink(vlink_t *vlink, const css_link_node_t *ln, base_info_t *base_info)
 {
 vlink_clean(vlink);
 char nofollow = 0;
 if (vlink_combine_url(vlink->url, nofollow, (char *) ln->url, base_info->base_domain, base_info->base_path,
 base_info->base_port) == -1)
 {
 return -1;
 }
 vlink->inner.is_for_screen = ln->is_for_screen;
 return 1;
 }
 */

/**
 * @brief 从CSS中提取链接。
 * @param [in] css_text   : const char*	CSS的文本.
 * @param [in] base_url   : const char*	页面url,或用于拼接的base url.
 * @param [out] vlink   : vlink_t*	用于存储解析出的link的数组.
 * @param [in] maxnum   : int	最多容纳的LINK数量.
 * @param [in] is_mark   : bool 是否同时标记链接属性.
 * @return  int -1:error; >=0:提取出的链接数量.
 **/
int css_extract_vlink(const char *css_text, const char *base_url, vlink_t *vlink, int maxnum, bool is_mark)
{
	int link_num = 0;
	base_info_t base_info;
	if (parse_base_url(&base_info, base_url) == -1)
	{
		return -1;
	}

	// add css link
	/* TODO
	 if (-1 == extract_link_in_css(css_text, NULL))
	 {
	 return -1;
	 }

	 for (const css_link_node_t *ln = iterator_get_first_css_link(); ln; ln = iterator_get_next_css_link())
	 {
	 if (link_num < maxnum && add_css_vlink(vlink + link_num, ln, &base_info) == 1)
	 {
	 link_num++;
	 }
	 else
	 {
	 Warn("%s:%d:buffer not enough or illegal url.", __FILE__, __LINE__);
	 }
	 }
	 */

	if (link_num > 0 && is_mark)
	{
		// mark property
		for (int i = 0; i < link_num; i++)
		{
			vlink[i].inner.is_goodlink = isGoodLink(vlink + i, &base_info);
			vlink[i].inner.is_outlink = isOutLink(vlink + i, base_info.base_domain);
			if (vlink[i].inner.is_for_screen)
			{
				vlink[i].linkFunc |= VLINK_CSS;
			}
		}
	}

	return link_num;
}

// 简单url pattern，用于在html_deref前发现url
static const char simple_url_pattern[] = "(http:// | https://){0,1}(www\\.){0,1}([a-z0-9](\\w|-){0,63}\\.){1,7}(com|cn|org|edu|info|net|gov|cc|tv|hk|tw)";
//static const char simple_url_pattern[] = "(http://){0,1}(www\\.){0,1}([a-z0-9](\\w|-){0,63}\\.){1,7}(com|cn|org|edu|info|net|gov|cc|tv|hk|tw)";

// 复杂url pattern，用于在html_deref后提取准确的url
static const char url_pattern[] = "(http:// | https://){0,1}(www\\.){0,1}([a-z0-9](\\w|-)*\\.){1,}(com|cn|org|edu|info|net|gov|cc|tv|hk|tw)(/((\\w|[.!?/=&_-])*(\\w|[:/=&_-])+){0,})?";
//static const char url_pattern[] = "(http://){0,1}(www\\.){0,1}([a-z0-9](\\w|-)*\\.){1,}(com|cn|org|edu|info|net|gov|cc|tv|hk|tw)(/((\\w|[.!?/=&_-])*(\\w|[:/=&_-])+){0,})?";

// 支持最大0个字符子串
static const int OVECTOR_MAX_SIZE = 2;

static pcre* simple_url_pcre = NULL; // 对应简单url pattern的pcre
static pcre* url_pcre = NULL; // 对应复杂url pattern的pcre
static pcre_extra* simple_url_pcre_extra = NULL; // 对应简单url pattern的pcre extra
static pcre_extra* url_pcre_extra = NULL; // 对应复杂url pattern的pcre extra

/**
 * @brief 初始化正规表达式结构体：simple_url_pcre和url_pcre
 * @return  void 
 * @retval   
 **/
static void init_url_pcre()
{
	const char* err;
	int erroffset;

	// init pcre
	simple_url_pcre = pcre_compile(simple_url_pattern, PCRE_CASELESS, &err, &erroffset, NULL);
	if (!simple_url_pcre)
	{
		Error("%s:%d Failed to compile simple_url_pattern! offset=%d; err=%s\n", __FILE__, __LINE__, erroffset, err);
		assert(0);
	}
	url_pcre = pcre_compile(url_pattern, PCRE_CASELESS, &err, &erroffset, NULL);
	if (!url_pcre)
	{
		Error("%s:%d Failed to compile url_pattern! offset=%d; err=%s\n", __FILE__, __LINE__, erroffset, err);
		assert(0);
	}

	// init prce_extra
	simple_url_pcre_extra = pcre_study(simple_url_pcre, 0, &err);
	if (err != NULL)
	{
		Error("%s:%d Failed to study simple_url_pattern! err=%s\n", __FILE__, __LINE__, err);
		simple_url_pcre_extra = NULL;
		assert(0);
	}
	url_pcre_extra = pcre_study(url_pcre, 0, &err);
	if (err != NULL)
	{
		Error("%s:%d Failed to study url_pattern! err=%s\n", __FILE__, __LINE__, err);
		url_pcre_extra = NULL;
		assert(0);
	}
}

/**
 * @brief 初始化正规表达式结构体，并保证只初始化一次
 * @return  void 
 * @retval   
 **/
static void init_url_pcre_once()
{
	static pthread_once_t key_once = PTHREAD_ONCE_INIT;
	if (simple_url_pcre && url_pcre && simple_url_pcre_extra && url_pcre_extra)
	{
		return;
	}
	pthread_once(&key_once, init_url_pcre);
}

/**
 * @brief 释放正规表达式结构体
 *
 * @return  void 
 * @retval   
 **/
void fin_url_pcre()
{
	if (simple_url_pcre)
		pcre_free(simple_url_pcre);
	if (url_pcre)
		pcre_free(url_pcre);
	if (simple_url_pcre_extra)
		pcre_free(simple_url_pcre_extra);
	if (url_pcre_extra)
		pcre_free(url_pcre_extra);
}

/**
 * @brief 是否有效url
 * @param [in] text   : const char* 包含url的文字
 * @param [in] match   : regmatch_t& 文字中发现url时的regmatch信息
 * @return  bool 
 * @retval  true 有效；false 无效
 **/
static bool is_valid_text_link(const char* text, int so, int eo)
{
	const char* pch = NULL;

	// make sure the url is not part of an email address
	if (so > 0)
	{
		pch = text + so - 1;
		if (*pch == '@' || *pch == '#')
		{
			return false;
		}
	}

	// make sure the url is not a 'non-http' url
	if (so > 2)
	{
		pch = text + so - 3;
		if (pch[0] == ':' && pch[1] == '/' && pch[2] == '/')
		{
			if (so < 7)
				return false;
			else
			{
				pch = text + so - 7;
				if (strncasecmp(pch, "http://", 7) != 0)
					return false;
			}
		}
	}

	return true;
}

/**
 * @brief 从一段html文字中提取出文字链接
 * @param [in] text   : const char* html文字
 * @param [in] text_len   : int html文字长度
 * @param [in/out] vlink   : vlink_t* 保存链接的数组
 * @param [in] maxnum   : int 最大能保存的链接数
 * @return  int 
 * @retval  >=0 成功提取的文字链接数；-1 错误
 **/
int text_extract_vlink(const char* text, int text_len, vlink_t *vlink, int maxnum)
{
	int link_num = 0;
	int len = 0;
	int ret = 0;
	int skip = 0;
	int url_len = 0;
	int so = 0;
	int eo = 0;
	int ovector[OVECTOR_MAX_SIZE];
	const char* pch = NULL;
	const char* url = NULL;
	vlink_t* current_vlink = NULL;

	init_url_pcre_once();

	// extract links in text
	pch = text;
	len = text_len;
	while (1)
	{
		ret = pcre_exec(simple_url_pcre, simple_url_pcre_extra, pch, len, 0, 0, ovector, OVECTOR_MAX_SIZE);
		if (ret == PCRE_ERROR_NOMATCH)
		{
			break;
		}
		else if (ret < 0)
		{
			Warn("%s:%d Failed to search for simple_url_pattern.", __FILE__, __LINE__);
			break;
		}

		so = ovector[0];
		eo = ovector[1];
		skip = eo;
		url_len = eo - so;

		if (!is_valid_text_link(pch, so, eo))
		{
			pch += skip;
			len -= skip;
			continue;
		}
		if (link_num < maxnum)
		{
			current_vlink = vlink + link_num;
			vlink_clean(current_vlink);
			url = pch + so;

			ret = html_deref_to_gb18030_str((char*) url, current_vlink->url, strlen(url), UL_MAX_URL_LEN);
			if (ret < 0)
			{
				pch += skip;
				len -= skip;
				continue;
			}

			ret = pcre_exec(url_pcre, url_pcre_extra, current_vlink->url, strlen(current_vlink->url), 0, 0, ovector, OVECTOR_MAX_SIZE);
			if (ret < 0)
			{
				if (ret != PCRE_ERROR_NOMATCH)
				{
					Warn("%s:%d Failed to search for url_pattern.", __FILE__, __LINE__);
				}
				pch += skip;
				len -= skip;
				continue;
			}

			so = ovector[0];
			eo = ovector[1];
			skip = skip - url_len + eo;
			if (eo >= UL_MAX_URL_LEN)
			{
				Info("%s:%d Buffer not enough or illegal url.", __FILE__, __LINE__);
				pch += skip;
				len -= skip;
				continue;
			}
			current_vlink->url[eo] = '\0';

			if (easou_normalize_url(current_vlink->url, current_vlink->url) == 0)
			{
				Info("%s:%d Failed to normalize url.", __FILE__, __LINE__);
				pch += skip;
				len -= skip;
				continue;
			}
			if (vlink_url_recheck(current_vlink->url) == false)
			{
				current_vlink->url[0] = '\0';
				pch += skip;
				len -= skip;
				continue;
			}
		}
		else
		{
			Warn("%s:%d Link number reached maximum size.", __FILE__, __LINE__);
			break;
		}

		pch += skip;
		len -= skip;
		link_num++;
	}

	return link_num;
}

/**
 * @brief 从TAG_PURETEXT类型的vnode中提取链接
 * @param [in/out] link_data   : vlink_data_t* 链接提取信息结构体
 * @param [in] vnode   : html_vnode_t* 待提链的vnode
 * @return  int 
 * @retval  >=0 成功提取的文字链接数；-1 错误
 **/
static int extract_text_link_in_vnode(vlink_data_t *link_data, html_vnode_t *vnode)
{
	html_tag_t *html_tag = &vnode->hpNode->html_tag;
	assert(TAG_PURETEXT == html_tag->tag_type);

	vlink_t *vlink = link_data->vlink + link_data->available;
	int leftnum = link_data->end - link_data->available;
	int link_num = 0;
	if (leftnum <= 0)
	{
		return link_num;
	}
	if (html_tag->text)
	{
		// extract text links
		link_num = text_extract_vlink(html_tag->text, strlen(html_tag->text), vlink, leftnum);
		if (-1 == link_num)
		{
			return -1;
		}
		// assign properties
		for (int i = 0; i < link_num; i++)
		{
			vlink_t *curr_vlink = link_data->vlink + link_data->available;
			vlink_assign_vnode_property(curr_vlink, vnode);
			curr_vlink->linkFunc |= VLINK_TEXT;
			curr_vlink->inner.route = link_data->route;
			curr_vlink->inner.is_goodlink = 0;
			curr_vlink->inner.is_outlink = isOutLink(curr_vlink, link_data->base_domain);
			link_data->available++;
		}
	}

	return link_num;
}
