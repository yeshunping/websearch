/**
 * @file easou_vhtml_com.cpp

 * @date 2011/06/20
 * @brief 一些vTree解析基础函数的实现。
 *
 * 1, 解析style属性的函数;
 * 2, vTree非递归遍历函数;
 *  
 **/
#include <ctype.h>
#include <stdio.h>
#include "util/htmlparser/utils/chinese.h"
#include "util/htmlparser/utils/string_util.h"
#include "util/htmlparser/vhtmlparser/vhtml_inner.h"

/**
 * @brief 解析一个style代码段

 * @date 2011/06/20
 **/
int parse_style_attr(const char *style_str, style_attr_t *style_attr, int max_style_attr)
{
	const char *p = style_str;
	int attr_cnt = 0;
	int i;

	// skip space
	while (*p != '\0' && easou_isspace((unsigned char)*p))
	{
		p++;
	}

	while (*p != '\0')
	{
		if (attr_cnt >= max_style_attr)
		{
//			Debug("style attribute much than %d\n", max_style_attr); // debug
			break;
		}

		style_attr[attr_cnt].name[0] = 0;
		style_attr[attr_cnt].value[0] = 0;

		//name
		i = 0;
		while (*p != '\0' && *p != ':' && !easou_isspace ((unsigned char)*p))
		{
			if (i < MAX_ATTR_NAME_LENGTH - 1)
				style_attr[attr_cnt].name[i++] = tolower(*p++);
			else
				p++;
		}
		style_attr[attr_cnt].name[i] = 0;

		// skip space
		while (*p != '\0' && easou_isspace ((unsigned char) *p))
			p++;
		// split char

		if (*p == ':')
		{
			p++;
			//space
			while (*p != '\0' && easou_isspace((unsigned char)*p))
			{
				p++;
			}
			i = 0;
			//value
			while (*p != '\0' && *p != ';')
			{
				if (i < MAX_ATTR_VALUE_LENGTH - 1)
				{
					style_attr[attr_cnt].value[i++] = tolower(*p++);
				}
				else
				{
					*p++;
				}
			}
			//skip value last space
			while (i - 1 >= 0 && easou_isspace((unsigned char)style_attr[attr_cnt].value[i-1]))
				i--;
			style_attr[attr_cnt].value[i] = '\0';
		}
		// skip  ';'
		while (*p != '\0' && (easou_isspace((unsigned int)*p) || *p == ';'))
			p++;
		attr_cnt++;
	}

	return attr_cnt;
}

/**
 * @brief 得到属性值

 * @date 2011/06/20
 **/
int get_style_attr_value(const char *name, char *value, int vsize, style_attr_t *style_attr, int style_num)
{
	int i;
	for (i = 0; i < style_num; i++)
	{
		if (strcmp(style_attr[i].name, name) == 0)
		{
			snprintf(value, vsize, "%s", style_attr[i].value);
			return 1;
		}
	}
	return 0;
}

/**
 * @brief 判断val是否给定的值.
 * @param [in/out] val   : const char* 待判断的属性值.
 * @param [in/out] dest   : const char* 给定的值.
 * @param [in/out] dest_len   : size_t 给定值的长度.

 * @date 2011/06/20
 **/
bool is_attr_value(const char *val, const char *dest, size_t dest_len)
{
	if (strncmp(val, dest, dest_len) == 0)
	{
		if (val[dest_len] == '\0' || val[dest_len] == ' ' || val[dest_len] == '!')
			return true;
	}
	return false;
}

/**
 * @brief 字符串转化为分数.
 * 为消除浮点数影响，将字符串化为分数进行计算.

 * @date 2011/06/20
 **/
void atofract(fract_num_t *fract, const char *val)
{
	const char *pval = val;
	bool is_negative = 0;

	switch (*pval)
	{
	case '+':
		pval++;
		break;
	case '-':
		pval++;
		is_negative = true;
		break;
	default:
		break;
	}

	fract->son = 0;
	fract->mother = 1;

	int d_factor = 1;

	while (*pval && fract->mother < 1000)
	{
		if (isdigit(*pval))
		{
			fract->son *= 10;
			fract->son += *pval - '0';
			fract->mother *= d_factor;
		}
		else if (*pval == '.')
		{
			d_factor = 10;
		}
		else
		{
			break;
		}
		pval++;
	}

	if (is_negative)
	{
		fract->son = 0 - fract->son;
	}
}

/**
 * @brief 是否单字符的转义符
 * assert(src[0] == '&');

 * @date 2011/06/27
 **/
static inline int check_single_reference_char(const char *src)
{
	if (src[1] == 'n' && src[2] == 'b' && src[3] == 's' && src[4] == 'p')
		return 5;
	else if (src[1] == 'g' && src[2] == 't')
		return 3;
	else if (src[1] == 'l' && src[2] == 't')
		return 3;
	else if (src[1] == 'q' && src[2] == 'u' && src[3] == 'o' && src[4] == 't')
		return 5;
	else if (src[1] == 'a' && src[2] == 'm' && src[3] == 'p')
		return 4;

	return 0;
}
