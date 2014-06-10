/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_friendex.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_friendex.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记友情链接
 **/

#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_vhtml_parser.h"
#include "html_text_utils.h"
#include <ctype.h>

#define MAX_DECLARE_LEN 128

static char * exchange_link_key1[] =
{ "pr分值", "pr值", "pr", "访问量", "日访问", "日ip", "ip" };

static char * exchange_link_key2[] =
{ ">=", ">", "&gt;", "&gt", "大于等于", "大于" };

//judge the text is a exchange link declare
int get_declare_text(char * src, char * declare, int size)
{
	if (!src || !declare)
		return 0;

	int len = 0;
	char * p = src;

	while (*p)
	{
		if (easou_isspace((unsigned char) (*p)))
		{
			p++;
		}
		else if (p + 4 && p[0] == '&' && p[1] == 'n' && p[2] == 'b' && p[3] == 's' && p[4] == 'p')
		{
			p += 5;
			if (*(p + 1) == ';')
				p++;
		}
		else
		{
			if (len + 1 < size)
			{
				declare[len++] = *p++;
			}
			else
			{
				len = 0;
				break;
			}
		}
	}

	declare[len] = 0;
	return len;
}

int mark_friendex_vlink(vlink_t *vlink, int link_count, html_vnode_t *declare_node)
{
	int bstart = 0; // begin flag
	int blinkdown = 1; // the friend-ex link in the upper of link-declare
	int front_dist = -1; // the distance between link-declare and its front link
	int back_dist = -1; // the distance between link-declare and its back link
	int cnt = 0; // the friend link-count over link declare
	int fcnt = 0; // friend link count
	int last_pos = 0; // last friend link pos
	int pre_pos = -1;

	if (link_count == 0)
		return 0;

	for (int i = 0; i < link_count; i++)
	{
		if (vlink[i].linkFunc & VLINK_FRIEND)
		{
			fcnt++;
			last_pos = i;
			if (declare_node->ypos > vlink[i].inner.ypos + vlink[i].inner.height)
			{
				front_dist = declare_node->ypos - vlink[i].inner.ypos - vlink[i].inner.height;
				cnt++;
			}
			else if (declare_node->ypos + declare_node->hx <= vlink[i].inner.ypos)
			{
				if (back_dist < 0)
					back_dist = vlink[i].inner.ypos - declare_node->ypos - declare_node->hx;
			}
		}
	} //judge if the declare below the links

	if ((front_dist < 0 || front_dist > back_dist) && back_dist >= 0)
		blinkdown = 1;
	else if ((back_dist < 0 || front_dist < back_dist) && front_dist >= 0)
		blinkdown = 0;
	else if (vlink[last_pos].inner.ypos < declare_node->ypos
			|| (vlink[last_pos].inner.ypos == declare_node->ypos && vlink[last_pos].inner.xpos < declare_node->xpos)
			|| (fcnt && cnt > 9 * fcnt / 10))
		blinkdown = 0;

	if (blinkdown)
	{
		for (int i = 0; i < link_count; i++)
		{
			if (vlink[i].linkFunc & VLINK_INVALID)
				continue;
			if (!bstart)
			{
				if (declare_node->ypos > vlink[i].inner.ypos
						|| (vlink[i].inner.ypos != declare_node->ypos
								&& vlink[i].inner.ypos - declare_node->ypos - declare_node->hx > 20)
						|| (vlink[i].inner.ypos == declare_node->ypos
								&& vlink[i].inner.xpos - declare_node->xpos - declare_node->wx < 0))
				{
					continue;
				}

				bstart = 1;
			}

			if (bstart)
			{
				if (pre_pos >= 0 && vlink[i].inner.ypos != vlink[pre_pos].inner.ypos
						&& vlink[i].inner.ypos - vlink[pre_pos].inner.ypos - vlink[pre_pos].inner.height > 20)
					break;
				if (vlink[i].linkFunc & VLINK_FRIEND)
					vlink[i].linkFunc |= VLINK_FRIEND_EX;
			}
			pre_pos = i;
		}
	}
	else
	{
		for (int i = link_count - 1; i >= 0; i--)
		{
			if (vlink[i].linkFunc & VLINK_INVALID)
				continue;
			if (!bstart)
			{
				if (declare_node->ypos < vlink[i].inner.ypos
						|| (vlink[i].inner.ypos != declare_node->ypos
								&& declare_node->ypos - vlink[i].inner.ypos - vlink[i].inner.height > 20)
						|| (vlink[i].inner.ypos == declare_node->ypos
								&& declare_node->xpos - vlink[i].inner.xpos - vlink[i].inner.width < 0))
				{
					continue;
				}

				bstart = 1;
			}

			if (bstart)
			{
				if (pre_pos >= 0 && vlink[i].inner.ypos != vlink[pre_pos].inner.ypos
						&& vlink[pre_pos].inner.ypos - vlink[i].inner.ypos - vlink[i].inner.height > 20)
					break;
				if (vlink[i].linkFunc & VLINK_FRIEND)
					vlink[i].linkFunc |= VLINK_FRIEND_EX;
			}

			pre_pos = i;
		}
	}
	return 1;
}

int is_exchange_link_declare(char * buff)
{
	if (!buff)
	{
		return 0;
	}

	int size = 0;
	char * p = NULL, *q = NULL;
	int len = 0;
	int c = 0;
	char declare[MAX_DECLARE_LEN];

	len = get_declare_text(buff, declare, MAX_DECLARE_LEN);

	if (len >= MAX_DECLARE_LEN || len <= 4)
		return 0;

	easou_trans2lower(declare, declare);
	size = sizeof(exchange_link_key1) / sizeof(char*);

	for (int i = 0; i < size; i++)
	{
		p = declare;
		c = 0;
		while ((p = strstr(p, exchange_link_key1[i])) && c < 2)
		{
			p += strlen(exchange_link_key1[i]);

			if (!p)
				continue;

			size = sizeof(exchange_link_key2) / sizeof(char*);
			for (int j = 0; j < size; j++)
			{
				if ((q = strstr(p, exchange_link_key2[j])) && q - p <= 4)
				{
					return 1;
				}
			}

			c++;
		}
	}

	return 0;
}

static int start_visit_for_friendex(html_vnode_t *vnode, void *result)
{
	html_vnode_t **pvnode = (html_vnode_t**) result;
	html_tag_t *html_tag = &vnode->hpNode->html_tag;
	if (html_tag->tag_type == TAG_PURETEXT)
	{
		if (*pvnode == NULL)
		{
			if (is_exchange_link_declare(html_tag->text))
			{
				*pvnode = vnode;
				return VISIT_FINISH;
			}
		}
	}
	return VISIT_NORMAL;
}

static int find_declare_node(html_area_t *html_area, html_vnode_t **delcare_node)
{
	html_vnode_t *vnode = html_area->begin;
	int ret = VISIT_NORMAL;

	while (1)
	{
		if (vnode->hpNode->html_tag.tag_type != TAG_PURETEXT)
			ret = html_vnode_visit(vnode, start_visit_for_friendex, NULL, delcare_node);
		else
		{
			if (is_exchange_link_declare(vnode->hpNode->html_tag.text))
			{
				*delcare_node = vnode;
			}
		}
		if (ret == VISIT_FINISH)
			break;

		if (ret == VISIT_ERROR)
		{
			return -1;
		}
		if (vnode == html_area->end)
			break;
		vnode = vnode->nextNode;
	}
	if (*delcare_node != NULL)
		return 1;
	return 0;
}

static int is_friend_candi_area(html_area_t *html_area, int page_width, int page_height, int &candi_type)
{
	if (html_area->abspos_mark == PAGE_LEFT)
	{
		candi_type = 1; // friend vlink align line by line
		return 1;
	}
	if (html_area->abspos_mark == PAGE_FOOTER)
	{
		candi_type = 2; // align in one line
		return 1;
	}
	if (html_area->abspos_mark == PAGE_MAIN)
	{
		if (html_area->area_info.height <= 300 && html_area->area_info.ypos >= page_height / 2
				&& html_area->area_info.width * 2 >= page_width)
		{
			candi_type = 2; // align in one line
			return 1;
		}
		if (10 * html_area->area_info.xpos <= page_width && html_area->area_info.width * 2 < page_width)
		{
			candi_type = 1; //line by line
			return 1;
		}
	}

	// for those single main area
	if (html_area->area_info.ypos == 0)
	{
		if (10 * html_area->area_info.height > page_height * 9 && 10 * html_area->area_info.width > page_width * 7)
		{
			candi_type = 3; // maybe exist friend vlink like type 1 and 2
			return 1;
		}
	}

	return 0;
}

static int mark_friendex_type(html_area_t *area, vlink_t *vlink, int link_count, char *url, html_vnode_t *root,
		html_vnode_t **global_declare_node)
{
	if (area->abspos_mark == PAGE_INVALID)
		return 0;

	int candi_type = 0;
	if (!is_friend_candi_area(area, root->wx, root->hx, candi_type))
		return 0;

	html_vnode_t *vnode = NULL;
	if (!find_declare_node(area, &vnode))
	{
		if (*global_declare_node == NULL)
			return 0;
		vnode = *global_declare_node;
	}
	else
	{
		*global_declare_node = vnode;
	}

	mark_friendex_vlink(vlink, link_count, vnode);
	return 1;
}

int mark_linktype_friendex(lt_args_t *pargs, lt_res_t *pres)
{
	html_area_link_t area_link;
	memset(&area_link, 0, sizeof(html_area_link_t));
	html_vnode_t *declare_node = NULL;
	html_area_t *subArea = pargs->atree->root->subArea;
	if (!pargs->atree->root->subArea)
		subArea = pargs->atree->root;
	for (; subArea; subArea = subArea->nextArea)
	{
		get_area_link(subArea, pargs->vlink, pargs->link_count, &area_link);

		mark_friendex_type(area_link.html_area, area_link.vlink, area_link.link_count, pargs->url, pargs->root,
				&declare_node);
	}
	return 1;
}
