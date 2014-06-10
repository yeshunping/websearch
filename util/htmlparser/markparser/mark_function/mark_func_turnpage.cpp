/*
 * mark_func_turnpage.cpp
 *	Description: 翻页链接块标记
 *  Created on: 2012-10-17
 *      Author: sue_zhang@staff.easou.com
 *     Version: 1.0
 */

#include "easou_url.h"
#include "easou_string.h"
#include "simplehashmap.h"
#include "easou_lang_relate.h"
#include "easou_html_attr.h"
#include "easou_vhtml_basic.h"
#include "easou_mark_switch.h"
#include "easou_mark_com.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "debuginfo.h"

#define CHECK_ANCHOR_LIMIT 30

struct turn_page_visit_t
{
	int digit_anchor_count; //数字锚文(比如 1 ,  [1], ［1］，(1) )个数
	int spec_word_count; //含有翻页特征字符串的锚文数
	int other_anchor_count; //其他锚文数
	int digit_area_count; //数字锚文区域个数
	int cur_page_num; //当前第几页

	/**<	遍历内部使用		*/
	bool in_anchor; //是否是链接中
	int in_pair_symbol; //被[ ]括起来的数字
	int last_digit; //最后一个被访问的数字锚文值
	int area_last_digit; //连续数字锚文区域的最后一个数字锚文值
	int area_first_digit; //连续数字锚文区域的第一个数字锚文值
	int pic_link_num; // 图片链接个数
	int anti_num; //非翻页块特征文本数
};

struct PairSymbol_T
{
	char *headChar;
	char *tailChar;
};

static PairSymbol_T g_pairSymbols[] =
{
{ "[", "]" },
{ "(", ")" },
{ "【", "】" } };

static char* spec_words[] =
{ "首页", "第一页", "第1页", "上页", "上一页", "下页", "下一页", "上1页", "下1页", "上一篇", "下一篇", "尾页" };
int spec_words_num = sizeof(spec_words) / sizeof(spec_words[0]);

static char* anti_words[] =
{ "截图" };
int anti_words_num = sizeof(anti_words) / sizeof(anti_words[0]);

static char* spec_words2[] =
{ "共1页" };
int spec_words_num2 = sizeof(spec_words2) / sizeof(spec_words2[0]);

static int g_pairSymbols_num = sizeof(g_pairSymbols) / sizeof(PairSymbol_T);

static int get_page_no(const char* pTextStart, const char* pTextEnd)
{
	int len = pTextEnd - pTextStart + 1;
	if (len <= 0 || len >= CHECK_ANCHOR_LIMIT)
	{
		return 0;
	}
	//全角转半角
	char buffer[CHECK_ANCHOR_LIMIT] =
	{ 0 };
	memcpy(buffer, pTextStart, len);
	int newLen = easou_trans2bj(buffer, buffer);
	if (newLen < 1)
	{
		return 0;
	}
	const char *pStart = buffer;
	const char *pEnd = pStart + newLen - 1;

	//跳过开始的.
	while ((pStart != pEnd) && *pStart == '.')
	{
		pStart++;
	}
	//跳过结尾的.
	while ((pEnd != pStart) && *pStart == '.')
	{
		pEnd--;
	}
	if (pStart == pEnd && !isdigit(*pStart))
	{
		return 0;
	}

	if (!isdigit(*pStart))
	{ //第1个字符不是数字
		for (int i = 0; i < g_pairSymbols_num; i++)
		{
			if (strstr(pStart, g_pairSymbols[i].headChar) == pStart && strstr((pEnd - strlen(g_pairSymbols[i].tailChar) - 1), g_pairSymbols[i].tailChar))
			{
				return atoi(pStart + strlen(g_pairSymbols[i].headChar));
			}
		}
	}
	else
	{
		const char *p = pStart;
		while (p <= pEnd && isdigit(*p))
		{
			p++;
		}
		if (p > pEnd)
		{
			//都是数字
			return atoi(pStart);
		}
	}
	return 0;
}

static int startVisitForTurnPageInfo(html_vnode_t* vnode, void* result)
{
	if (!vnode->isValid)
	{
		return VISIT_SKIP_CHILD;
	}
	html_tag_t *tag = &vnode->hpNode->html_tag;
	turn_page_visit_t *turnPageInfo = (turn_page_visit_t*) result;
	if (tag->tag_type == TAG_A)
	{
		char *pHref = get_attribute_value(tag, "href");
		if (pHref && strstr(pHref, "javascript:"))
		{
			turnPageInfo->other_anchor_count++;
			return VISIT_NORMAL;
		}
		turnPageInfo->in_anchor = true;
		return VISIT_NORMAL;
	}
	char *src = NULL;
	int srcLen = 0;
	if (turnPageInfo->in_anchor && tag->tag_type == TAG_IMG)
	{
		src = get_attribute_value(tag, ATTR_TITLE);
		if (src == NULL)
		{
			turnPageInfo->pic_link_num++;
		}
	}
	if (turnPageInfo->in_anchor && tag->tag_type == TAG_PURETEXT)
	{ //锚文
		src = tag->text;
	}
	else if (tag->tag_type == TAG_PURETEXT)
	{ //纯文本
		for (int i = 0; i < spec_words_num2; i++)
		{
			if (strstr(tag->text, spec_words2[i]))
			{
				turnPageInfo->spec_word_count++;
				return VISIT_NORMAL;
			}
		}
		//统计含有非翻页特征文本数
		for (int i = 0; i < anti_words_num; i++)
		{
			if (strstr(tag->text, anti_words[i]))
			{
				turnPageInfo->anti_num++;
			}
		}
		if (vnode->cn_num == 0)
		{
			src = tag->text;
		}
	}
	if (!src)
	{
		return VISIT_NORMAL;
	}
	srcLen = strlen(src);

	//跳过太长的锚文
	if (srcLen >= CHECK_ANCHOR_LIMIT)
	{
		if (turnPageInfo->in_anchor)
		{
			turnPageInfo->other_anchor_count++;
		}
		return VISIT_NORMAL;
	}

	char buffer[CHECK_ANCHOR_LIMIT];
	int len = copy_html_text(buffer, 0, CHECK_ANCHOR_LIMIT, src);
	if (len == 0)
	{
		return VISIT_NORMAL;
	}
	*(buffer + len) = 0;

	//跳过开始的空格
	char *pStart = buffer;
	char *pEnd = buffer + len - 1;
	while ((pStart != pEnd) && isspace(*pStart))
	{
		pStart++;
	}
	//跳过结尾的空格
	while ((pEnd != pStart) && isspace(*pEnd))
	{
		pEnd--;
	}
	if (pStart == pEnd && !isdigit(*pStart))
	{
		return VISIT_NORMAL;
	}

	//统计含有翻页特征字符串的锚文个数
	for (int i = 0; i < spec_words_num; i++)
	{
		if (strstr(pStart, spec_words[i]))
		{
			turnPageInfo->spec_word_count++;
			return VISIT_NORMAL;
		}
	}

	int pageNo = get_page_no(pStart, pEnd);
	if (pageNo > 0)
	{
		//统计数字锚文个数
		if (turnPageInfo->in_anchor)
		{
			turnPageInfo->digit_anchor_count++;
		}
		else if (pageNo == 1 && turnPageInfo->area_first_digit == 0)
		{
			turnPageInfo->cur_page_num = pageNo;
		}
		else if (turnPageInfo->area_first_digit > 0 && pageNo >= turnPageInfo->area_first_digit)
		{
			turnPageInfo->cur_page_num = pageNo;
		}
		if (pageNo == turnPageInfo->last_digit + 1 && turnPageInfo->last_digit > 0)
		{
			turnPageInfo->area_last_digit = pageNo;
			if (turnPageInfo->area_first_digit > 0 && turnPageInfo->area_last_digit == turnPageInfo->area_first_digit + 1)
			{
				turnPageInfo->digit_area_count++;
			}
		}
		else if (pageNo > 0)
		{
			turnPageInfo->area_first_digit = pageNo;
		}
		turnPageInfo->last_digit = pageNo;
	}
	else
	{
		turnPageInfo->other_anchor_count++;
		turnPageInfo->area_first_digit = 0;
		turnPageInfo->area_last_digit = 0;
		turnPageInfo->last_digit = 0;
	}
	return VISIT_NORMAL;
}

static int finishVisitForTurnPageInfo(html_vnode_t* vnode, void* result)
{
	turn_page_visit_t *turnPageInfo = (turn_page_visit_t*) result;
	if (vnode->hpNode->html_tag.tag_type == TAG_A)
	{
		turnPageInfo->in_anchor = false;
	}
	return VISIT_NORMAL;
}

static int start_mark_turnpage(html_area_t *area, mark_area_info_t *mark_info)
{
	if (!area->isValid)
	{
		marktreeprintfs(MARK_TURNPAGE, "skip the area(id=%d) for it is not valid at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return VISIT_SKIP_CHILD;
	}
	int height = area->area_info.height;
	int inlink_num = area->baseinfo->link_info.inner_num;
	int text_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	int position = area->pos_plus;
	if (position == IN_PAGE_LEFT || position == IN_PAGE_RIGHT || position == IN_PAGE_FOOTER)
	{
		marktreeprintfs(MARK_TURNPAGE, "skip the area(id=%d) for it is not IN_PAGE_MAIN at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return VISIT_SKIP_CHILD;
	}
	if (area->valid_subArea_num == 0)
	{
		if (height >= 60)
		{
			marktreeprintfs(MARK_TURNPAGE, "the area(id=%d) is not for it is too high at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return VISIT_NORMAL;
		}
		if (text_size >= 100)
		{
			marktreeprintfs(MARK_TURNPAGE, "the area(id=%d) is not for it is has too many words at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return VISIT_NORMAL;
		}
		turn_page_visit_t turnPageInfo;
		memset(&turnPageInfo, 0, sizeof(turn_page_visit_t));
		turnPageInfo.in_pair_symbol = -1;
		for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
		{
			html_vnode_visit(vnode, startVisitForTurnPageInfo, finishVisitForTurnPageInfo, &turnPageInfo);
			if (vnode == area->end)
			{
				break;
			}
		}
		marktreeprintfs(MARK_TURNPAGE, "the area(id=%d) maybe turn page area[digit_anchor:%d, spec_word_anchor:%d, pic_anchor:%d, other_anchor:%d, digit_area:%d, cur_page:%d, start:%d, end:%d] at %s(%d)-%s\r\n", area->no, turnPageInfo.digit_anchor_count, turnPageInfo.spec_word_count, turnPageInfo.pic_link_num, turnPageInfo.other_anchor_count, turnPageInfo.digit_area_count, turnPageInfo.cur_page_num, turnPageInfo.area_first_digit, turnPageInfo.area_last_digit, __FILE__, __LINE__, __FUNCTION__);

		bool isTurnPageArea = false;
		if (turnPageInfo.anti_num == 0 && (turnPageInfo.digit_area_count == 1 || turnPageInfo.digit_area_count == 2) && turnPageInfo.pic_link_num <= 2)
		{
			if ((turnPageInfo.digit_anchor_count + turnPageInfo.spec_word_count + turnPageInfo.pic_link_num) >= inlink_num - 2)
			{
				isTurnPageArea = true;
			}
		}
		if (isTurnPageArea)
		{
			tag_area_func(area, AREA_FUNC_TURNPAGE);
		}
	}
	return AREA_VISIT_NORMAL;
}

bool mark_func_turnpage(area_tree_t *atree)
{
	bool ret = areatree_visit(atree, (FUNC_START_T) start_mark_turnpage, NULL, atree->mark_info);
	return ret;
}

