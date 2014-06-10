/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_timematch.h,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/
 
 
 
/**
 * @file easou_link_timematch.h
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 从字符串中找时间串
 **/

#ifndef _EASOU_LINK_TIMEMATCH_H
#define _EASOU_LINK_TIMEMATCH_H

/*
 * 用于时间查找的结构
 */
#include "pcre.h"

/**
* @brief short description 时间识别用的包
*/
typedef struct _TIMEMATCH_PACK{
	pcre **date_re;
	pcre_extra **date_pe;
}timematch_pack_t;

//create pack for timematch
timematch_pack_t * timematch_pack_create();

//del timematch pack
void timematch_pack_del(timematch_pack_t *ppack);

//find the text whether has time string
//ppack : timematch pack
//text : input string
//bbs : whether the text is from bbs, use diffrent regex string in bbs page
//return : 0, has time    1, not has time
int hastime(timematch_pack_t *ppack, char *text, int bbs);

#endif
