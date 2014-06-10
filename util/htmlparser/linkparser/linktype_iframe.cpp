/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_copyright.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_copyright.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记版权链接
 **/

#include "easou_link_mark.h"
#include "easou_link_common.h"
#include "easou_mark_parser.h"
#include "easou_mark_func.h"
#include "easou_vhtml_parser.h"
#include "html_text_utils.h"

#include <ctype.h>

/**
 * @brief 标记iframe链接
 */
int mark_linktype_iframe(lt_args_t *pargs, lt_res_t *pres)
{
	for (int i = 0; i < pargs->link_count; i++)
	{
		if (pargs->vlink[i].inner.vnode == NULL)
			continue;
		if ((pargs->vlink[i].inner.tag_type == TAG_IFRAME))
			pargs->vlink[i].linkFunc |= VLINK_IFRAME;
	}
	return 1;
}
