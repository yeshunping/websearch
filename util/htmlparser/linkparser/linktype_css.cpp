/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_css.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_css.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 
 **/

#include "easou_link.h"
#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_css_utils.h"

/**
 * @brief 是否作用于屏幕媒体的style标签.
 **/
static bool is_valid_style_tag(html_tag_t *html_tag)
{
	return html_tag->tag_type == TAG_STYLE && is_apply_for_screen_media(html_tag);
}

int mark_linktype_css(lt_args_t *pargs, lt_res_t *pres)
{
	vlink_t *vlink = pargs->vlink;
	int link_count = pargs->link_count;

	for (int i = 0; i < link_count; i++)
	{
		if (vlink[i].inner.vnode == NULL)
			continue;
		html_tag_t *html_tag = &(vlink[i].inner.node->html_tag);
		if (is_css_link_tag(html_tag) || (is_valid_style_tag(html_tag) && vlink[i].inner.is_for_screen))
			vlink[i].linkFunc |= VLINK_CSS;
	}
	return 1;
}
