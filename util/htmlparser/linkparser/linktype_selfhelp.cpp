/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_selfhelp.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_selfhelp.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记友情自助链接
 **/

#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_vhtml_parser.h"
#include "html_text_utils.h"
#include <ctype.h>

#define PAGE_NULL 0x0
#define PAGE_SELFHELP 0x1
#define PAGE_INDEX 0x2
#define MAX_FORBID_COUNT 5

#define SELFHELP_MATCH_TITLE 0x01
#define SELFHELP_MATCH_CONT 0x02
#define SELFHELP_MATCH_SPEC 0x04

#define VLINK_TEMP 0x80000000

static const char * selfhelp_link_key[] =
{ "申请自助链接", "申请友情链接", "自助友情链接", "自助式友情链接", "自助式链接", "自助链接", "交换友情链接", "链接交换", "交换链接", "免费链接", "合作链接", "互换链接", "友情链接",
		"链接" };
static const int selfhelp_link_key_len[] =
{ 12, 12, 12, 14, 10, 8, 12, 8, 8, 8, 8, 8, 8, 4 };
static const int selfhelp_link_key_size = sizeof(selfhelp_link_key) / sizeof(char*);

static const char * selfhelp_link_key2[] =
{ "点入排行", "点出排行", "点入点出统计排行", "点入统计排行", "点出统计排行" };
static const int selfhelp_link_key2_len[] =
{ 8, 8, 16, 12, 12 };
static const int selfhelp_link_key2_size = sizeof(selfhelp_link_key2) / sizeof(char*);

static const char * selfhelp_link_forbid4title[] =
{ "下载", "版", };
static const int selfhelp_link_forbid4title_len[] =
{ 4, 2 };
static const int selfhelp_link_forbid4title_size = sizeof(selfhelp_link_forbid4title_size) / sizeof(char *);

static const char * selfhelp_link_forbid4page[] =
{
// for filter software download and bbs
		"链接系统", "下载",

		// for filter bbs
		"上一主题", "下一主题", "收藏主题", "回复主题", "发表主题" };
static const int selfhelp_link_forbid4page_len[] =
{ 8, 4, 8, 8, 8, 8, 8 };
static const int selfhelp_link_forbid4page_size = sizeof(selfhelp_link_forbid4page) / sizeof(char*);

typedef struct _selfhelp_info_t
{
	char title_begin;
	char anchor_begin;
	char is_selfhelp;

	u_int page_width;
	u_int page_height;

	u_int ct_len; //main block
	u_int at_len; //main block
	u_int total_ct_len;
	u_int total_at_len;
	u_int anchor_cnt; //main block
	u_int total_anchor_cnt;
	u_int forbid_match_cnt;
	u_int invalid_link_cnt; // continous invalid link count

	html_area_abspos_mark_t cur_mark; // current block mark
	u_int key_match_cnt; // key match count 

	int prev_ypos; // prev invalid link ypos
	int prev_hx; // prev invalid link hx
} selfhelp_info_t;

static int is_valid_selfhelp_page_title(char * title, selfhelp_info_t * pinfo)
{
	int ret = 0;
	char * p = 0;

	for (int i = 0; i < selfhelp_link_key_size - 2; i++)
	{
		if ((p = strstr(title, selfhelp_link_key[i])))
		{
			for (int j = 0; j < selfhelp_link_forbid4title_size; j++)
			{
				if (strstr(p + selfhelp_link_key_len[i], selfhelp_link_forbid4title[j]))
				{
					ret = -1;
					return ret;
				}
			}
			ret = 1;
			break;
		}
	}

	return ret;
}

static int linknode_comp4table(html_node_t * nd1, html_node_t * nd2)
{
	if (!nd1 || !nd2)
		return 0;

	int depth = 2;

	html_node_t * tmpnd1 = nd1;
	html_node_t * tmpnd2 = nd2;

	if (tmpnd1->parent && tmpnd1->parent->html_tag.tag_type == TAG_TD && tmpnd2->parent
			&& tmpnd2->parent->html_tag.tag_type == TAG_TD)
	{
		tmpnd1 = tmpnd1->parent;
		tmpnd2 = tmpnd2->parent;
		if (tmpnd1->parent && tmpnd1->parent->html_tag.tag_type == TAG_TR && tmpnd2->parent
				&& tmpnd2->parent->html_tag.tag_type == TAG_TR)
		{
			tmpnd1 = tmpnd1->parent;
			tmpnd2 = tmpnd2->parent;

			for (int i = 0; i < depth; i++)
			{
				tmpnd1 = tmpnd1->parent;
				tmpnd2 = tmpnd2->parent;
				if (tmpnd1 && tmpnd2 && tmpnd1->parent && tmpnd2->parent
						&& tmpnd1->parent->html_tag.tag_type == tmpnd2->parent->html_tag.tag_type)
				{
					if (tmpnd1->parent->html_tag.tag_type == TAG_TABLE)
						return 1;
					continue;
				}
				else
					break;
			}
		}
	}

	return 0;
}

static int finish_visit_for_selfhelp(html_vnode_t *vnode, void *result)
{
	selfhelp_info_t * info = (selfhelp_info_t*) result;

	if (vnode->hpNode->html_tag.tag_type == TAG_TITLE)
		info->title_begin = 0;
	else if (vnode->hpNode->html_tag.tag_type == TAG_A)
		info->anchor_begin = 0;

	return VISIT_NORMAL;
}

static int start_visit_for_selfhelp(html_vnode_t *vnode, void *result)
{
	selfhelp_info_t * info = (selfhelp_info_t*) result;
	html_node_t * node = vnode->hpNode;
	int ret = 0;
	int len = 0;
	int pos = 0;
	char * p = NULL;
	char buff[128];

	if (node->html_tag.tag_type == TAG_A)
	{
		info->anchor_begin = 1;
		return VISIT_NORMAL;
	}
	if (node->html_tag.tag_type == TAG_TITLE)
	{
		info->title_begin = 1;
		return VISIT_NORMAL;
	}

	if (node->html_tag.tag_type == TAG_PURETEXT)
	{
		len = vnode->textSize > 0 ? vnode->textSize : 0;
		if (vnode->isValid)
		{
			if (info->cur_mark == PAGE_MAIN)
				info->ct_len += len;
			info->total_ct_len += len;
			if (info->anchor_begin)
			{
				if (info->cur_mark == PAGE_MAIN)
				{
					info->at_len += len;
					info->anchor_cnt++;

					if (len >= 26)
					{
						if (info->prev_ypos >= 0 && abs(vnode->ypos - info->prev_ypos - info->prev_hx) <= 15)
						{
							info->invalid_link_cnt++;
							if (info->invalid_link_cnt >= 8)
							{
								info->is_selfhelp = 0;
								return VISIT_FINISH;
							}
						}
						else
							info->invalid_link_cnt = 1;

						info->prev_ypos = vnode->ypos;
						info->prev_hx = vnode->hx;
					}
					else
					{
						info->invalid_link_cnt = 0;
						info->prev_ypos = -1;
						info->prev_hx = -1;
					}

					info->total_at_len += len;
					info->total_anchor_cnt++;
				}
			}
		}

		if (info->title_begin && !(info->is_selfhelp & SELFHELP_MATCH_TITLE))
		{
			ret = is_valid_selfhelp_page_title(node->html_tag.text, info);
			if (ret)
			{
				if (ret == 1)
				{
					info->is_selfhelp |= SELFHELP_MATCH_TITLE;
					return VISIT_NORMAL;
				}
				else
				{
					info->is_selfhelp = 0;
					return VISIT_FINISH;
				}
			}
		}

		if (vnode->isValid && vnode->textSize < 64)
		{
			len = copy_html_text(buff, 0, sizeof(buff), node->html_tag.text);

			if (strstr(buff, selfhelp_link_key[selfhelp_link_key_size - 1]))
				info->key_match_cnt++;
			//for the subtitle less the 24 hanzi
			if (info->is_selfhelp & SELFHELP_MATCH_TITLE)
			{
				for (int i = 0; i < selfhelp_link_key2_size; i++)
				{
					if (strstr(buff, selfhelp_link_key2[i]))
					{
						info->is_selfhelp |= SELFHELP_MATCH_SPEC;
						return VISIT_FINISH;
					}
				}
			}

			for (int i = 0; i < selfhelp_link_forbid4page_size; i++)
			{
				if (strstr(buff, selfhelp_link_forbid4page[i]))
				{
					if (i < 2)
						info->forbid_match_cnt++;
					if ((i >= 2 && info->anchor_begin) // bbs
					|| info->forbid_match_cnt >= MAX_FORBID_COUNT)
					{
						info->is_selfhelp = 0;
						return VISIT_FINISH;
					}
				}
			}

			if (!(info->is_selfhelp & SELFHELP_MATCH_CONT) && !info->anchor_begin && vnode->ypos < 500
					&& 2 * vnode->ypos < (int) info->page_height)
			{
				for (int i = 0; i < selfhelp_link_key_size - 7; i++)
				{
					if ((p = strstr(buff, selfhelp_link_key[i]))
							&& ((pos = p - buff) < 4 || pos + selfhelp_link_key_len[i] > len - 4)) // the key should be in border
					{
						info->is_selfhelp |= SELFHELP_MATCH_CONT;
						return VISIT_NORMAL;
					}
				}
			}
		}
	}

	return VISIT_NORMAL;
}

static int is_selfhelp_link_page(html_vnode_t *root, area_tree_t *atree)
{
	selfhelp_info_t shinfo;
	memset(&shinfo, 0, sizeof(selfhelp_info_t));
	html_vnode_t * vnode = NULL;
	int ret = 0;
	int bend = 0;

	shinfo.page_width = root->wx;
	shinfo.page_height = root->hx;
	shinfo.prev_ypos = -1;
	shinfo.prev_hx = -1;

	//small page
	if (shinfo.page_width < 60 || shinfo.page_height < 60)
		return PAGE_NULL;

	for (html_area_t *subArea = atree->root->subArea; subArea; subArea = subArea->nextArea)
	{
		vnode = subArea->begin;
		shinfo.cur_mark = subArea->abspos_mark;
		while (vnode)
		{
			ret = html_vnode_visit(vnode, start_visit_for_selfhelp, finish_visit_for_selfhelp, &shinfo);
			if (ret == VISIT_ERROR)
				return -1; // failed

			if (ret == VISIT_FINISH)
			{
				bend = 1;
				break;
			}
			if (vnode == subArea->end)
				break;
			vnode = vnode->nextNode;
		}
		if (bend)
			break;
	}

	if (shinfo.is_selfhelp)
	{
		if (!(shinfo.is_selfhelp & SELFHELP_MATCH_SPEC) && shinfo.key_match_cnt == 0)
			return PAGE_NULL;
		if (10 * shinfo.total_at_len >= 8 * shinfo.total_ct_len && shinfo.ct_len < 128) // index page
			return PAGE_SELFHELP | PAGE_INDEX;
		if (!(shinfo.is_selfhelp & SELFHELP_MATCH_SPEC)
				&& (16 * shinfo.at_len < shinfo.ct_len || shinfo.anchor_cnt < 8))
			return PAGE_NULL;
		return PAGE_SELFHELP;
	}
	return PAGE_NULL;
}

static int is_selfhelp_candi_area(html_area_t *html_area, int page_width, int page_height)
{
	if (html_area->abspos_mark == PAGE_MAIN && html_area->area_info.width * 3 > page_width)
		return 1; // line style

	return 0;
}

static void mark_selfhelp_vlink(vlink_t *vlink, int num, char *base_url)
{
	int sn = 0;
	int step = 0;
	int prev_ypos = 0;
	int prev_hx = 0;
	int prev_xborder = 0;
	int prev_xpos = 0;
	html_node_t * prev_node = NULL;
	char base_trunk[UL_MAX_SITE_LEN] = "";
	char prev_trunk[UL_MAX_SITE_LEN] = "";
	int invalid_cnt = 0;
	int total_cnt = 0;
	int cnt = 0;
	char * p = NULL;
	int valid_anchor = 1;
	//html_node_t * node = NULL;

	if (!get_trunk_from_url(base_url, base_trunk))
		return;

	while (sn < num)
	{
		if (sn + 3 >= num)
			break;

		step = 0;
		prev_ypos = -1;
		prev_hx = 0;
		prev_xborder = -1;
		prev_xpos = -1;
		prev_node = NULL;
		invalid_cnt = 0;
		total_cnt = 0;
		cnt = -1;
		valid_anchor = 1;
		strcpy(prev_trunk, base_trunk);

		while (step < num)
		{
			if (sn + step >= num)
				break;
			valid_anchor = 1;

			if (vlink[sn+step].inner.vnode == NULL)
			{
				step++;
				continue;
			}
			if ((vlink[sn + step].linkFunc & VLINK_INVALID) || vlink[sn + step].inner.tag_type == TAG_IMG
					|| vlink[sn + step].inner.tag_type == TAG_EMBED)
			{
				step++;
				continue;
			}

			if (vlink[sn + step].inner.tag_type != TAG_A)
				break;

			if (vlink[sn + step].inner.width > 240) //no less than 16 hanzi
				break;

			if (check_url_trunk(vlink[sn + step].url, base_trunk))
			{
				if (cnt < 0)
					cnt = 1;
				else if (cnt > 3)
					break;
				else
					cnt++;

				vlink[sn + step].linkFunc |= VLINK_TEMP;
				invalid_cnt++;
				step++;
				total_cnt++;
				continue;
			}

			if (check_url_trunk(vlink[sn + step].url, prev_trunk))
			{
				if (cnt < 0)
					cnt = 1;
				else if (cnt > 3)
					break;
				else
					cnt++;

				prev_ypos = vlink[sn + step].inner.ypos;
				prev_xborder = vlink[sn + step].inner.xpos + vlink[sn + step].inner.width;
				prev_hx = vlink[sn + step].inner.height;
				prev_xpos = vlink[sn + step].inner.xpos;
				prev_node = vlink[sn + step].inner.node;

				invalid_cnt++;
				step++;
				total_cnt++;
				continue;
			}

			cnt = -1;

			for (int i = 0; i < selfhelp_link_key_size; i++)
			{
				if ((p = strstr(vlink[sn + step].text, selfhelp_link_key[i])))
				{
					valid_anchor = 0;
					break;
				}
			}

			if (!valid_anchor)
				break;

			if (prev_ypos >= 0)
			{
				if (vlink[sn + step].inner.ypos != prev_ypos)
				{
					if (vlink[sn + step].inner.ypos - prev_hx - prev_ypos > 30)
					{
						if (abs(prev_xpos - vlink[sn + step].inner.xpos) > 15
								|| vlink[sn + step].inner.ypos - prev_hx - prev_ypos > 60
								|| !linknode_comp4table(vlink[sn + step].inner.node, prev_node))
							break;
					}
				}
				else if (prev_xborder >= 0)
				{
					if (vlink[sn + step].inner.xpos - prev_xborder > 60)
						break;
				}
			}

			prev_ypos = vlink[sn + step].inner.ypos;
			prev_xborder = vlink[sn + step].inner.xpos + vlink[sn + step].inner.width;
			prev_hx = vlink[sn + step].inner.height;
			prev_xpos = vlink[sn + step].inner.xpos;
			prev_node = vlink[sn + step].inner.node;
			get_trunk_from_url(vlink[sn + step].url, prev_trunk);
			step++;
			total_cnt++;
		}

		if (step < 4)
		{
			if (step == 0)
				sn++;
			else
				sn += step;
			continue;
		}

		if (total_cnt - invalid_cnt >= 10 || (total_cnt >= 10 && 10 * invalid_cnt <= total_cnt))
		{
			for (int i = sn; i < sn + step; i++)
			{
				if ((vlink[i].inner.tag_type == TAG_A) && !(vlink[i].linkFunc & VLINK_INVALID)
						&& !(vlink[i].linkFunc & VLINK_TEMP))
					vlink[i].linkFunc |= VLINK_SELFHELP;
			}
		}

		sn = sn + step + 1;
	}

	for (int i = 0; i < num; i++)
		vlink[i].linkFunc &= ~VLINK_TEMP;

	return;
}

static int mark_selfhelp_type(html_area_t *html_area, vlink_t *vlink, int link_count, char *url, html_vnode_t *root)
{
	if (link_count == 0)
		return 0;

	if (html_area->abspos_mark == PAGE_INVALID)
		return 0;

	if (!is_selfhelp_candi_area(html_area, root->wx, root->hx))
		return 0;

	mark_selfhelp_vlink(vlink, link_count, url);
	return 1;
}

int mark_linktype_selfhelp(lt_args_t *pargs, lt_res_t *pres)
{
	html_area_link_t area_link;
	memset(&area_link, 0, sizeof(html_area_link_t));

	if (is_selfhelp_link_page(pargs->root, pargs->atree) == PAGE_NULL)
		return 0;

	html_area_t *subArea = pargs->atree->root->subArea;
	if (!pargs->atree->root->subArea)
		subArea = pargs->atree->root;
	for (; subArea; subArea = subArea->nextArea)
	{
		get_area_link(subArea, pargs->vlink, pargs->link_count, &area_link);

		mark_selfhelp_type(area_link.html_area, area_link.vlink, area_link.link_count, pargs->url, pargs->root);
	}
	return 1;
}
