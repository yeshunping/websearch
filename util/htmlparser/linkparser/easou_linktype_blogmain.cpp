/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_blogmain.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_blogmain.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 
 * 	博客正文链接标记
 **/

#include "PageType.h"
#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_mark_parser.h"
#include "easou_mark_srctype.h"

#define LINK_SET_0 0x00
#define LINK_SET_X 0x01
#define LINK_SET_Y 0x02

static void is_link_set(vlink_t *vlink, int num)
{
	int i;
	for (i = 0; i < num; i++)
		vlink[i].inner.link_set = LINK_SET_0;

	int x[2];
	int y[2];
	int xMean, xVar;
	int yMean, yVar;
	for (i = 0; i < num - 2; i++)
	{
		x[0] = vlink[i + 1].inner.xpos - vlink[i].inner.xpos;
		y[0] = vlink[i + 1].inner.ypos - vlink[i].inner.ypos;
		x[1] = vlink[i + 2].inner.xpos - vlink[i + 1].inner.xpos;
		y[1] = vlink[i + 2].inner.ypos - vlink[i + 1].inner.ypos;

		xMean = (x[0] + x[1]) / 2;
		yMean = (y[0] + y[1]) / 2;

		xVar = ((x[0] - xMean) * (x[0] - xMean) + (x[1] - xMean) * (x[1] - xMean)) / 2;
		yVar = ((y[0] - yMean) * (y[0] - yMean) + (y[1] - yMean) * (y[1] - yMean)) / 2;

		if (xVar <= 4)
		{
			vlink[i].inner.link_set |= LINK_SET_X;
			vlink[i + 1].inner.link_set |= LINK_SET_X;
			vlink[i + 2].inner.link_set |= LINK_SET_X;
		}
		if (yVar <= 4)
		{
			vlink[i].inner.link_set |= LINK_SET_Y;
			vlink[i + 1].inner.link_set |= LINK_SET_Y;
			vlink[i + 2].inner.link_set |= LINK_SET_Y;
		}
	}
}

blogmain_res_t *blogmain_res_create()
{
	const char * where = "blogmain_res_create()";
	blogmain_res_t *pres = NULL;

	pres = (blogmain_res_t *) calloc(1, sizeof(blogmain_res_t));
	if (pres == NULL)
	{
		Error("calloc blogmain_res_t error. %s", where);
		blogmain_res_del(pres);
		return NULL;
	}
	pres->pparas = (extract_time_paras *) calloc(1, sizeof(extract_time_paras));
	if (pres->pparas == NULL)
	{
		Error("calloc extract_time_paras error. %s", where);
		blogmain_res_del(pres);
		return NULL;
	}
	if (init_etpara(pres->pparas) <= 0)
	{
		Error("init_etpara error. %s", where);
		blogmain_res_del(pres);
		return NULL;
	}

	return pres;
}

void blogmain_res_del(blogmain_res_t * pres)
{
	if (pres)
	{
		if (pres->pparas)
		{
			free_etpara(pres->pparas);
			free(pres->pparas);
			pres->pparas = NULL;
		}
		free(pres);
		pres = NULL;
	}
	return;
}

static int mark_blogmain(html_area_t *html_area, vlink_t *vlink, int link_count, extract_time_paras *pparas)
{
	char *p1 = NULL, *p2 = NULL;
	char *p3 = NULL, *p4 = NULL;
	int has_time_next = 0;
	int has_time_prev = 0;
	int ret;
	struct match_position mp;
	long dst_time;

	if (is_srctype_area(html_area, AREA_SRCTYPE_TEXT) && // 策略1:分块类型是文本
			html_area->abspos_mark == PAGE_MAIN) // 策略2:分块位置是MAIN
	{
		is_link_set(vlink, link_count);
		for (int i = 0; i < link_count; i++)
		{
			if (vlink[i].inner.vnode == NULL)
				continue;
			if (vlink[i].inner.link_set == LINK_SET_0 && // 策略3:链接不属于link集
					vlink[i].inner.is_goodlink) // 策略4:链接是有效链接(非内链)
			{
				int ct_len = 0;

				html_vnode_t *pnode = vlink[i].inner.vnode->nextNode;
				while (pnode != NULL && !pnode->isValid)
					pnode = pnode->nextNode;
				if (pnode != NULL)
				{
					ct_len = pnode->textSize;
					if (pnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
						p1 = pnode->hpNode->html_tag.text;
					else if (pnode->firstChild != NULL && pnode->firstChild->hpNode->html_tag.tag_type == TAG_PURETEXT)
						p1 = pnode->firstChild->hpNode->html_tag.text;

					if (pnode->nextNode != NULL && pnode->nextNode->isValid)
					{
						ct_len += pnode->nextNode->textSize;
						if (pnode->nextNode->hpNode->html_tag.tag_type == TAG_PURETEXT)
							p2 = pnode->nextNode->hpNode->html_tag.text;
						else if (pnode->nextNode->firstChild != NULL
								&& pnode->nextNode->firstChild->hpNode->html_tag.tag_type == TAG_PURETEXT)
							p2 = pnode->nextNode->firstChild->hpNode->html_tag.text;
					}

					if (p1 != NULL)
					{
						ret = extract_time_from_str(&dst_time, &mp, p1, pparas, 0, 0);
						if (ret > 0)
							has_time_next = 1;
					}

					if (has_time_next == 0 && p2 != NULL)
					{
						ret = extract_time_from_str(&dst_time, &mp, p2, pparas, 0, 0);
						if (ret > 0)
							has_time_next = 1;
					}
				}

				pnode = vlink[i].inner.vnode->prevNode;
				while (pnode != NULL && !pnode->isValid)
					pnode = pnode->prevNode;
				if (pnode != NULL)
				{
					ct_len += pnode->textSize;
					if (pnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
						p3 = pnode->hpNode->html_tag.text;
					else if (pnode->firstChild != NULL && pnode->firstChild->hpNode->html_tag.tag_type == TAG_PURETEXT)
						p3 = pnode->firstChild->hpNode->html_tag.text;
					if (pnode->prevNode != NULL && pnode->prevNode->isValid)
					{
						ct_len += pnode->prevNode->textSize;
						if (pnode->prevNode->hpNode->html_tag.tag_type == TAG_PURETEXT)
							p4 = pnode->prevNode->hpNode->html_tag.text;
						else if (pnode->prevNode->firstChild != NULL
								&& pnode->prevNode->firstChild->hpNode->html_tag.tag_type == TAG_PURETEXT)
							p4 = pnode->prevNode->firstChild->hpNode->html_tag.text;
					}
					if (p3 != NULL)
					{
						ret = extract_time_from_str(&dst_time, &mp, p3, pparas, 0, 0);
						if (ret > 0)
							has_time_prev = 1;
					}

					if (has_time_prev == 0 && p4 != NULL)
					{
						ret = extract_time_from_str(&dst_time, &mp, p4, pparas, 0, 0);
						if (ret > 0)
							has_time_prev = 1;
					}
				}
				if (ct_len > 0 && // 策略5:链接上下4兄弟节点有文本内容
						!has_time_next && !has_time_prev) // 策略6:前后两个节点中无时间串
				{
					vlink[i].linkFunc |= VLINK_BLOG_MAIN; // DONE!!
				}
			}
		}
	}
	return 1;
}

int mark_linktype_blogmain(lt_args_t *pargs, lt_res_t *pres)
{
	if (!PT_IS_BLOG(pargs->pagetype))
	{
		return 0;
	}

	blogmain_res_t *pres_bm = pres->res_blogmain;
	html_area_link_t area_link;
	memset(&area_link, 0, sizeof(html_area_link_t));

	html_area_t *subArea = pargs->atree->root->subArea;
	if (!pargs->atree->root->subArea)
		subArea = pargs->atree->root;

	for (; subArea; subArea = subArea->nextArea)
	{
		get_area_link(subArea, pargs->vlink, pargs->link_count, &area_link);

		mark_blogmain(area_link.html_area, area_link.vlink, area_link.link_count, pres_bm->pparas);
	}
	return 1;
}

