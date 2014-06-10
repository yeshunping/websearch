/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_friend.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_friend.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记友情链接
 **/

#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_vhtml_parser.h"
#include "easou_mark_parser.h"
#include "easou_mark_func.h"
#include "html_text_utils.h"
#include <ctype.h>

/**
 * @brief 是否友情链接组
 */
static bool is_friend_group(group_link_t *pgl)
{
	if (pgl->homepage_count > (pgl->inner_count - 1) && pgl->outer_count > pgl->inner_count && pgl->homepage_count >= 2
			&& pgl->outer_count >= 3 && pgl->link_count * 10 > pgl->text_len)
		return true;
	return false;
}

/**
 * @brief 通过分组信息标记友情链接
 */
int mark_linktype_friend_by_group(lt_args_t *pargs, lt_res_t *pvres)
{
	int link_count = pargs->link_count;
	vlink_t *vlink = pargs->vlink;

	for (int i = 0; i < pvres->group.group_num; i++)
	{
		group_link_t *pgl = &pvres->group.groups[i];
		if (is_friend_group(pgl))
		{
			for (int j = 0; j < link_count; j++)
			{
				if (vlink[j].group == i && vlink[j].inner.is_goodlink)
				{
					if (!(vlink[j].linkFunc & VLINK_BLOGRE) && !(vlink[j].linkFunc & VLINK_BBSCONT)
							&& (!(vlink[j].linkFunc & VLINK_COPYRIGHT) || pgl->homepage_count >= 5)
							&& !(vlink[j].linkFunc & VLINK_BBSRE) && !(vlink[j].linkFunc & VLINK_BBSSIG))
					{
						vlink[j].linkFunc |= VLINK_FRIEND;
					}
				}
			}
		}
	}
	return 1;
}

/**
 * @brief 通过分块信息补充友情链接标注
 */
int mark_linktype_friend(lt_args_t *pargs, lt_res_t *pres)
{
	const area_list_t *area_list = get_func_mark_result(pargs->atree, AREA_FUNC_FRIEND);
	if (area_list == NULL)
		return 1;
	for (int i = 0; i < pargs->link_count; i++)
	{
		if (pargs->vlink[i].inner.vnode == NULL)
			continue;
		if (in_area(&pargs->vlink[i].inner, area_list))
		{
			if ((pargs->vlink[i].inner.tag_type != TAG_IMG) && !(pargs->vlink[i].linkFunc & VLINK_INVALID))
				pargs->vlink[i].linkFunc |= VLINK_FRIEND;
		}
	}

	return 1;
}
