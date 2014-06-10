/***************************************************************************
 *
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_extract_time.h,v 1.0 2012/09/01 pageparse Exp $
 *
 **************************************************************************/

#ifndef __EASOU_EXTRACT_TIME__
#define __EASOU_EXTRACT_TIME__

#include "easou_html_tree.h"
#include "easou_extract_blogtime.h"

#include <regex.h>
#include <sys/types.h>

// additional error number
#define IllegalPattern  (MINRETURN-1)
#define IllegalTimeStr  (MINRETURN-2)
#define NOMATCH         (MINRETURN-3)
#define REGMATCHERROR   (MINRETURN-4)
#define IllegalTime	(MINRETURN-5)

// static data
#define MAX_CONVERT_LENGTH 128000

static int num_list_gbkdigit = 30;
static unsigned char list_gbkdigit[30][3] =
{ "０", "１", "２", "３", "４", "５", "６", "７", "８", "９", "零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "零", "壹", "贰",
		"叁", "肆", "伍", "陆", "柒", "捌", "玖" }; //--> 0~9

static int num_list_gbkdsp = 5;
static unsigned char list_gbkdsp[5][3] =
{ "年", "月", "／", "－", "　" }; //-->'-'

static int num_list_gbktsp = 5;
static unsigned char list_gbktsp[5][3] =
{ "時", "时", "点", "分", "：" }; //-->':'

static unsigned char maptable_gbk2asc[65536] =
{ 0 };

// date 
#define D_OVECCOUNT 30 // should be a multiple of 3
#define DP_NUM 6 
static int date_substring_index[DP_NUM][3];
enum DateType
{
	YMD = 2, MDY = 5, MDY_E = 3, DMY_E = 4, DMY_C = 0, MDY_LINUX = 1
};
#define PATTERN_YEAR	"((?:(?:19)?[7-9][0-9])|(?:(?:20)?(?:(?:[0-2][0-9])|(?:3[0-7]))))"
#define PATTERN_MON	"((?:0?[1-9])|(?:1[0-2]))"
#define PATTERN_MON_E	"(jan|feb|mar|apr|may|jun|jul|jy|aug|sep|sept|oct|nov|dec)"
#define PATTERN_MON_EA	"(january|february|march|april|may|june|july|august|september|october|november|december)"
#define PATTERN_DAY	"((?:0?[1-9])|(?:[1-2][0-9])|(?:3[0-1]))"
#define PATTERN_YMD	"(?:[-/.\040]{1,2})"

// time
#define T_OVECCOUNT 30 // should be a multiple of 3
static int time_substring_index[4];
enum TimeType
{
	HMSA = 0
};
#define PATTERN_HOUR            "((?:1[0-9])|(?:2[0-3])|(?:0?[0-9]))"
#define PATTERN_MINUTE          "((?:[1-5][0-9])|(?:0?[0-9]))"
#define PATTERN_SECOND          "((?:[1-5][0-9])|(?:0?[0-9]))"
#define PATTERN_HMS             "[:]{1,2}"
#define PATTERN_AP              "[\040]{1,2}(pm|am|p.m.|a.m.)(?:$|[^a-zA-Z])"

// functions
int init_maptable_gbk2digit();
int init_pattern(char date_p[][512], char time_p[]);
int init_pcre(struct extract_time_paras *et_paras);
int free_pcre(struct extract_time_paras *et_paras);

int conv_chnum2digit(const char *src, char *dst, int position_match[]);
int get_time_from_string(struct SBlogTime *time, char *src, struct match_position *mp);
int match_date(struct tm *time, char *src, int *start, int *end, struct extract_time_paras *et_paras);
int set_date_from_regstr(struct tm *time, char *time_string, enum DateType dt, int *ovector);
int match_time(struct tm *time, char *src, int *start, int *end, struct extract_time_paras *et_paras);
int set_time_from_regstr(struct tm *time, char *time_string, int *ovector);
int html_tree_extract_blogtime_(long *dst_time, html_tree_t *html_tree, struct extract_time_paras *et_paras);
int start_visit_for_blogtime(html_tag_t *html_tag, void *result, int flag);
int finish_visit_for_blogtime(html_tag_t *html_tag, void *result);

struct SBlogTime
{
	time_t time; // 和ttime匹配
	struct tm stime; // 日期和时间分隔
	unsigned int content_len;
	int set_date;
	int set_time;
	int error;
	struct extract_time_paras *et_paras;
	struct tm default_time;
	long d_time;
};

#endif
