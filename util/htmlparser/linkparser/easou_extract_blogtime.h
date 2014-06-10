/***************************************************************************
 *
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_extract_blogtime.h,v 1.0 2012/09/01 pageparse Exp $
 *
 **************************************************************************/

#ifndef __EASOU_EXTRACT_BLOGTIME__
#define __EASOU_EXTRACT_BLOGTIME__

#include "easou_html_tree.h"
#include "pcre.h"

#ifndef __FUNCTION_ERROR__
#define __FUNCTION_ERROR__

// the range of the return value
#define MAXRETURN    9
#define MINRETURN -100

// function return success
#define RETURNOK        1
// function fail with file opration error
#define FOPENERROR      -1
#define FREADERROR      -2
#define FGETSERROR      -3
#define FWRITEERROR     -4
#define FPUTSERROR      -5
#define FCLOSEERROR     -6
// function fail with memory error
#define MALLOCERROR     -11
// function fail with network communication error

// function fail with ul_lib calling error

// system call error
#define SYSTIMEGETERROR -41
// function fail with illegal input paras
#define ILLEGALINPUT    -51

#endif

#define GET_YEAR	128
#define GET_MON		64
#define GET_DAY		32
#define GET_HOUR	16
#define GET_MIN		8
#define GET_SEC		4
#define GET_PM		2
#define GET_MATCH	1

#define GET_TIME	(GET_HOUR | GET_MIN)
#define GET_DATE	(GET_MON | GET_DAY)
#define GET_FULLTIME	(GET_TIME | GET_DATE)

#ifndef __EXTRACT_TIME_PARAS__
#define __EXTRACT_TIME_PARAS__

struct extract_time_paras
{
	pcre **date_re;
	pcre_extra **date_pe;
	pcre *time_re;
	pcre_extra *time_pe;
	time_t biggest_date;
	time_t smallest_date;
};
#endif

struct match_position
{
	int date_start;
	int date_end;
	int time_start;
	int time_end;
};

enum TimeExtractMethod
{
	TEM_VisitTree = 1, TEM_Content = 2, TEM_MainContent = 3
};

// 定义含有blog发表日期的PURETEXT的text串的最大长度
// only used in TEM_VisitTree 
#define MAX_CONTENT_LEN 128

// 初始化所需变量
// flag = 1, 复杂规则
// flag = 0, 简单规则
int init_etpara(struct extract_time_paras *et_paras);

// 从一个文本串提取时间
int extract_time_from_str(long *dst_time, struct match_position *mp, const char *src_text,
		struct extract_time_paras *et_paras, long default_time, long punish_time);

// 从一个建立好的网页树（单日志网页）提取日志发表时间
int html_tree_extract_blogtime(long *dst_time, // output, time_t
		struct match_position *mp, // output, the posion of time string and date string
		void *src, // input, html_tree_t* when tem = TEM_VisitTree, char* other
		struct extract_time_paras *et_paras, // input, necesary parameters
		enum TimeExtractMethod tem, // input, indicate the method used for extracting time
		long default_time, // input, the default time gived by spider
		long punish_time); // input, the punish time for wrong time

// 释放变量空间
int free_etpara(struct extract_time_paras *et_paras);

#endif

