/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_invalid_and_control.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_invalid_and_control.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记不可见链接
 **/
#include "easou_link.h"
#include "easou_url.h"
#include "easou_mark_parser.h"
#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_vhtml_parser.h"
#include "html_text_utils.h"
#include <ctype.h>

/**
 * @brief short description 标记不可见链接
 */
int mark_linktype_invalid_and_control(lt_args_t *pargs, lt_res_t *pres)
{
	for (int i = 0; i < pargs->link_count; i++)
	{
		if (pargs->vlink[i].inner.vnode == NULL)
			continue;
		html_vnode_t *vnode = pargs->vlink[i].inner.vnode;
		if (vnode && !vnode->isValid)
		{
			if ((pargs->vlink[i].inner.tag_type != TAG_IMG))
			{
				pargs->vlink[i].linkFunc |= VLINK_INVALID;
			}
		}
	}
	return 1;
}
