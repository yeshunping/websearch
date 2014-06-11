
#include <stdlib.h>
#include <string.h>

#include "base/logging.h"
#include "css_dtd.h"
#include "css_parser.h"
#include "css_parser_inner.h"
#include "util/htmlparser/htmlparser/html_dtd.h"
#include "util/htmlparser/utils/url.h"
#include "util/htmlparser/utils/string_util.h"
#include "DownLoad.h"

/**
 * @brief .
 * @param [in/out] css   : css_t*
 * @param [in/out] len   : size_t
 * @return  int

 * @date 2011/06/20
 **/
int adjust_str_heap(css_t *css, size_t len)
{
	if (css->css_inner.str_heap.heap_size < len)
	{
		char *origin_mem = css->css_inner.str_heap.p_heap;
		css->css_inner.str_heap.p_heap = (char *) realloc(css->css_inner.str_heap.p_heap, len);
		if (css->css_inner.str_heap.p_heap == NULL)
		{
			css->css_inner.str_heap.p_heap = origin_mem;
			LOG(ERROR) << "realloc error!";
			return -1;
		}
		//TODO free origin_mem
		css->css_inner.str_heap.heap_size = len;
		css->css_inner.str_heap.p_heap_avail = css->css_inner.str_heap.p_heap;
	}
	return 0;
}

/**
 * @brief 判断是否是css中的空格
 * @param [in/out] css   : css_t*
 * @param [in/out] len   : size_t
 * @return  int

 * @date 2011/06/20
 **/
static inline bool is_css_space(char chr)
{
	return g_whitespace_map[(unsigned char)(chr)];
}

/**
 * @brief 跳过comment
 *  assert(*pstr == '/' && *(pstr+1) == '*');
 * @param [in] pstr   : const char* 起始点
 * @return  const char* 截止点

 * @date 2011/06/20
 **/
static const char *skip_comment(const char *pstr)
{
	const char *pt = pstr;
	pt += 2;
	while (*pt != '\0')
	{
		if (*pt == '*' && *(pt + 1) == '/')
		{
			pt += 2;
			break;
		}
		pt++;
	}
	return pt;
}

/**
 * @brief css中的跳过commemt.
 * @return  const char* 截止点

 * @date 2011/06/20
 **/
const char *css_skip_comment(const char *pstr)
{
	if ('/' == *pstr && '*' == *(pstr + 1))
	{
		return skip_comment(pstr);
	}

	return pstr;
}

/**
 * @brief css中的跳过一块{}包含着的.
 * @return  const char* 截止点

 * @date 2011/06/20
 **/
const char *skip_block(const char *pstr)
{
	int n_lbracket = 0;
	const char *pt = pstr;
	while (*pt)
	{
		if (*pt == '{')
		{
			n_lbracket++;
		}
		else if (*pt == '}')
		{
			if (--n_lbracket <= 0)
			{
				pt++;
				break;
			}
		}
		pt++;
	}
	return pt;
}

/**
 * @brief 跳过一段规则 assert(*pstr == '@');
 * @param [in/out] pstr   : const char*
 * @return  const char*
 * @retval
 * @see

 * @date 2011/06/20
 **/
static const char *skip_at_rule(const char *pstr)
{
	const char *pt = pstr;
	while (*(++pt))
	{
		if (*pt == ';')
		{
			pt++;
			break;
		}
		else if (*pt == '{')
		{
			pt = skip_block(pt);
			break;
		}
	}
	return pt;
}

/**
 * @brief 跳过一段规则@import
 * 	if not at-rule, return the input pointer.
 * @param [in] pstr   : const char*
 * @return  const char*

 * @date 2011/06/20
 **/
const char *css_skip_at_rule(const char *pstr)
{
	if ('@' == *pstr)
	{
		return skip_at_rule(pstr);
	}

	return pstr;
}

/**
 * @brief 跳过当前属性
 * @param [in/out] pstr   : const char*
 * @return  const char*

 * @date 2011/06/20
 **/
const char* skip_current_prop(const char *pstr)
{
	const char *pt = pstr;
	while (*pt)
	{
		if (*pt == ';' || *pt == '}')
			break;
		pt++;
	}
	return pt;
}

/**
 * @brief 跳过当前的选择子.

 * @date 2011/06/20
 **/
const char *skip_current_selector(const char *pstr)
{
	const char *pt = pstr;
	while (*pt)
	{
		if (*pt == ',' || *pt == '{')
			break;
		pt++;
	}
	return pt;
}

/**
 * @brief 扫描字符串 assert(*(css_scan->p_next) == '"' || *(css_scan->p_next) == '\'')
 * @param [in/out] css_scan   : css_scan_t*
 * @param [in] str_heap   : css_str_heap_t*
 * @return  void
 * @retval
 * @see

 * @date 2011/06/20
 **/
void scan_string(css_scan_t *css_scan, css_str_heap_t *str_heap)
{
	char sep_chr = *(css_scan->p_next);
	css_scan->p_next++;
	while (*(css_scan->p_next) != '\0' && (*(css_scan->p_next) == sep_chr && *(css_scan->p_next - 1) != '\\') == false)
	{
		*(str_heap->p_heap_avail++) = *(css_scan->p_next++);
	}
	if (*(css_scan->p_next) == sep_chr)
		css_scan->p_next++;
}

static const int css_selector_sep_chr_map[256] =
{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, //0~15 0x09(TAB), 0x0A(LF), 0x0C(FF), 0x0D(CR)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16~31 a
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, //32~47 32(space) 43(+) 44(,)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, //48~63  62(>) a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //64~79 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80~95 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //96~111 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //112~127  123({) a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //128~143 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //144~159 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160~175 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //176~191 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //192~207 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //208~223 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //224~239 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240~255 a
};

static const int css_prop_name_sep_chr_map[256] =
{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, //0~15 0x09(TAB), 0x0A(LF), 0x0C(FF), 0x0D(CR)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16~31 a
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //32~47 32(space)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, //48~63  58(:) 59(;) a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //64~79 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80~95 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //96~111 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, //112~127  125(}) a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //128~143 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //144~159 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160~175 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //176~191 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //192~207 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //208~223 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //224~239 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240~255 a
};

static const int css_prop_value_sep_chr_map[256] =
{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0~15 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16~31 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //32~47 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //48~63  59(;) a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //64~79 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80~95 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //96~111 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, //112~127  125(}) a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //128~143 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //144~159 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160~175 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //176~191 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //192~207 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //208~223 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //224~239 a
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240~255 a
};

/**
 * @brief

 * @date 2011/06/20
 **/
bool is_css_sep_chr_state(css_state_t state, char chr)
{
	switch (state)
	{
	case CSS_STAT_SCAN_SELECTOR:
		if (css_selector_sep_chr_map[(unsigned char)chr])
			return true;
		break;
	case CSS_STAT_SCAN_PROP_NAME:
		if (css_prop_name_sep_chr_map[(unsigned char)chr])
			return true;
		break;
	case CSS_STAT_SCAN_PROP_VALUE:
		if (css_prop_value_sep_chr_map[(unsigned char)chr])
			return true;
		break;
	default:
		break;
	}
	return false;
}

/**
 * @brief	扫描属性

 * @date 2011/06/20
 **/
void scan_attr(css_scan_t *css_scan, css_str_heap_t *str_heap)
{
	while (*(css_scan->p_next) != '\0' && *(css_scan->p_next) != ']')
	{
		if (is_css_space(*(css_scan->p_next)) == false)
		{
			*(str_heap->p_heap_avail++) = *(css_scan->p_next++);
		}
		else
		{
			css_scan->p_next++;
		}
	}
}

/**
 * @brief

 * @date 2011/06/20
 **/
inline void scan_a_step(css_scan_t *css_scan, css_str_heap_t *str_heap)
{
	if (*css_scan->p_next == '/' && *(css_scan->p_next + 1) == '*')
	{
		css_scan->p_next = skip_comment(css_scan->p_next);
	}
	else
	{
		*(str_heap->p_heap_avail++) = *(css_scan->p_next++);
	}
}

/**
 * @brief

 * @date 2011/06/20
 **/
void scan_normal(css_scan_t *css_scan, css_str_heap_t *str_heap)
{
	while (*(css_scan->p_next) != '\0' && is_css_sep_chr_state(css_scan->state, *(css_scan->p_next)) == false)
	{
		if (*(css_scan->p_next) == '[')
		{
			scan_attr(css_scan, str_heap);
		}
		else
		{
			scan_a_step(css_scan, str_heap);
		}
	}
}

/**
 * @brief

 * @date 2011/06/20
 **/
void scan_url(css_scan_t *css_scan, css_str_heap_t *str_heap)
{
	while (*(css_scan->p_next) != '\0' && *(css_scan->p_next) != '\t' && *(css_scan->p_next) != '\r' && *(css_scan->p_next) != '\n'
			&& *(css_scan->p_next) != '\f' && !(*(css_scan->p_next) == ')' && *(css_scan->p_next - 1) != '\\'))
	{
		*(str_heap->p_heap_avail++) = *(css_scan->p_next++);
	}
}

static const int scan_ignore_map[256] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0~15
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16~31
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, //32~47	-[45]	/[47]
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, //48~63	<[60]
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //64~79	@[64]
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80~95
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //96~111
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //112~127
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //128~143
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //144~159
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160~175
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //176~191
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //192~207
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //208~223
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //224~239
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240~255
		};

void scan_ignore(css_scan_t *css_scan, css_t *css)
{
	while (1)
	{
		css_scan->p_next = skip_space(css_scan->p_next);
		if (!scan_ignore_map[(unsigned char)(*(css_scan->p_next))])
			break;
		if (*(css_scan->p_next) == '@')
		{
			const char *p = skip_at_rule(css_scan->p_next);
			int skipLen = p - css_scan->p_next;
			if(skipLen >= 1000){
				css_scan->p_next = p;
				continue;
			}
			char importStr[1000] = "";
			memcpy(importStr, css_scan->p_next, skipLen);
			if (strstr(importStr, "@import"))
			{
				char importUrl[MAX_URL_SIZE] = "";
				char fullUrl[MAX_URL_SIZE] = "";
				char *p1 = strstr(importStr, "url(");
				char *p2 = strstr(importStr, ")");
				if (p1 && p2)
				{
					if(*(p2-1)=='"')
					{
						p2--;
					}
					*p2 = 0;
					p1+=4;
					if(*p1=='"')
					{
						p1++;
					}
					combine_url(importUrl, css_scan->css_url, p1);
					sprintf(fullUrl, "http://%s", importUrl);
					//Warn("import css:%s", fullUrl);
					if (css_scan->test_import && false)
					{
						DownLoad cDown;
						char buf[MAX_PAGE_SIZE] = "";
						char *pcsspos;
						int page_len = cDown.download(fullUrl, strlen(fullUrl), buf, MAX_PAGE_SIZE, pcsspos);
						if (page_len > 0)
						{
							LOG(INFO) << "download " << fullUrl << " success";
							//最后一个参数设置成false，只处理第一次import的css
							if (css_parse(css, pcsspos, importUrl, true, false) < 0)
							{
								//Warn("parse import css:%s error", importUrl);
							}
						}
					}
				}
			}
			css_scan->p_next = p;
		}
		else if (strncmp(css_scan->p_next, "/*", 2) == 0)
		{
			css_scan->p_next = skip_comment(css_scan->p_next);
		}
		else if (strncmp(css_scan->p_next, "<!--", 4) == 0)
		{
			css_scan->p_next += 4;
		}
		else if (strncmp(css_scan->p_next, "-->", 3) == 0)
		{
			css_scan->p_next += 3;
		}
		else
		{
			break;
		}
	}
}
