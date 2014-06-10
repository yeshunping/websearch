/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_nav.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_nav.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记导航栏链接
 **/

#include "easou_link_mark.h"
#include "easou_mark_parser.h"
#include "easou_link_common.h"
#include "easou_vhtml_parser.h"
#include "easou_mark_func.h"
#include "html_text_utils.h"
#include <ctype.h>

static int is_nav_candi_area(html_area_t *html_area, int page_width, int page_height)
{
	if (html_area->abspos_mark == PAGE_HEADER)
		return 1;
	if (html_area->abspos_mark == PAGE_MAIN)
	{
		if (html_area->area_info.height <= 200 && html_area->area_info.ypos <= 500
				&& html_area->area_info.width * 2 >= page_width) //&& page_height>=600)
			return 1;
	}

	// for those single main area
	if (html_area->area_info.ypos == 0)
	{
		if (10 * html_area->area_info.height > page_height * 9 && 10 * html_area->area_info.width > page_width * 7)
			return 1;
	}

	return 0;
}

	/* TODO
static int start_check_my_position(html_tag_t *html_tag, void *result)
{
	if (html_tag->tag_type != TAG_PURETEXT)
		return VISIT_NORMAL;

	 int *is_position = (int *) result;
	 char mytext[256] = "";
	 if (html_derefrence_text(mytext, 0, 255, html_tag->text) > 0)
	 {
	 if (strchr(mytext, '>'))
	 {
	 *is_position = 1;
	 return VISIT_FINISH;
	 }
	 if (strstr(mytext, "＞"))
	 {
	 *is_position = 1;
	 return VISIT_FINISH;
	 }
	 if (strstr(mytext, "→"))
	 {
	 *is_position = 1;
	 return VISIT_FINISH;
	 }
	 }

	return VISIT_NORMAL;
}
	 */

static char *bbs_nav_text[] =
{ "登陆", "登录", "注册", "排行", "搜索", "帮助", "退出", "返回", "发表", "收藏", "打印", "好友", "会员", "推荐", "留言", "社区", "回复", "跟贴", "关闭",
		"引用", "书签", "精华", "帖子", "热贴", "作者", "列表", "告诉", "更新", "我要", "查看", "评论", "查询", "上传", "阅读", "进入", "删除", "全文",
		"联系", "访问", "投票", "浏览", "检索", "讨论", "日历", "中心", "管理", "论坛", "我们", "积分", "统计", "铃声", "发送", "点击", "分类", "更多",
		"加入", "编辑", "股票", "声明", "上一", "下一", "前一", "后一", "第", "search", "list", "calendar", "blog", "view", "post",
		"click", "user", "login", "help", NULL };

static int is_in_filter4nav(const char *text)
{
	int i = 0;
	while (bbs_nav_text[i])
	{
		if (strstr(text, bbs_nav_text[i]))
			return 1;
		i++;
	}
	return 0;
}

// get next node in html tree according to preorder
/*
static html_node_t *get_next_hp2node(html_node_t *node)
{
	assert(node);
	if (node->child)
		return node->child;

	while (node)
	{
		if (node->next)
			return node->next;
		if (node->html_tag.tag_type == TAG_ROOT)
			return NULL;
		node = node->parent;
	}

	return NULL;
}
*/

// Function : visit node in html tree, between from_node and to_node;
// Notice : the from_node and to_node do NOT visit;
// Input : from_node, which has visited
// 	   to_node, the end node
// 	   start_visit, the real visit function
// 	   result, visit parameter
/*
static int html_node_visit_from_to(html_node_t *from_node, html_node_t *to_node,
		int (*start_visit)(html_tag_t *, void *), void *result)
{
	assert(from_node);
	assert(from_node->html_tag.tag_code < to_node->html_tag.tag_code);
	assert(start_visit);

	// get first node to visit	
	html_node_t *node = from_node->next;
	while (node == NULL)
	{
		if (from_node->html_tag.tag_type == TAG_ROOT)
			return VISIT_ERROR;
		from_node = from_node->parent;
		node = from_node->next;
	}

	int ret = VISIT_NORMAL;
	// visit node one by one
	do
	{
		if (node->html_tag.tag_code >= to_node->html_tag.tag_code)
			return VISIT_FINISH;
		ret = (*start_visit)(&node->html_tag, result);
		if ((ret == VISIT_FINISH) || (ret == VISIT_ERROR))
			return ret;
		node = get_next_hp2node(node);
	} while (node);

	return VISIT_ERROR;
}
*/

static int is_page_number_vlink(vlink_t *vlink, int begin_sn, int end_sn)
{
	int pn_count = 0;
	unsigned long pn = 0;
	int i = 0;
	int j = 0;
	char *next = NULL;
	for (i = begin_sn; i <= end_sn; i++)
	{
		j = 0;
		next = NULL;
		pn = 0;
		if (vlink[i].inner.tag_type == TAG_IMG)
			continue;
		while (isspace(vlink[i].text[j]) || (vlink[i].text[j] == '['))
			j++;
		if (vlink[i].text[j] == '\0')
			continue;
		pn = strtoul(vlink[i].text + j, &next, 10);
		if (next && next - vlink[i].text - j > 0)
		{
			while (isspace(*next) || (*next == ']'))
				next++;
			if (*next == '\0')
			{
				pn_count++;
				if (pn_count >= 2)
					return 1;
			}
		}
	}
	return 0;
}

// Function : check to see if "my position" vlink, such as :
// "您现在的位置： 文秘写作网 >> 文章中心 >> 心得体会 >> 学习体会 >> 正文"
/* TODO
static int is_my_position_vlink(vlink_t *vlink, int begin_sn, int end_sn)
{
	int is_position = 0;
	vlink_t *from_vlink = NULL;
	vlink_t *to_vlink = NULL;

	for (int i = begin_sn; i <= end_sn; i++)
	{
		if ((vlink[i].inner.tag_type != TAG_IMG) && !(vlink[i].linkFunc & VLINK_INVALID))
		{
			if (!from_vlink)
			{
				from_vlink = vlink + i;
			}
			else
			{
				to_vlink = vlink + i;
				if (from_vlink->inner.node->html_tag.tag_code > to_vlink->inner.node->html_tag.tag_code)
				{
					//in case that dom tree tag code not strictly increase by preorder traverse
					from_vlink = to_vlink;
					to_vlink = NULL;
				}
			}
		}
		if (from_vlink && to_vlink)
		{
			is_position = 0;
			html_node_visit_from_to(from_vlink->inner.node, to_vlink->inner.node, start_check_my_position,
					&is_position);
			if (is_position)
				return 1;

			from_vlink = NULL;
			to_vlink = NULL;
		}
	}
	return 0;
}
*/

// Pre-required : vlink's text is GBK coded
static int is_bbs_other_vlink(vlink_t *vlink, int begin_sn, int end_sn)
{
	int nav_count = 0;
	int bbs_nav_count = 0;

	for (int i = begin_sn; i <= end_sn; i++)
	{
		if ((vlink[i].inner.tag_type != TAG_IMG) && !(vlink[i].linkFunc & VLINK_INVALID))
		{
			nav_count++;
			if (vlink[i].inner.anchor_from_alt == 0 && is_in_filter4nav(vlink[i].text))
				bbs_nav_count++;
		}
	}

	if (bbs_nav_count >= nav_count / 2)
		return 1;

	return 0;
}

// Function : check to see if valid nav vlink
// Return : 1, valid; 0, invalid
static int check_valid_nav_vlink(vlink_t *vlink, int begin_sn, int end_sn)
{
	if (is_page_number_vlink(vlink, begin_sn, end_sn))
		return 0;
	/* TODO
	if (is_my_position_vlink(vlink, begin_sn, end_sn))
		return 0;
	*/
	if (is_in_func_area(vlink->inner.vnode->hp_area, AREA_FUNC_MYPOS))
	{
		return 0;
	}
	if (is_bbs_other_vlink(vlink, begin_sn, end_sn))
		return 0;

	return 1;
}

// Input : num, available vlink number of this area
static void mark_nav_vlink(vlink_t *vlink, int num, char *base_url)
{
	int sn = 0; // current check vlink serial no
	int begin_sn = 0;
	int end_sn = 0;

	int ypos = 0;
	int xborder = 0;
	int total_wx = 0;
	//int average_wx = 0;
	int invalid_url = 0;

	char base_trunk[UL_MAX_SITE_LEN] = "";
	if (!get_trunk_from_url(base_url, base_trunk))
		return;

	int step = 1;
	int total_nav = 0;
	while (sn < num)
	{
		total_wx = 0;
		total_nav = 0;
		invalid_url = 0;

		if (sn + 3 >= num)
			break;
		if (vlink[sn].inner.tag_type != TAG_A)
		{
			sn++;
			continue;
		}
		total_wx += vlink[sn].inner.width;
		total_nav++;
		if (vlink[sn].inner.width > 90)
		{ // 6 Chinese words
			sn++;
			continue;
		}
		if (!check_url_trunk(vlink[sn].url, base_trunk))
			invalid_url++;
		ypos = vlink[sn].inner.ypos;

		step = 1;
		// sn+1 vlink
		xborder = vlink[sn].inner.xpos + vlink[sn].inner.width;
		if ((vlink[sn + 1].inner.tag_type == TAG_IMG) && (vlink[sn + 1].inner.width <= 45)
				&& (vlink[sn + 1].inner.xpos >= xborder))
			step = 2; // small <img> seperator

		if (sn + step >= num)
			break;
		if (vlink[sn + step].inner.tag_type != TAG_A)
		{
			sn++;
			continue;
		}
		total_wx += vlink[sn + step].inner.width;
		total_nav++;
		if (vlink[sn + step].inner.width > 90)
		{ // 6 Chinese words
			sn++;
			continue;
		}
		//xborder = vlink[sn].xpos+vlink[sn].wx;
		if (vlink[sn + step].inner.ypos != ypos)
		{
			sn++;
			continue;
		}
		if (vlink[sn + step].inner.xpos - xborder > 45)
		{
			sn++;
			continue;
		}
		if (!check_url_trunk(vlink[sn + step].url, base_trunk))
			invalid_url++;

		// sn+2 vlink
		xborder = vlink[sn + step].inner.xpos + vlink[sn + step].inner.width;
		if (sn + step + 1 >= num)
			break;
		if ((vlink[sn + step + 1].inner.tag_type == TAG_IMG) && (vlink[sn + step + 1].inner.width <= 45)
				&& (vlink[sn + step + 1].inner.xpos >= xborder))
			step += 2; // small <img> seperator
		else
			step += 1;

		if (sn + step >= num)
			break;
		if (vlink[sn + step].inner.tag_type != TAG_A)
		{
			sn += 2;
			continue;
		}
		total_wx += vlink[sn + step].inner.width;
		total_nav++;
		if (vlink[sn + step].inner.width > 90)
		{ // 6 Chinese words
			sn += 2;
			continue;
		}
		//xborder = vlink[sn+1].xpos+vlink[sn+1].wx;
		if (vlink[sn + step].inner.ypos != ypos)
		{
			sn += 2;
			continue;
		}
		if (vlink[sn + step].inner.xpos - xborder > 45)
		{
			sn += 2;
			continue;
		}
		if (!check_url_trunk(vlink[sn + step].url, base_trunk))
			invalid_url++;

		// sn+3 vlink
		xborder = vlink[sn + step].inner.xpos + vlink[sn + step].inner.width;
		if (sn + step + 1 >= num)
			break;
		if ((vlink[sn + step + 1].inner.tag_type == TAG_IMG) && (vlink[sn + step + 1].inner.width <= 45)
				&& (vlink[sn + step + 1].inner.xpos >= xborder))
			step += 2; // small <img> seperator
		else
			step += 1;

		if (sn + step >= num)
			break;
		if (vlink[sn + step].inner.tag_type != TAG_A)
		{
			sn += 3;
			continue;
		}

		total_wx += vlink[sn + step].inner.width;
		total_nav++;
		if (vlink[sn + step].inner.width > 90)
		{ // 6 Chinese words
			sn += 3;
			continue;
		}
		//xborder = vlink[sn+2].xpos+vlink[sn+2].wx;
		if (vlink[sn + step].inner.ypos != ypos)
		{
			sn += 3;
			continue;
		}
		if (vlink[sn + step].inner.xpos - xborder > 45)
		{
			sn += 3;
			continue;
		}
		if (!check_url_trunk(vlink[sn + step].url, base_trunk))
			invalid_url++;

		// when here, find vlink of nav func
		begin_sn = sn;
		end_sn = sn + step;

		// go on to look for the vlinks similar to the before 4 vlinks
		step = 1;
		while (end_sn + 1 < num)
		{
			xborder = vlink[end_sn].inner.xpos + vlink[end_sn].inner.width;
			if ((vlink[end_sn + 1].inner.tag_type == TAG_IMG) && (vlink[end_sn + 1].inner.width <= 45)
					&& (vlink[end_sn + 1].inner.xpos >= xborder))
				step = 2; // small <img> seperator
			else
				step = 1;

			if (end_sn + step >= num)
				break;
			if (vlink[end_sn + step].inner.tag_type != TAG_A)
				break;
			if (vlink[end_sn + step].inner.width > 90) // 6 Chinese words
				break;
			if (vlink[end_sn + step].inner.ypos != ypos)
				break;
			//xborder = vlink[end_sn].xpos+vlink[end_sn].wx;
			if (vlink[end_sn + step].inner.xpos - xborder > 45)
				break;
			if (!check_url_trunk(vlink[end_sn + step].url, base_trunk))
				invalid_url++;

			end_sn += step;
			total_wx += vlink[end_sn].inner.width;
			total_nav++;
		}

		// check if too many invaid urls (point to site outside)
		if (invalid_url >= total_nav / 2)
		{
			sn = end_sn + 1; // go on for next line vlinks
			continue;
		}

		if (!check_valid_nav_vlink(vlink, begin_sn, end_sn))
		{
			sn = end_sn + 1;
			continue;
		}

		// mark the vlinks from begin_sn to end_sn as NAV
		for (int i = begin_sn; i <= end_sn; i++)
		{
			if ((vlink[i].inner.tag_type != TAG_IMG) && !(vlink[i].linkFunc & VLINK_INVALID))
				vlink[i].linkFunc |= VLINK_NAV;
		}

		sn = end_sn + 1;
	} // end for while	

	return;
}

/**
 * @brief 标记导航信息
 */
static int mark_nav_type(html_area_t *html_area, vlink_t *vlink, int link_count, char *url, html_vnode_t *root)
{
	if (link_count == 0)
		return 0;

	if (html_area->abspos_mark == PAGE_INVALID)
		return 0;

	if (!is_nav_candi_area(html_area, root->wx, root->hx))
		return 0;

	mark_nav_vlink(vlink, link_count, url);
	return 1;
}

/**
 * @brief 标记导航链接
 */
int mark_linktype_nav(lt_args_t *pargs, lt_res_t *pres)
{
	html_area_link_t area_link;
	memset(&area_link, 0, sizeof(html_area_link_t));

	html_area_t *subArea = pargs->atree->root->subArea;
	if (!pargs->atree->root->subArea)
		subArea = pargs->atree->root;
	for (; subArea; subArea = subArea->nextArea)
	{
		get_area_link(subArea, pargs->vlink, pargs->link_count, &area_link);

		mark_nav_type(area_link.html_area, area_link.vlink, area_link.link_count, pargs->url, pargs->root);
	}
	//通过分快标注补充
	const area_list_t *area_list = get_func_mark_result(pargs->atree, AREA_FUNC_NAV);
	if (!area_list)
		return 1;

	for (int i = 0; i < pargs->link_count; i++)
	{
		if (pargs->vlink[i].inner.vnode == NULL)
			continue;
		if (in_area(&pargs->vlink[i].inner, area_list))
		{
			if ((pargs->vlink[i].inner.tag_type != TAG_IMG) && !(pargs->vlink[i].linkFunc & VLINK_INVALID))
				pargs->vlink[i].linkFunc |= VLINK_NAV;
		}
	}

	return 1;
}
