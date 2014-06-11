

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chinese.h"
#include "string_util.h"
#include "html_text_utils.h"

char legal_char_set[256] =
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

char url_eng_set[256] =
{ /* 0-9; A-Z; a-z; */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 00 - 0f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 10 - 1f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 20 - 2f */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, /* 30 - 3f */
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 40 - 4f */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 50 - 5f */
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 60 - 6f */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 70 - 7f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 80 - 8f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 90 - 9f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* a0 - af */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* b0 - bf */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* c0 - cf */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* d0 - df */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* e0 - ef */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 /* f0 - ff */
};

char legal_word_set[256] =
{ /* 0-9; A-Z; a-z; and chinese char */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 00 - 0f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 10 - 1f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 20 - 2f */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, /* 30 - 3f */
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 40 - 4f */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 50 - 5f */
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 60 - 6f */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 70 - 7f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 80 - 8f */
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 90 - 9f */
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* a0 - af */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* b0 - bf */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* c0 - cf */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* d0 - df */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* e0 - ef */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 /* f0 - ff */
};

int CHAR_SPACE[256] =
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* 0x09(TAB), 0x0A(LF), 0x0C(FF), 0x0D(CR), 0x20(SPACE) */
char g_whitespace_map[] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const unsigned char G_MAP[256] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
                177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };

static const unsigned char CHAR_ALPHA_LOW[256] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
                177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };

/**
 * 大写转小写(字符)
 */
char tolower(unsigned char chr)
{
	return CHAR_ALPHA_LOW[chr];
}

/**
 * 大写转小写(字符串)
 */
int trans2lower(char* lower, char* upper)
{
	if ((lower == NULL) || (upper == NULL))
	{
		return 0;
	}
	while (*lower)
	{
		*upper = CHAR_ALPHA_LOW[(unsigned char) *lower];
		upper++;
		lower++;
	}
	*upper = 0;
	return 1;
}

/**
 * @brief 转化为半角. 输出和输入buffer可相同.
 * @return int, 转换后的长度
 **/
int trans2bj(const char *in, char *out)
{
	int i = 0;
	int j = 0;
	while (in[j])
	{
		if (strncmp(in + j, "！", 2) >= 0 && strncmp(in + j, "｝", 2) <= 0)
		{
			/* 0301 - 0393 */
			out[i] = in[j + 1] - 0xa0 + ' ';
			j++;
		}
		else
		{
			out[i] = in[j];
		}
		i++;
		j++;
	}
	out[i] = 0;
	return i;
}

/**
 * @brief 转化为半角和小写. 输出和输入buffer可相同.
 **/
void trans2bj_lower(const char *in, char *out)
{
	trans2bj(in, out);
	trans2lower(out, out);
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

int getTextSize(const char *src, int &cn_num)
{
	cn_num = 0;
	int len;
	int i;
	if (!src)
	{
		return 0;
	}
	len = 0;
	i = 0;
	while (src[i] != '\0')
	{
		int l;
		//skip转义字符
		if (src[i] == '&')
		{
			l = check_single_reference_char(src + i);
			if (l > 0)
			{
				//if (l != 6)
				{
					len++;
				}
				i += l;
				if (src[i] == ';')
				{
					i++;
				}
			}
			else
			{
				int k = i + 10;
				int j = i + 1;
				if (src[j] == '#')
				{
					j++;
				}
				for (; j < k; j++)
				{
					if ((src[j] >= 'a' && src[j] <= 'z') || (src[j] >= 'A' && src[j] <= 'Z') || (src[j] >= '0' && src[j] <= '9'))
					{
					}
					else
					{
						break;
					}
				}
				if (j >= i + 3 && src[j] == ';')
				{
					len += 2;
					i = j + 1;
				}
				else
				{
					len++;
					i++;
				}
			}
		}
		else if ((l = GET_CHR_LEN(src+i)) >= 2)
		{
			len += 2;
			i += l;
			cn_num++;
		}
		else
		{
			if (src[i] != ' ' && src[i] != '\n' && src[i] != '\r' && src[i] != '\t' && src[i] != '\f' && src[i] != '\v')
			{
				len++;
			}
			i++;
		}
	}
	return len;
}

/**
 * @brief	根据编码区间判断是否有用的字. 跟编码相关.
 **/
int is_valid_word(const char *p)
{
	unsigned char h = (unsigned char) *p;
	unsigned char l = (unsigned char) *(p + 1);
	if (h >= 0xB0 && h <= 0xF7 && l >= 0xA1 && l <= 0xFE)
	{ //GBK汉字区
		return 1;
	}
	if (h >= 0x81 && h <= 0xA0 && l >= 0x40 && l <= 0xFE)
	{ //CJK汉字
		return 1;
	}
	if (h >= 0xAA && h <= 0xFE && l >= 0x40 && l <= 0xA0)
	{ //增补汉字
		return 1;
	}
	return 0;
}

/**
 * @brief	是否空白字符,展示在页面的空白字符
 **/
bool is_space_text(const char *text)
{
	for (const char *p = text; *p;)
	{
		if (isspace(*p))
		{
			p++;
		}
		else if (*p == '&' && strncmp(p + 1, "nbsp", strlen("nbsp")) == 0)
		{
			p += strlen("&nbsp");
			if (*p == ';')
			{
				p++;
			}
		}
		else if (strncmp(p, "　", strlen("　")) == 0)
		{
			p += strlen("　");
		}
		else
		{
			return false;
		}
	}
	return true;
}

const char *skip_space(const char *pstr)
{
	const char *pt = pstr;
	while (g_whitespace_map[(unsigned char) (*pt)])
		pt++;
	while (1)
	{
		if (g_whitespace_map[(unsigned char) (*pt)])
		{
			pt++;
		}
		else if (strncmp(pt, "	", strlen("	")) == 0)
		{
			pt += strlen("	");
		}
		else
			break;
	}
	return pt;
}

bool is_only_space_between(const char *begin, const char *end)
{
	const char *p = skip_space(begin);
	return (p == end);
}

/**
 * @brief 字符是否数字. 实验表明，这样写比试过的其他方式要快.
 **/
bool q_isdigit(const char ch)
{
	switch (ch)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return true;
	default:
		return false;
	}
}

/**
 * @brief	删除空白字符，判断word中是否含有spec_words中的某个串
 **/
bool is_has_special_word(const char * spec_words[], const char * word)
{
	/*输入框的提示信息不会太长*/
	const int buffer_len = 32;
	const int min_buffer_len = 4;
	char buffer[buffer_len];
	buffer[0] = '\0';
	int len = strlen(word);
	if (len >= buffer_len || len < min_buffer_len)
	{
		return false;
	}

	/*去除所有空格*/
	char * pdes = buffer;
	const unsigned char * psrc = (const unsigned char *) word;
	while (*psrc)
	{
		if (!IS_GBK(psrc))
		{
			if (*psrc >= 0x80 && psrc[1] == 0)
			{
				break;
			}
			if (isspace(*psrc))
			{
				psrc++;
			}
			else
			{
				*pdes++ = *psrc++;

			}
		}
		else
		{ //gbk 2字节
			if (IS_GB_SPACE(psrc))
			{
				psrc += 2;
			}
			else
			{
				*pdes++ = *psrc++;
				*pdes++ = *psrc++;
			}
		}
	}
	*pdes = 0;
	/*strstr 看是否在特殊词列表中出现*/
	for (int i = 0; spec_words[i]; i++)
	{
		if (strstr(buffer, spec_words[i]) != NULL)
		{
			return true;
		}
	}
	return false;
}

/**
 * @brief 字符串拷贝
 **/
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0)
	{
		do
		{
			if ((*d++ = *s++) == 0)
			{
				break;
			}
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0)
	{
		if (siz != 0)
			*d = '\0'; /* NUL-terminate dst */
		while (*s++)
			;
	}

	return (size_t)(s - src - 1); /* count does not include NUL */
}

/**
 * @brief 最长公共子串. 返回公共子串的长度，可能与编码有关.
 **/
int longest_common_substring(const char *l, const char *r)
{
	static const int LIS_ARR_SIZE = 128;
	short lis_arr[2][LIS_ARR_SIZE];
	int i = 0;
	int j = 0;
	int cur_row_no = 1;
	int prev_row_no = 0;

	for (i = 0; l[i]; i++)
	{
		cur_row_no = 1 - cur_row_no;
		prev_row_no = 1 - prev_row_no;
		for (j = 0; r[j] && j < LIS_ARR_SIZE; j++)
		{
			int max = 0;
			if (i > 0)
				max = lis_arr[prev_row_no][j];
			if (j > 0 && lis_arr[cur_row_no][j - 1] > max)
				max = lis_arr[cur_row_no][j - 1];
			int ext_val = 0;
			if (l[i] == r[j])
				ext_val = 1;
			if (i > 0 && j > 0)
				ext_val += lis_arr[prev_row_no][j - 1];
			if (ext_val > max)
				max = ext_val;
			lis_arr[cur_row_no][j] = max;
		}
	}

	if (i == 0 || j == 0)
	{
		return 0;
	}
	return lis_arr[cur_row_no][j - 1];
}

/**
 * @brief 过掉字符串内的空白，英文单词间空白除外. 返回转换后的长度.
 **/
int trim_space(char *buf)
{
	const char *p = buf;
	char *q = buf;

	while (*p)
	{
		if (*p == ' ' && (GET_CHR_LEN(p+1) >= 2 || *(p + 1) == '\0' || *(p + 1) == ' '))
		{
			p++;
		}
		else
		{
			for (unsigned int i = 0; i < GET_CHR_LEN(p); i++)
			{
				*q++ = *p++;
			}
		}
	}

	*q = '\0';
	return q - buf;
}

/**
 * @brief 裁剪字符串两边的空格， 如果中间有空格则会截取到中间的第1个空格处。在原字符串上修改。
 * @return int, 返回转换后的长度
 **/
int str_trim_side_space(char *str)
{
	char *p = str;
	for (; *p; p += GET_CHR_LEN(p))
	{
		if (!isspace(*p) && strncmp(p, "　", strlen("　")) != 0)
			break;
	}
	char *new_end = p;
	for (char *q = p; *q; q += GET_CHR_LEN(q))
	{
		if (!isspace(*q) && strncmp(q, "　", strlen("　")) != 0)
			new_end = q + GET_CHR_LEN(q);
	}
	for (const char *q = p; q < new_end; q++, p++)
	{
		*p = *q;
	}
	*p = '\0';
	return p - str;
}

/*判断一个串是不是时间串*/
static bool is_time_unit(const char * str)
{
	int num = atoi(str);
	if ((num >= 1970 && num <= 2100) //年
	|| (num >= 1 && num <= 12) //月
	                || (num >= 1 && num <= 31) //天
	                || (num >= 0 && num < 24) //小时
	                || (num >= 0 && num < 60) //分钟，秒
	                )
	{
		return true;
	}
	return false;
}

typedef struct _time_pos_t
{
	const unsigned char * pos;
	int len;
} time_pos_t;

static bool is_time_str_inner(time_pos_t * tp, int n)
{
	assert(tp != NULL);
	if (n <= 1)
	{
		return false;
	}
	int del_num = 0;
	int valid_num = 0;
	for (int i = 1; i < n - 1; i++)
	{
		int skip1 = tp[i].pos - (tp[i - 1].pos + tp[i - 1].len);
		int skip2 = tp[i + 1].pos - (tp[i].pos + tp[i].len);
		if (skip1 == skip2 && skip1 <= 2 && skip2 <= 2)
		{
			valid_num++;
		}
		if (skip1 > 2 && skip2 > 2)
		{
			del_num++;
		}
	} //end for
	if (tp[0].pos + tp[0].len + 2 < tp[1].pos)
	{
		del_num++;
	}
	if (tp[n - 2].pos + tp[n - 2].len + 2 < tp[n - 1].pos)
	{
		del_num++;
	}
	if (del_num < n && valid_num > 0)
	{
		return true;
	}
	return false;
}

/**
 * @brief	是否是一个表示时间的字符串
 **/
bool is_time_str(const char * str_time)
{
	if (str_time == NULL || *str_time == 0)
	{
		return false;
	}
	int str_len = strlen(str_time);
	if (str_len > MAX_TIME_STR_LEN)
	{
		return false;
	}
	char time_buff[MAX_TIME_STR_LEN];
	time_buff[0] = '\0';
//	snprintf(time_buff, sizeof(time_buff) , "%s" , str_time ) ;
	copy_html_text(time_buff, 0, MAX_TIME_STR_LEN, (char*) str_time);
	str_len = strlen(time_buff);
	unsigned char * looper = (unsigned char *) time_buff;
	trans2bj(time_buff, time_buff);
	unsigned char * p_begin = NULL;
	time_pos_t time_pos[MAX_TIME_UNIT_NUM];
	memset(time_pos, 0, sizeof(time_pos));
	int time_pos_i = 0;
	int all_digit_num = 0;
	int seg_digit_num = 0;
	unsigned char *seg_start_pos = (unsigned char*) time_buff;
	while (*looper)
	{
		if (!IS_GBK(looper))
		{
			if (looper[0] > 0x80 && looper[1] == 0)
			{
				break;
			}
			int num = 0;
			if (q_isdigit(*looper))
			{ //连续字符个数
				p_begin = looper;
				looper++;
				num++;
				while (q_isdigit(*looper))
				{
					looper++;
					num++;
				}
				all_digit_num += num;
				seg_digit_num += num;
				if (num < 5)
				{ //过滤连续数字大于5的串
					char tmp[8] =
					{ 0 };
					strncpy(tmp, (char*) p_begin, num);
					if (is_time_unit(tmp))
					{
						time_pos[time_pos_i].pos = p_begin;
						time_pos[time_pos_i].len = num;
						time_pos_i++;
					}
					if (time_pos_i >= MAX_TIME_UNIT_NUM)
					{ //只统计前MAX_TIME_UNIT_NUM个时间格式
						break;
					}
				}

			}
			else
			{
				if (*looper == ' ')
				{
					if (seg_digit_num == 0)
					{
						int offset = looper - seg_start_pos;
						str_len -= offset;
					}
					seg_start_pos = looper;
					seg_digit_num = 0;
				}
				looper++;
			}
		}
		else
		{

			looper += 2;
		}
	}
	if (seg_digit_num == 0)
	{
		int offset = looper - seg_start_pos;
		str_len -= offset;
	}
	if (all_digit_num * 10 < str_len * 3)
	{

		return false;
	}
	return is_time_str_inner(time_pos, time_pos_i);
}

int remove_tail_rnt(char * line)
{
	//指针跳到末尾
	char *p = line;
	while (*p != '\0')
	{
		p++;
	}

	while (p > line && (*(p - 1) == '\r' || *(p - 1) == '\t' || *(p - 1) == '\n'))
	{
		*(p - 1) = '\0';
		p--;
	}

	return 1;
}

/*双字节1区 0xA1A0~ 0xAFFF*/
unsigned char double_a1_table[2][6][16] =
{
{
{ 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF },
{ 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF },
{ 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF },
{ 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF },
{ 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF },
{ 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF } },
{
{ 0x0, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7e, 0x0, 0x0, 0x0, 0x0 },
{ 0x0, 0x0, 0x5b, 0x5d, 0x3c, 0x3e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 },
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } } };

int get_next_gb18030_bytes(unsigned char* s)
{
	unsigned char *p = s;
	if ((unsigned char) *p <= 0x7F) // 1 bytes
	{
		return 1;
	}
	else if ((unsigned char) *p >= 0x81 && (unsigned char) *p <= 0xFE)
	{
		if ((unsigned char) *(p + 1) >= 0x30 && (unsigned char) *(p + 1) <= 0x39)
			return 4;
		if ((unsigned char) *(p + 1) >= 0x40)
			return 2;
	}
	return -1;
}

int trans_full_to_half(const char *src, const int srcLen, char *dest, int *destLen)
{
	if (NULL == src)
		return -1;
	unsigned char* p = (unsigned char*) (src);
	char* pd = dest;
	unsigned char hc = 0;
	*destLen = 0;
	int blen = 0;
	unsigned char* pEnd = (unsigned char*) (src + srcLen);

	while (*p && ((blen = get_next_gb18030_bytes(p)) <= (pEnd - p)) && (blen >= 0))
	{
		if (*(p + 1) >= 0xA0 && (unsigned char) *p == (unsigned char) 0xA1)
		{ // 0xA1A1-0xA1FE之间的全角转半角字符
			if ((hc = double_a1_table[1][(*(p + 1) - 0xA0) / 16][(*(p + 1) - 0xA0) % 16]) != 0)
			{
				*pd++ = hc;
				p += 2;
			}
			else
			{
				*pd++ = *p++;
				*pd++ = *p++;
			}

		}
		else if (((unsigned char) *(p + 1) >= 0xA1) && (unsigned char) *p == (unsigned char) 0xA3) //0xA3A1-0xA3FF
		{
			*pd++ = *(p + 1) - (unsigned char) 0x80;
			p += 2;
		}
		else
		{
			for (int i = 0; i < blen; i++)
				*pd++ = *p++;
		}
		if (p >= pEnd)
			break;
	}
	*pd++ = 0; //'\0'
	*destLen = pd - dest;
	return (*destLen);
}

/**
 * @brief 只保留中文汉字
 * @param src [in], 输入字符串
 * @param srcLen [in], src的长度
 * @param dest [in/out], 保存只保留汉字的字符串
 * @param destLen [out], dest的长度
 * @return dest保存的字符串的长度
 */
int remain_only_chinese(const char *src, const int srcLen, char *dest, int *destLen)
{
	if (NULL == src)
		return -1;
	unsigned char* p = (unsigned char*) (src);
	char* pd = dest;
	unsigned char hc = 0;
	*destLen = 0;
	int blen = 0;
	unsigned char* pEnd = (unsigned char*) (src + srcLen);

	while (*p && ((blen = get_next_gb18030_bytes(p)) <= (pEnd - p)) && (blen >= 0))
	{
		if (*(p + 1) >= 0xA0 && (unsigned char) *p == (unsigned char) 0xA1)
		{ // 0xA1A1-0xA1FE之间的全角字符丢弃
			if ((hc = double_a1_table[1][(*(p + 1) - 0xA0) / 16][(*(p + 1) - 0xA0) % 16]) != 0)
			{
//				*pd++ = hc;
				p += 2;
			}
			else
			{ //汉字保留
				*pd++ = *p++;
				*pd++ = *p++;
			}
		}
		else if (((unsigned char) *(p + 1) >= 0xA1) && (unsigned char) *p == (unsigned char) 0xA3) //0xA3A1-0xA3FF
		{ //全角字符丢弃
//			*pd++ = *(p + 1) - (unsigned char) 0x80;
			p += 2;
		}
		else
		{
			if (blen == 1)
			{ //单字节字符和数字丢弃
				p++;
			}
			else
			{
				for (int i = 0; i < blen; i++)
					*pd++ = *p++;
			}
		}
		if (p >= pEnd)
			break;
	}
	*pd++ = 0; //'\0'
	*destLen = pd - dest;
	return (*destLen);
}
