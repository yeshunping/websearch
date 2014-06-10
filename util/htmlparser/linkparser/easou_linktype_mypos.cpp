/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_linktype_mypos.cpp,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_linktype_mypos.cpp
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 标记mypos链接
 **/

#include "easou_link_mark.h"
#include "easou_mark_parser.h"
#include "easou_mark_func.h"
#include "easou_link_common.h"
#include "easou_vhtml_parser.h"
#include "html_text_utils.h"
#include <ctype.h>

/**
 * @brief short description 标记mypos链接
 */
int mark_linktype_mypos(lt_args_t *pargs, lt_res_t *pres)
{
	const area_list_t *area_list = get_func_mark_result(pargs->atree, AREA_FUNC_MYPOS);
	if (!area_list)
		return 1;
	for (int i = 0; i < pargs->link_count; i++)
	{
		if (pargs->vlink[i].inner.vnode == NULL)
			continue;
		if (in_area(&pargs->vlink[i].inner, area_list))
		{
			if ((pargs->vlink[i].inner.tag_type != TAG_IMG) && !(pargs->vlink[i].linkFunc & VLINK_INVALID))
				pargs->vlink[i].linkFunc |= VLINK_MYPOS;
		}
	}
	return 1;
}
