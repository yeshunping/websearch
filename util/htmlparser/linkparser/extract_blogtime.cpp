/***************************************************************************
 *
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_extract_blogtime.cpp,v 1.0 2012/09/01 pageparse Exp $
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

#include "log.h"
#include "chinese.h"
#include "easou_extract_time.h"
#include "easou_html_parser.h"

using namespace EA_COMMON;

int init_etpara(struct extract_time_paras *et_paras)
{
	int nret = RETURNOK;

	assert(et_paras != NULL);

	// 最大合理时间: 2037年1月1日 00:00:00
	et_paras->biggest_date = 2114352000;

	// 最小合理时间: 1995年1月1日 00:00:00
	et_paras->smallest_date = 788889600;

	// 初始化GBK数字相关字符的映射表
	if (maptable_gbk2asc[0] == 0)
		init_maptable_gbk2digit();
	// 初始化正则表达式变量
	if ((nret = init_pcre(et_paras)) != RETURNOK)
		return nret;

	return nret;
}

int free_etpara(struct extract_time_paras *et_paras)
{
	assert(et_paras != NULL);

	free_pcre(et_paras);
	return RETURNOK;
}

/****************************************************************************************
 * 功能: 从字符串@src_text提取时间和日期
 * 输入: struct extract_time_paras *et_paras - 提取时间的固定变量
 * 	 char *src                 - 待匹配字符串
 * 	 long default_time         - 给定的默认时间,只有无匹配年份或者无匹配日期是有用
 * 	 long punish_time	   - 对时间作弊网页的惩罚力度
 * 输出: struct match_position *mp - 匹配到的时间和日期串的头尾在@src_text中的偏移;
 * 				     NULL则忽略;没有匹配则均置为-1.
 * 				     注: date_end和time_end指最后一个字的开始位置
 *       long *dst_time            - 匹配到的日期和时间数据,秒为单位,1970/1/1 8:0:0为0
 *                                   若只匹配到日期,默认时间为0:0:0;
 *                                   若只匹配到时间,默认日期为default_time指定的日期;
 *                                   若只没有匹配到年,默认年份为default_time指定的年;
 *                                   若没有匹配或出错,置为-1.
 * 返回: < 0 - 出现错误
 *       = 0 - 没有匹配
 *       > 0 - 有匹配, 返回值后八位按位表示提取结果Year|Mon|Day|Hour|Min|Sec|Pm|Match,
 *                     其中最后一位Match如果有日期或者时间的两者之一有匹配到就设置为1
 * 调用: int get_time_from_string(struct SBlogTime *, char *, struct match_position *)
 *****************************************************************************************/
int extract_time_from_str(long *dst_time, struct match_position *mp, const char *src_text,
		struct extract_time_paras *et_paras, long default_time, long punish_time)
{
	int nret = RETURNOK;
	struct SBlogTime bt;
	char *dst = NULL;
	int *position_match = NULL;
	size_t src_len;
	long replace_time;

	assert(dst_time != NULL);
	assert(src_text != NULL);
	assert(et_paras != NULL);

	// init memory
	src_len = strlen(src_text) + 1;
	if ((dst = (char*) malloc(src_len)) == NULL)
	{
		nret = MALLOCERROR;
		goto EXITetfs;
	}
	else
		dst[0] = '\0';
	if ((position_match = (int*) malloc(sizeof(int) * src_len)) == NULL)
	{
		nret = MALLOCERROR;
		goto EXITetfs;
	}

	// 源字符串预处理
	if ((nret = conv_chnum2digit(src_text, dst, position_match)) != RETURNOK)
		goto EXITetfs;

	// 参数结构体初始化
	if (time(&(bt.d_time)) == -1)
	{
		nret = SYSTIMEGETERROR;
		goto EXITetfs;
	}
	if (bt.d_time > et_paras->biggest_date)
	{
		nret = SYSTIMEGETERROR;
		goto EXITetfs;
	}
	if (bt.d_time > default_time)
		bt.d_time = default_time;
	replace_time = bt.d_time - punish_time;
	bzero(&bt.default_time, sizeof(struct tm));
	if (localtime_r((time_t*) &replace_time, &bt.default_time) == NULL)
	{
		nret = SYSTIMEGETERROR;
		goto EXITetfs;
	}
	bt.time = 0x7FFFFFFF;
	bt.content_len = MAX_CONTENT_LEN; // unuse
	bt.set_date = 0;
	bt.set_time = 0;
	bt.error = RETURNOK; // unuse
	bt.et_paras = et_paras;

	// 匹配
	nret = get_time_from_string(&bt, dst, mp);
	if (nret < 0)
		goto EXITetfs;

	// 匹配位置处理
	if (mp != NULL)
	{
		if (mp->date_start >= 0)
		{
			mp->date_start = position_match[mp->date_start];
			mp->date_end = position_match[mp->date_end];
			if (isdigit(src_text[mp->date_end]))
				mp->date_end += 1;
			else
				mp->date_end += 2;
			if (strncmp(src_text + mp->date_end, "日", 2) == 0 || strncmp(src_text + mp->date_end, "年", 2) == 0)
			{
				mp->date_end += 2;
			}
		}
		if (mp->time_start >= 0)
		{
			mp->time_start = position_match[mp->time_start];
			mp->time_end = position_match[mp->time_end];
			if (isalnum(src_text[mp->time_end]))
				mp->time_end += 1;
			else
				mp->time_end += 2;
			if (strncmp(src_text + mp->time_end, "秒", 2) == 0 || strncmp(src_text + mp->time_end, "分", 2) == 0)
			{
				mp->time_end += 2;
			}
		}
	}

	// 非完全匹配处理
	if (nret <= 0)
	{
		*dst_time = -1;
	}
	else if (!(nret & GET_DATE))
	{
		bt.stime.tm_year = bt.default_time.tm_year;
		bt.stime.tm_mon = bt.default_time.tm_mon;
		bt.stime.tm_mday = bt.default_time.tm_mday;
		bt.stime.tm_hour = bt.default_time.tm_hour;
		bt.stime.tm_min = bt.default_time.tm_min;
		bt.stime.tm_sec = bt.default_time.tm_sec;
		bt.stime.tm_isdst = -1;
		bt.time = mktime(&bt.stime);
		*dst_time = (long) bt.time;
		nret &= ~(GET_MATCH);
	}
	else if (!(nret & GET_TIME))
	{
		bt.stime.tm_hour = 0;
		bt.stime.tm_min = 0;
		bt.stime.tm_sec = 0;
		bt.stime.tm_isdst = -1;
		bt.time = mktime(&bt.stime);
		*dst_time = (long) bt.time;
	}
	else
		*dst_time = (long) bt.time;

	// deal with "下午" and "午後"
	if (!(nret & GET_PM) && (nret & GET_DATE) && (nret & GET_TIME) && mp != NULL && mp->time_start != -1)
	{
		char *p;
		char temp[16];
		temp[0] = '\0';
		if (mp->time_start >= 6)
		{
			strncpy(temp, src_text + mp->time_start - 6, 6);
			temp[6] = '\0';
		}
		else if (mp->time_start == 5)
		{
			strncpy(temp, src_text + mp->time_start - 5, 5);
			temp[5] = '\0';
		}
		else if (mp->time_start == 4)
		{
			strncpy(temp, src_text + mp->time_start - 4, 4);
			temp[4] = '\0';
		}

		if (temp[0] != '\0')
		{
			p = strstr(temp, "下午");
			if (p == NULL)
				p = strstr(temp, "午後");
			if (p == NULL)
				p = strstr(temp, "午后");
			if (p != NULL)
			{
				nret |= GET_PM;
				if (nret & GET_TIME)
					localtime_r(&bt.time, &bt.stime);
				if (bt.stime.tm_hour < 12)
					*dst_time += 43200;
				mp->time_start -= strlen(temp) - (p - temp);
			}
		}
	}

	// deal with no year case, such as msn
	if ((nret & GET_DATE) && !(nret & GET_YEAR))
	{
		if (GET_FULLTIME == (nret & GET_FULLTIME))
			localtime_r(&bt.time, &bt.stime);
		while (*dst_time > bt.d_time)
		{
			bt.stime.tm_year--;
			bt.stime.tm_isdst = -1;
			bt.time = mktime(&bt.stime);
			*dst_time = (long) bt.time;
		}
	}

	if (*dst_time > bt.d_time || *dst_time < et_paras->smallest_date)
	{
		*dst_time = replace_time;
		nret &= ~(GET_MATCH);
	}

	EXITetfs: if (dst != NULL)
		free(dst);
	if (position_match != NULL)
		free(position_match);
	return nret;
}

int html_tree_extract_blogtime(long *dst_time, // output, time_t
		struct match_position *mp, // output, the posion of time string and date string
		void *src, // input, html_tree_t* when tem = TEM_VisitTree, char* other
		struct extract_time_paras *et_paras, // input, necesary parameters
		enum TimeExtractMethod tem, // input, indicate the method used for extracting time
		long default_time, // input, the default time gived by spider
		long punish_time)
{
	int nret = RETURNOK;

	assert(dst_time != NULL && src != NULL && et_paras != NULL && mp != NULL);

	*dst_time = -1;
	switch (tem)
	{
	case TEM_MainContent:
	case TEM_Content:
		nret = extract_time_from_str(dst_time, mp, (char *) src, et_paras, default_time, punish_time);
		break;
	case TEM_VisitTree:
		nret = html_tree_extract_blogtime_(dst_time, (html_tree_t *) src, et_paras);
		break;
	default:
		nret = ILLEGALINPUT;
		break;
	}

	char str[32];
	char *p;
	int s_len = mp->date_end - mp->date_start;

	if (mp->date_start < 0)
		goto BadCase;

	// 长串过滤
	if (s_len > 31)
		goto BadCase;

	strncpy(str, ((char*) src) + mp->date_start, s_len);
	str[s_len] = '\0';

	// 超短串过滤
	if (s_len == 3)
		if (str[1] == '.' || str[1] == '/' || str[1] == ' ')
			goto BadCase;

	// 数字空格串过滤"3 24",保留英文日期串"May 3"
	if ((p = strchr(str, ' ')) != NULL)
	{
		do
		{
			p++;
		} while (*p == ' ');
		if (strchr(p, ' ') == NULL)
		{
			for (p = str; *p != '\0'; p++)
				if (isalpha(int((unsigned char) (*p))))
				{
					p = NULL;
					break;
				}
			if (p != NULL)
				goto BadCase;
		}
	}

	// 过滤年月日不全的情况
	if (strstr(str, "年") != NULL)
		if (strstr(str, "月") == NULL)
			goto BadCase;
		else if (strstr(str, "日") == NULL)
			goto BadCase;
		else
			return nret;

	if (s_len >= 8)
		return nret;
	return nret;

	BadCase: nret = 0;
	*dst_time = -1;
	mp->date_start = -1;
	mp->date_end = -1;
	mp->time_start = -1;
	mp->time_end = -1;
	return nret;
}

int html_tree_extract_blogtime_(long *dst_time, html_tree_t *html_tree, struct extract_time_paras *et_paras)
{
	int nret;
	struct SBlogTime bt;

	assert(html_tree != NULL);
	assert(dst_time != NULL);

	bt.time = 0x7FFFFFFF;
	bt.content_len = MAX_CONTENT_LEN;
	bt.set_date = 0;
	bt.set_time = 0;
	bt.error = RETURNOK;
	bt.et_paras = et_paras;

	html_tree_visit(html_tree, &start_visit_for_blogtime, NULL, &bt, 0);

	nret = bt.error;
	// 出错
	if (nret < 0)
		*dst_time = -1;
	// 没有匹配到日期或者时间
	else if (!(nret & GET_MATCH))
		*dst_time = -1;
	// 匹配到了完整的时间和日期
	else if ((nret & GET_FULLTIME) == GET_FULLTIME)
		*dst_time = (long) bt.time;
	// 只匹配到了时间
	else if ((nret & GET_TIME) == GET_TIME)
	{
		bt.stime.tm_year = 70;
		bt.stime.tm_mon = 0;
		bt.stime.tm_mday = 1;
		bt.stime.tm_isdst = -1;
		bt.time = mktime(&bt.stime);
		*dst_time = (long) bt.time;
	}
	// 只匹配到了日期
	else if ((nret & GET_DATE) == GET_DATE)
	{
		bt.stime.tm_hour = 0;
		bt.stime.tm_min = 0;
		bt.stime.tm_sec = 0;
		bt.stime.tm_isdst = 0;
		bt.time = mktime(&bt.stime);
		*dst_time = (long) bt.time;
	}

	return nret;
}

int start_visit_for_blogtime(html_tag_t *html_tag, void *result, int flag)
{
	/*	int nret;
	 struct SBlogTime *t = (struct SBlogTime *)result;
	 char str[1024];
	 unsigned int len;
	 assert(1024 > MAX_CONTENT_LEN);

	 if( html_tag->tag_type == TAG_A
	 || html_tag->tag_type == TAG_HEAD
	 || html_tag->tag_type == TAG_TITLE
	 || html_tag->tag_type == TAG_NOFRAME
	 )
	 {
	 return VISIT_SKIP_CHILD;
	 }
	 else if(html_tag->tag_type == TAG_PURETEXT)
	 {
	 len = strlen(html_tag->text);
	 if( len >= MAX_CONTENT_LEN || len < 3)
	 return VISIT_NORMAL;

	 conv_chnum2digit(html_tag->text, str);
	 nret = get_time_from_string(t, str, NULL);
	 // 出错
	 if( nret < 0 )
	 {
	 t->error = nret;
	 return VISIT_FINISH;
	 }
	 // 得到完整匹配
	 else if( (nret & GET_FULLTIME) == GET_FULLTIME )
	 {
	 //ul_writelog(UL_LOG_NOTICE, "\n\t%s\n\t%s" , html_tag->text, str);
	 t->error = nret;
	 return VISIT_FINISH;
	 }
	 // 不能得到完整匹配
	 else if( nret & GET_MATCH )
	 return VISIT_NORMAL;
	 // 不能匹配
	 else
	 return VISIT_NORMAL;
	 assert(0);
	 return VISIT_NORMAL;
	 }
	 else
	 return VISIT_NORMAL;
	 */
	return VISIT_NORMAL;
}

int finish_visit_for_blogtime(html_tag_t *html_tag, void *result)
{
	return VISIT_NORMAL;
}

/****************************************************************************************
 * 功能: 从字符串@src提取时间和日期
 * 输入: char *src                 - 待匹配字符串
 * 输出: struct match_position *mp - 匹配到的时间和日期串的头尾在@src中的偏移,NULL则忽略
 *                                   没有匹配则均置为-1. 注: 头尾有1的误差.
 *       struct SBlogTime *dst     - 匹配到的日期和时间数据,具体值根据返回值获取
 * 返回: < 0 - 出现错误
 *       = 0 - 没有匹配
 *       > 0 - 有匹配, 返回值后八位按位表示提取结果Year|Mon|Day|Hour|Min|Sec|Pm|Match,
 *                     其中最后一位Match如果有日期或者时间的两者之一有匹配到就设置为1
 * 调用: int match_date(struct tm *, char *, int *, int *, pcre **)
 *       int match_time(struct tm *, char *, int *, int *, pcre *)
 *****************************************************************************************/
int get_time_from_string(struct SBlogTime *dst, char *src, struct match_position *mp)
{
	int nret = 0;
	int dret = 0, tret = 0;

	char *str = src;
	int date_start = -1, date_end = -1;
	int time_start = -1, time_end = -1;
	int flag = 0; // 1 only date, 2 only time, 3 date and time
	time_t time;
	struct tm tm_dst;

	assert(src != NULL);
	assert(dst != NULL);

	bzero(&tm_dst, sizeof(struct tm));

	// match time string first
	tret = match_time(&tm_dst, str, &time_start, &time_end, dst->et_paras);
	// no match
	if (0 == tret)
	{
	}
	else if (tret > 0)
	{
		// match the time
		assert(GET_TIME == (tret & GET_TIME));
		flag = flag | 1;
	}
	else
	{
		// error, return error number.
		nret = tret;
		goto gtfsEXIT;
	}

	// match date string
	dret = match_date(&tm_dst, str, &date_start, &date_end, dst->et_paras);
	if (0 == dret)
	{
		// no match
	}
	else if (dret > 0)
	// match the date
	{
		assert(GET_DATE == (dret & GET_DATE));
		if (!(dret & GET_YEAR))
			tm_dst.tm_year = dst->default_time.tm_year;
		flag = flag | 2;
	}
	else
	// error, return error number. 
	{
		nret = dret;
		goto gtfsEXIT;
	}

	// combine the date and time
	if (0 == flag)
		// no date or time match
		nret = 0;
	else if (3 == flag)
	// date and time both match
	{
		// convert tm to time_t
		tm_dst.tm_isdst = -1;
		time = mktime(&tm_dst);
		if (0)
		// specially design for visiting tree node method
		{
			FLAG3: dst->stime.tm_isdst = -1;
			time = mktime(&dst->stime);
		}
		if (time != -1 && time < dst->time)
			dst->time = time;
		nret = dret | tret | GET_MATCH;
	}
	else if (1 == flag)
	// only time match
	{
		if (!dst->set_time)
		// for content and maincontent method, always true, and do once only.
		{
			dst->stime.tm_hour = tm_dst.tm_hour;
			dst->stime.tm_min = tm_dst.tm_min;
			dst->stime.tm_sec = tm_dst.tm_sec;
			if (dst->set_date)
			// for content and maincontent method, always false, and never do this.
			{
				dst->set_date = 0;
				goto FLAG3;
			}
			else
				dst->set_time = 1;
		}
		nret = tret | GET_MATCH;
	}
	else /* flag == 2 */
	// only date match
	{
		if (!dst->set_date)
		// for content and maincontent method, always true, and do once only.
		{
			dst->stime.tm_year = tm_dst.tm_year;
			dst->stime.tm_mon = tm_dst.tm_mon;
			dst->stime.tm_mday = tm_dst.tm_mday;
			if (dst->set_time)
			// for content and maincontent method, always false, and never do this.
			{
				dst->set_time = 0;
				goto FLAG3;
			}
			else
				dst->set_date = 1;
		}
		nret = dret | GET_MATCH;
	}

	//ul_writelog(UL_LOG_NOTICE, "%d", dst->time);

	gtfsEXIT: if (mp != NULL)
	{
		mp->date_start = date_start;
		mp->date_end = date_end;
		mp->time_start = time_start;
		mp->time_end = time_end;
	}
	return nret;
}

int init_pattern(char date_pattern[][512], char time_pattern[])
{
	if (1)
	{
		// 1982-07-20  82-07-20
		sprintf(date_pattern[YMD], "(?:^|[^0-9])%s%s%s%s%s(?:$|[^0-9])", PATTERN_YEAR, PATTERN_YMD, PATTERN_MON,
				PATTERN_YMD, PATTERN_DAY);
		date_substring_index[YMD][0] = 2;
		date_substring_index[YMD][1] = 4;
		date_substring_index[YMD][2] = 6;

		// 07-20-1982  07-20-82
		sprintf(date_pattern[MDY], "(?:^|[^0-9])%s%s%s(?:%s%s)?(?:$|[^0-9])", PATTERN_MON, PATTERN_YMD, PATTERN_DAY,
				PATTERN_YMD, PATTERN_YEAR);
		date_substring_index[MDY][0] = 6;
		date_substring_index[MDY][1] = 2;
		date_substring_index[MDY][2] = 4;

		// Jul 20th, 1982  Jul 20th, 82
		sprintf(date_pattern[MDY_E],
				"(?:^|[^a-zA-Z])(?:%s[a-z]{0,6})%s%s(?:(?:(?:[^0-9]{1,4})%s(?:$|[^0-9]))|(?:$|[^0-9]))", PATTERN_MON_E,
				PATTERN_YMD, PATTERN_DAY, PATTERN_YEAR);
		date_substring_index[MDY_E][0] = 6;
		date_substring_index[MDY_E][1] = 2;
		date_substring_index[MDY_E][2] = 4;

		// 20th Jul, 1982  20th Jul, 82
		sprintf(date_pattern[DMY_E],
				"(?:^|[^0-9])%s[a-zA-Z]{0,2}%s(?:%s[a-z]{0,6})(?:(?:(?:[^0-9a-zA-Z]{0,2})%s%s(?:$|[^0-9]))|($|[^a-zA-Z]))",
				PATTERN_DAY, PATTERN_YMD, PATTERN_MON_E, PATTERN_YMD, PATTERN_YEAR);
		date_substring_index[DMY_E][0] = 6;
		date_substring_index[DMY_E][1] = 4;
		date_substring_index[DMY_E][2] = 2;

		// 20 十一月 2006
		sprintf(date_pattern[DMY_C], "(?:^|[^0-9])%s[\040]%s-[\040]%s(?:$|[^0-9a-z])", PATTERN_DAY, PATTERN_MON,
				PATTERN_YEAR);
		date_substring_index[DMY_C][0] = 6;
		date_substring_index[DMY_C][1] = 4;
		date_substring_index[DMY_C][2] = 2;

		// Thu Dec 21 23:35:07 CST 2006
		sprintf(date_pattern[MDY_LINUX], "(?:^|[^a-zA-Z])(?:%s[\040]%s[\040][a-z0-9:]{8}[\040]cst[\040]%s)(?:$|[^0-9])",
				PATTERN_MON_E, PATTERN_DAY, PATTERN_YEAR);
		date_substring_index[MDY_LINUX][0] = 6;
		date_substring_index[MDY_LINUX][1] = 2;
		date_substring_index[MDY_LINUX][2] = 4;

		// 12:12:12 am
		sprintf(time_pattern, "(?:^|[^0-9])%s%s%s(?:%s%s)?(?:%s)?", PATTERN_HOUR, PATTERN_HMS, PATTERN_MINUTE,
				PATTERN_HMS, PATTERN_SECOND, PATTERN_AP);
		time_substring_index[0] = 2;
		time_substring_index[1] = 4;
		time_substring_index[2] = 6;
		time_substring_index[3] = 8;
	}
	else
	{
		sprintf(date_pattern[YMD], "([0-9]{2,4})[-/.\040]{1,2}([0-9]{1,2})[-/.\040]{1,2}([0-9]{1,2})(?:$|[^0-9])");
		date_substring_index[YMD][0] = 2;
		date_substring_index[YMD][1] = 4;
		date_substring_index[YMD][2] = 6;

		sprintf(date_pattern[MDY], "([0-9]{1,2})[-/.\040]{1,2}([0-9]{1,2})(?:[-/.\040]{1,2}([0-9]{2,4}))?");
		date_substring_index[MDY][0] = 6;
		date_substring_index[MDY][1] = 2;
		date_substring_index[MDY][2] = 4;

		sprintf(date_pattern[MDY_E],
				"(jan|feb|mar|apr|may|jun|jul|jy|aug|sep|sept|oct|nov|dec)[a-zA-Z]{0,6}[^0-9a-zA-Z]{1,2}([0-9]{1,2})[^0-9a-zA-Z]{1,2}([0-9]{2,4})");
		date_substring_index[MDY_E][0] = 6;
		date_substring_index[MDY_E][1] = 2;
		date_substring_index[MDY_E][2] = 4;

		sprintf(date_pattern[DMY_E],
				"([0-9]{1,2})[^0-9a-zA-Z]{1,2}(jan|feb|mar|apr|may|jun|jul|jy|aug|sep|sept|oct|nov|dec)[a-zA-Z]{0,6}[^0-9a-zA-Z]{1,2}([0-9]{2,4})");
		date_substring_index[DMY_E][0] = 6;
		date_substring_index[DMY_E][1] = 4;
		date_substring_index[DMY_E][2] = 2;

		sprintf(time_pattern, "(?:^|[^0-9])%s%s%s(?:%s%s)?(?:%s)?", PATTERN_HOUR, PATTERN_HMS, PATTERN_MINUTE,
				PATTERN_HMS, PATTERN_SECOND, PATTERN_AP);
		time_substring_index[0] = 2;
		time_substring_index[1] = 4;
		time_substring_index[2] = 6;
		time_substring_index[3] = 8;
	}
	return RETURNOK;
}

int init_pcre(struct extract_time_paras *et_paras)
{
	int i;
	const char *error = NULL;
	int erroffset = 0;
	char date_pattern[DP_NUM][512] =
	{ "" };
	char time_pattern[256] =
	{ "" };
	char *where = "init_pcre()";

	// patterns
	init_pattern(date_pattern, time_pattern);

	// Compile the date regular expression	
	if ((et_paras->date_re = (pcre **) malloc(sizeof(pcre*) * DP_NUM)) == NULL)
		return MALLOCERROR;
	if ((et_paras->date_pe = (pcre_extra **) malloc(sizeof(pcre_extra *) * DP_NUM)) == NULL)
		return MALLOCERROR;
	for (i = 0; i < DP_NUM; i++)
	{
		et_paras->date_re[i] = pcre_compile(date_pattern[i], /* the pattern */
		PCRE_CASELESS | PCRE_EXTENDED, /* options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character tables */
		if (et_paras->date_re[i] == NULL)
		// compile error
		{
			Error("%s: PCRE compil date failed at offset %d: %s\n", where, erroffset, error);
			return IllegalPattern;
		}
		et_paras->date_pe[i] = pcre_study(et_paras->date_re[i], /* result of pcre_compile() */
		0, /* no options exist */
		&error); /* set to NULL or points to a message */

	}

	// Compile the time regular expression
	et_paras->time_re = pcre_compile(time_pattern, /* the pattern */
	PCRE_CASELESS | PCRE_EXTENDED, /* options */
	&error, /* for error message */
	&erroffset, /* for error offset */
	NULL); /* use default character tables */
	if (et_paras->time_re == NULL)
	// compile error
	{
		Error("%s: PCRE compil time failed at offset %d: %s\n", where, erroffset, error);
		return IllegalPattern;
	}
	et_paras->time_pe = pcre_study(et_paras->time_re, 0, &error);

	return RETURNOK;
}

int free_pcre(struct extract_time_paras *et_paras)
{
	int i;

	// free date pcre
	for (i = 0; i < DP_NUM; i++)
	{
		(*pcre_free)(et_paras->date_re[i]);
		(*pcre_free)(et_paras->date_pe[i]);
	}
	free(et_paras->date_re);
	free(et_paras->date_pe);
	// free time pcre
	(*pcre_free)(et_paras->time_re);
	(*pcre_free)(et_paras->time_pe);

	return RETURNOK;
}

/*****************************************************************************
 * 功能: 使用@date_re匹配字符串@src
 * 输入: char *src             - 待匹配字符串
 *       pcre *date_re[DP_NUM] - pcre正则表达模式变量,由pattern编译得到
 * 输出: struct tm *time       - 匹配到的日期结果
 *       int *start            - 匹配到的日期串的开始字符偏移量
 *       int *end              - 匹配到的日期串的结束字符偏移量
 * 返回: < 0        - 出现错误
 *       = 0 	    - 无匹配
 *       = GET_DATE - 得到日期的匹配数据
 * 调用: int set_date_from_regstr(struct tm *, char *, enum DateType, int *)
 *****************************************************************************/
int match_date(struct tm *time, char *src, int *start, int *end, struct extract_time_paras *et_paras)
{
	int nret = 0;
	int i = 0;
	char *where = "date_time()";
	int rc = 0;
	int ovector[D_OVECCOUNT];
	int slen = strlen(src);
	int offset = 0;
	time_t t_temp = 0;

	for (i = 0; i < DP_NUM; i++)
	{
//MATCH:
		rc = pcre_exec(et_paras->date_re[i], /* the compiled pattern */
		et_paras->date_pe[i], /* we didn't study the pattern */
		src, /* the subject string */
		slen, /* the length of the subject */
		offset, /* start at offset 0 in the subject */
		0, /* default options */
		ovector, /* vector for substring information */
		D_OVECCOUNT); /* number of elements in the vector */
		if (rc == PCRE_ERROR_NOMATCH)
			// no match
			continue;
		else if (rc < 0)
		// error
		{
			Error("%s: date pattern %d match error[%d].", where, i, rc);
			nret = REGMATCHERROR;
			return nret;
		}
		else
		// get match
		{
			nret = set_date_from_regstr(time, src, (enum DateType) i, ovector);
			// check year
			time->tm_isdst = -1;
			t_temp = mktime(time);
//			if( t_temp > et_paras->biggest_date || t_temp < et_paras->smallest_date)
//			{
//				offset    = ovector[1]-1;
//				goto MATCH;
//			}
			switch ((enum DateType) i)
			{
			case YMD:
			case MDY:
			case DMY_C:
			case DMY_E:
				if (!isdigit(src[ovector[0]]))
					*start = ovector[0] + 1;
				else
					*start = ovector[0];
				break;
			case MDY_E:
			case MDY_LINUX:
				if (!isdigit(src[ovector[0]]))
					*start = ovector[0] + 1;
				else
					*start = ovector[0];
				break;
			default:
				assert(0);
				break;
			}
			if (!isdigit(src[ovector[1] - 1]))
				*end = ovector[1] - 2;
			else
				*end = ovector[1] - 1;
			break;
		}
	}

	if (i == DP_NUM)
		// can't match any pattern
		nret = 0;

	return nret;
}

int set_date_from_regstr(struct tm *time, char *date_string, enum DateType dt, int *ovector)
{
	int nret = GET_DATE;
	char *p = NULL;

	if (dt == MDY_E || dt == MDY_LINUX || dt == DMY_E)
	{
		p = date_string + ovector[date_substring_index[dt][1]];
		assert(isalpha(*p));
		if (*p == 'a' || *p == 'A')
		{
			if (*(p + 1) == 'p' || *(p + 1) == 'P')
				*p++ = '4';
			else
				*p++ = '8';
		}
		else if (*p == 'd' || *p == 'D')
		{
			*p++ = '1';
			*p++ = '2';
		}
		else if (*p == 'f' || *p == 'F')
			*p++ = '2';
		else if (*p == 'j' || *p == 'J')
			if (*(p + 1) == 'a' || *(p + 1) == 'A')
				*p++ = '1';
			else if (*(p + 1) == 'y' || *(p + 1) == 'Y')
				*p++ = '7';
			else if (*(p + 2) == 'n' || *(p + 2) == 'N')
				*p++ = '6';
			else
				*p++ = '7';
		else if (*p == 'm' || *p == 'M')
			if (*(p + 2) == 'r' || *(p + 2) == 'R')
				*p++ = '3';
			else
				*p++ = '5';
		else if (*p == 'n' || *p == 'N')
		{
			*p++ = '1';
			*p++ = '1';
		}
		else if (*p == 's' || *p == 'S')
			*p++ = '9';
		else
		{
			*p++ = '1';
			*p++ = '0';
		}
	}

	int index = ovector[date_substring_index[dt][0]];
	if (index != -1)
	{
		nret |= GET_YEAR;
		time->tm_year = atoi(date_string + ovector[date_substring_index[dt][0]]);
	}
	else
		time->tm_year = 0;
	time->tm_mon = atoi(date_string + ovector[date_substring_index[dt][1]]);
	time->tm_mday = atoi(date_string + ovector[date_substring_index[dt][2]]);
	// deal with the year num 
	if (time->tm_year >= 1970)
		time->tm_year -= 1900;
	else if (time->tm_year < 38)
		time->tm_year += 100;
	// deal with the month num
	time->tm_mon -= 1;

//	char dstr[256];
//	strncpy(dstr, date_string+ovector[0], ovector[1]-ovector[0]);
//	dstr[ovector[1]-ovector[0]] = '\0';
//	printf("DateStr = [%s], DateType = [%d], year = [%d], month = [%d], day = [%d].\n", dstr, dt, time->tm_year, time->tm_mon, time->tm_mday);

	return nret;
}

/*****************************************************************************
 * 功能: 使用@time_re匹配字符串@src
 * 输入: char *src             - 待匹配字符串
 *       pcre *time_re         - pcre正则表达模式变量,由pattern编译得到
 * 输出: struct tm *time       - 匹配到的时间结果
 *       int *start            - 匹配到的时间串的开始字符偏移量
 *       int *end              - 匹配到的时间串的结束字符偏移量
 * 返回: < 0        - 出现错误
 *       = 0        - 无匹配
 *       = GET_TIME - 得到时间的匹配数据
 * 调用: int set_time_from_regstr(struct tm *, char *, int *)
 ******************************************************************************/
int match_time(struct tm *time, char *src, int *start, int *end, struct extract_time_paras *et_paras)
{
	int nret = 0;

	int i = 0;
	char *where = "match_time()";
	int rc;
	int ovector[T_OVECCOUNT];
	int slen = strlen(src);

	rc = pcre_exec(et_paras->time_re, /* the compiled pattern */
	et_paras->time_pe, /* we didn't study the pattern */
	src, /* the subject string */
	slen, /* the length of the subject */
	0, /* start at offset 0 in the subject */
	0, /* default options */
	ovector, /* vector for substring information */
	T_OVECCOUNT); /* number of elements in the vector */
	if (rc == PCRE_ERROR_NOMATCH)
	// no match
	{
		nret = 0;
		goto mtEXIT;
	}
	else if (rc < 0)
	// error
	{
		Error("%s: time pattern %d match error[%d].", where, i, rc);
		nret = REGMATCHERROR;
		goto mtEXIT;
	}
	else
	{
		if (!isdigit(src[ovector[0]]))
			*start = ovector[0] + 1;
		else
			*start = ovector[0];
		nret = set_time_from_regstr(time, src, ovector);
		if (nret & GET_PM)
		{
			if (!isalpha(src[ovector[1] - 1]))
				*end = ovector[1] - 2;
			else
				*end = ovector[1] - 1;
		}
		else
		{
			if (!isdigit(src[ovector[1]]))
				*end = ovector[1] - 1;
			else
				*end = ovector[1];
		}
	}

	mtEXIT: return nret;
}

int set_time_from_regstr(struct tm *time, char *time_string, int *ovector)
{
	int nret = GET_TIME;
	int index;

	if (isdigit((unsigned int) time_string[ovector[1]]))
		time_string[ovector[1]] = ' ';
	// get hour
	time->tm_hour = atoi(time_string + ovector[time_substring_index[0]]);
	// get minute
	time->tm_min = atoi(time_string + ovector[time_substring_index[1]]);
	// get second
	index = ovector[time_substring_index[2]];
	if (index != -1)
	{
		nret |= GET_SEC;
		time->tm_sec = atoi(time_string + index);
	}
	else
		time->tm_sec = 0;
	// get am or pm
	index = ovector[time_substring_index[3]];
	if (index != -1)
	{
		nret |= GET_PM;
		if (time_string[index] == 'p' || time_string[index] == 'P')
			if (time->tm_hour < 12)
				time->tm_hour += 12;
	}

	// destory the time string to make sure date matching can't use time string.
	time_string[ovector[time_substring_index[0]]] = 'u';
	time_string[ovector[time_substring_index[0]] + 1] = 's';
	time_string[ovector[time_substring_index[0]] + 2] = 'e';
	if (time_string[ovector[time_substring_index[0]] + 3] != '\0')
		time_string[ovector[time_substring_index[0]] + 3] = 'd';

//	char tstr[256];
//	strncpy(tstr, time_string+ovector[0], ovector[1]-ovector[0]);
//	printf("TimeStr = [%s], hour = [%d], minute = [%d], second = [%d].\n", tstr, time->tm_hour, time->tm_min, time->tm_sec);

	return nret;
}

unsigned int get_gbkvalue(const unsigned char *gbk)
{
	if (gbk == NULL || gbk[0] == '\0')
		return 0;
	return (((unsigned int) gbk[0]) << 8) + (unsigned int) gbk[1];
}

int init_maptable_gbk2digit()
{
	memset(maptable_gbk2asc, '\n', 65535);

	int i;
	// 0 ~ 9
	for (i = 0; i < num_list_gbkdigit; i++)
		maptable_gbk2asc[get_gbkvalue(list_gbkdigit[i])] = '0' + i % 10;
	// 十
	maptable_gbk2asc[get_gbkvalue((unsigned char *) "十")] = 'a';

	// dsp
	for (i = 0; i < num_list_gbkdsp; i++)
		maptable_gbk2asc[get_gbkvalue(list_gbkdsp[i])] = '-';

	// tsp
	for (i = 0; i < num_list_gbktsp; i++)
		maptable_gbk2asc[get_gbkvalue(list_gbktsp[i])] = ':';

	return 1;
}

/********************************************
 * 功能: 中文数字转成阿拉伯数字
 * 输入: const char *src
 * 输出: char *dst
 ********************************************/
int conv_chnum2digit(const char *src, char *dst, int position_match[])
{
	int i_s, i_d;
	char *ptr, *ptmp;
	unsigned int value;

	assert(src != NULL);
	assert(dst != NULL);
	assert(position_match != NULL);

	ptr = (char*) src;
	ptmp = dst;
	i_s = 0;
	i_d = 0;
	while (*ptr)
	{
		if (i_d + 2 >= MAX_CONVERT_LENGTH)
			break;
		if (IS_GBK(ptr))
		{
			value = get_gbkvalue((unsigned char*) ptr);
			if (maptable_gbk2asc[value] >= '0' && maptable_gbk2asc[value] <= '9')
			{
				*ptmp++ = maptable_gbk2asc[value];
				position_match[i_d++] = i_s;
				ptr += 2;
				i_s += 2;
			}
			else if ('a' == maptable_gbk2asc[value])
			{
				if (ptmp - 1 < dst)
				// now is first one
				{
					*ptmp++ = '1';
					position_match[i_d++] = i_s;
					ptr += 2;
					i_s += 2;
				}
				else if (*(ptmp - 1) >= '0' && *(ptmp - 1) <= '9')
				{
					ptr += 2;
					i_s += 2;
				}
				else
				{
					*ptmp++ = '1';
					position_match[i_d++] = i_s;
					ptr += 2;
					i_s += 2;
				}

				if (IS_GBK(ptr))
				{
					value = get_gbkvalue((unsigned char*) ptr);
					if ('0' > maptable_gbk2asc[value] || '9' < maptable_gbk2asc[value])
					// 中文＂十＂以后跟的并非数字，则添加０
					{
						*ptmp++ = '0';
						position_match[i_d++] = i_s - 2;

					}
					*ptmp++ = maptable_gbk2asc[value];
					position_match[i_d++] = i_s;
					ptr += 2;
					i_s += 2;
				}
				else
				{
					*ptmp++ = '0';
					position_match[i_d++] = i_s - 2;
				}
			}
			else
			{
				*ptmp++ = maptable_gbk2asc[value];
				position_match[i_d++] = i_s;
				ptr += 2;
				i_s += 2;
			}
		}
		else if (*ptr >= 0 && *ptr <= 31)
		{
			*ptmp++ = '\n';
			position_match[i_d++] = i_s;
			ptr++;
			i_s++;
		}
		else
		{
			*ptmp++ = *ptr++;
			position_match[i_d++] = i_s++;
		}
	}

	*ptmp = '\0';

	char *p;
	ptmp = dst; // old
	ptr = dst; // new
	while (*ptmp != '\0')
	{
		while (' ' == *ptmp || '\n' == *ptmp)
			ptmp++;
		p = ptmp;
		ptmp = strchr(ptmp, '\n');
		if (ptmp != NULL && ptmp - p < 3)
		{
			ptmp++;
			continue;
		}
		while (*p != '\n' && *p != '\0')
		{
			*ptr = *p;
			position_match[ptr - dst] = position_match[p - dst];
			ptr++;
			p++;
		}
		if (*p == '\n')
		{
			ptmp = p + 1;
			*ptr = '*';
			position_match[ptr - dst] = position_match[ptr - dst - 1];
			ptr++;
		}
		else
			break;
	}
	*ptr = '\0';

	return RETURNOK;
}

