/**
 * easou_mark_pos.cpp
 *	Description: 标注分块的位置�1�7�包括每个分块相对父分块的位置，以及每个分块在页面中的相对位置和绝对位置〄1�7
 *  Created on: 2011-06-27
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easou.com
 *     Version: 1.1
 */
#include "easou_ahtml_tree.h"
#include "easou_mark_inner.h"
#include "easou_mark_pos.h"
#include "easou_mark_com.h"
#include "debuginfo.h"

/**
 * @brief 判断分块是否在页面的左边.
 * @param [in] page_width   : int	页面宽度
 * @param [in] area   : html_area_t*	待判断的分块
 * @return  bool	1：是＄1�7�否.
 * @author xunwu
 * @date 2011/07/13
 **/
static int is_left_area(int page_width, html_area_t *area)
{
	int right_border = area->area_info.xpos + area->area_info.width;
	if ((10 * right_border <= page_width * 3)||(area->area_info.xpos<10&&right_border*5<page_width*2)||(area->area_info.width*5<page_width*2&&get_right_area_in_page(area)&&!get_left_area_in_page(area)) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief 判断分块是否在页面的右边.
 * @param [in] page_width   : int	页面宽度
 * @param [in] area   : html_area_t*	待判断的分块
 * @return  bool	1：是＄1�7�否.
 * @author xunwu
 * @date 2011/07/13
 **/
static int is_right_area(int page_width, html_area_t *area)
{
	int left_border = area->area_info.xpos;
	if ((10 * left_border >= 7 * page_width )|| (left_border + area->area_info.width >= page_width && 3 * area->area_info.width <= page_width)||(area->area_info.width*5<page_width*2&&(!get_right_area_in_page(area))&&get_left_area_in_page(area)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief 判断分块是否在页面的头部.
 * @param [in] page_height   : int	页面高度
 * @param [in] area   : html_area_t*	待判断的分块
 * @return  bool	1：是＄1�7�否.
 * @author xunwu
 * @date 2011/07/13
 **/
static int is_head_area(int page_height, html_area_t *area)
{
	int under_border = area->area_info.ypos + area->area_info.height;
	int header_value = 0;
	if (page_height >= 750)
	{
		header_value = 380;
	}
	else if (page_height <= 300)
	{
		header_value = page_height / 2;
	}
	else
	{
		header_value = 200 + 5000 / (100 + (100 * area->area_info.height) / (area->area_info.width + 2));
	}
	if (under_border <= header_value)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief 判断分块是否在页面的底部.
 * @param [in] page_height   : int	页面高度
 * @param [in] area   : html_area_t*	待判断的分块
 * @return  bool	1：是＄1�7�否.
 * @author xunwu
 * @date 2011/07/13
 **/
static int is_foot_area(int page_height, html_area_t *area)
{
	int footer_value = 0;
	if (page_height < 300)
	{
		footer_value = page_height / 2;
	}
	else if (page_height <= 600) // small page
		footer_value = page_height / 3;
	else
	{
		footer_value = 180 + 5000 / (100 + (100 * area->area_info.height) / (area->area_info.width + 2));
	}
	if (area->area_info.ypos + footer_value >= page_height)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief 标记分块相对于页面的位置.
 * @param [in/out] area   : html_area_t *	待标注的分块.
 * @param [in] page_width   : int	页面宽度 〄1�7
 * @param [in] page_height   : int	页面长度.
 * @author xunwu
 * @date 2011/07/07
 **/
static void tag_area_pos(html_area_t *area, int page_width, int page_height)
{
	if (area->area_info.width <= 0 || area->area_info.height <= 0)
	{
		marktreeprintfs(MARK_POS, "skip the area(id=%d) for abspos_mark for it is invisible at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (is_head_area(page_height, area))
	{
		marktreeprintfs(MARK_POS, "mark the area(id=%d) PAGE_HEADER at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		area->abspos_mark = PAGE_HEADER;
	}
	else if (is_foot_area(page_height, area))
	{
		marktreeprintfs(MARK_POS, "mark the area(id=%d) PAGE_FOOTER at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		area->abspos_mark = PAGE_FOOTER;
	}
	else if (area->parentArea&&area->parentArea->abspos_mark==PAGE_LEFT||is_left_area(page_width, area))
	{
		marktreeprintfs(MARK_POS, "mark the area(id=%d) PAGE_LEFT at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		area->abspos_mark = PAGE_LEFT;
	}
	else if (area->parentArea&&area->parentArea->abspos_mark==PAGE_RIGHT||is_right_area(page_width, area))
	{
		marktreeprintfs(MARK_POS, "mark the area(id=%d) PAGE_RIGHT at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		area->abspos_mark = PAGE_RIGHT;
	}
	else
	{
		marktreeprintfs(MARK_POS, "mark the area(id=%d) PAGE_MAIN at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		area->abspos_mark = PAGE_MAIN;
	}
}

/**
 * @brief 结合绝对位置和相对父分块的位置�1�7�进行位置标评1�7
 * @param [in/out] area   : html_area_t *	待标注的分块.
 * @author xunwu
 * @date 2011/07/11
 **/
static void mark_pos_plus(html_area_t *area)
{
	if (area->depth == 1)
	{
		switch (area->pos_mark)
		{
		case RELA_LEFT:
			area->pos_plus = IN_PAGE_LEFT;
			marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_LEFT at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			goto __OVER;
		case RELA_RIGHT:
			area->pos_plus = IN_PAGE_RIGHT;
			marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_RIGHT at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			goto __OVER;
		case RELA_HEADER:
			area->pos_plus = IN_PAGE_HEADER;
			marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_HEADER at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			goto __OVER;
		case RELA_FOOTER:
			area->pos_plus = IN_PAGE_FOOTER;
			marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_FOOTER at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			goto __OVER;
		case RELA_MAIN:
			area->pos_plus = IN_PAGE_MAIN;
			marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_MAIN at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			goto __OVER;
		default:
			/**
			 * 必须已标记了相对位置
			 */
			assert(0);
			break;
		}
	}
	else if (area->depth == 2)
	{
		switch (area->pos_mark)
		{
		case RELA_LEFT:
			if (area->parentArea->pos_mark == RELA_MAIN)
			{
				area->pos_plus = IN_PAGE_LEFT;
				marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_LEFT at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
				goto __OVER;
			}
			break;
		case RELA_RIGHT:
			if (area->parentArea->pos_mark == RELA_MAIN)
			{
				area->pos_plus = IN_PAGE_RIGHT;
				marktreeprintfs(MARK_POS, "mark the area(id=%d) IN_PAGE_RIGHT at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
				goto __OVER;
			}
			break;
		default:
			break;
		}
	}
	/**
	 * 继承父分块，到了第三层，再�1�7�么折腾也改变不了自己在页面中的位置亄1�7
	 */
	area->pos_plus = area->parentArea->pos_plus;
	marktreeprintfs(MARK_POS, "mark the area(id=%d) pos_plus the same with parent area for its area depth>=3 at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
	__OVER: return;
}

static int start_visit_for_mark_pos(html_area_t * area, void * data)
{
	assert(area != NULL && data != NULL);
	if (area->isValid == false)
	{
		marktreeprintfs(MARK_POS, "skip the area(id=%d) for pos_mark and pos_plus mark for it is not valid at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	html_area_t * root = (html_area_t *) data;
	int page_height = root->area_info.height;
	int page_width = root->area_info.width;
	html_area_t * parent_area = area->parentArea;
	//第一屄1�7
	if (parent_area == NULL)
	{
		area->abspos_mark = PAGE_UNDEFINED;
		area->pos_plus = IN_PAGE_MAIN;
		marktreeprintfs(MARK_POS, "the area(id=%d) for pos_mark and pos_plus mark at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	//标记相对于页面的相对位置
	tag_area_pos(area, page_width, page_height);
	//结合相对于页面的位置和相对于父分块的位置，得出一个分块的综合位置
	mark_pos_plus(area);
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 标注绝对位置
 * @author xunwu
 * @date 2011/07/15
 **/
bool mark_pos_area(area_tree_t * atree)
{
	assert(atree!=NULL);
	if (IS_MARKED_POS(atree->mark_status))
	{
		return true;
	}
	html_area_t * root = atree->root;
	bool ret = areatree_visit(atree, start_visit_for_mark_pos, NULL, root);
	if (ret)
	{
		SET_MARKED_POS(atree->mark_status);
	}
	return ret;
}

/**
 * @brief 当前分块是否同一层在同一高度朄1�7右边的分坄1�7.
 * @author xunwu
 * @date 2011/07/15
 **/
static bool ismostright(html_area_t *subArea, html_area_t *superArea)
{
	int sTop = subArea->area_info.ypos;
	int sBot = subArea->area_info.ypos + subArea->area_info.height;
	for (html_area_t *area = superArea->subArea; area; area = area->nextArea)
	{
		if (area != subArea && area->isValid && area->area_info.width * 4 > subArea->area_info.width)
		{
			int top = area->area_info.ypos;
			int bot = area->area_info.ypos + area->area_info.height;
			if (bot > sTop && top < sBot)
				if (area->area_info.xpos > subArea->area_info.xpos)
					return false;
		}
	}

	return true;
}

#define HX_LIMIT_HEADER	200
#define HX_LIMIT_FOOTER	200

/**
 * @brief 标记丄1�7个分块的相对位置.
 * @author xunwu
 * @date 2011/07/11
 **/
static void markAreaRelaPos(html_area_t *subArea, html_area_t *superArea)
{
	int superWx = superArea->area_info.width;
	int superHx = superArea->area_info.height;
	int relaY = subArea->area_info.ypos + subArea->area_info.height - superArea->area_info.ypos;
	int relaRightX = superArea->area_info.xpos + superWx - subArea->area_info.xpos;
	int relaBotY = superArea->area_info.ypos + superHx - subArea->area_info.ypos;

	bool isVertical = (subArea->area_info.width * 100 < superWx * 30 && subArea->area_info.height * 100 >= superHx * 30) || (subArea->area_info.width * 100 < subArea->area_info.height * 80) || (subArea->area_info.width <= 200 && subArea->area_info.width <= subArea->area_info.height * 2);
	bool isHorizontal = (subArea->area_info.height * 100 < subArea->area_info.width * 40);

	if (isHorizontal)
	{
		int hxLimitHeader = (int) (superHx * 20);
		if (hxLimitHeader > HX_LIMIT_HEADER * 100)
			hxLimitHeader = HX_LIMIT_HEADER * 100;
		if (relaY * 100 <= hxLimitHeader)
		{
			marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_HEADER at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
			subArea->pos_mark = RELA_HEADER;
			return;
		}
		int hxLimitBot = (int) (superHx * 20);
		if (hxLimitBot > HX_LIMIT_FOOTER * 100)
			hxLimitBot = HX_LIMIT_FOOTER * 100;
		if (relaBotY * 100 <= hxLimitBot)
		{
			marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_FOOTER for its ypos relative to parent area is too low at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
			subArea->pos_mark = RELA_FOOTER;
			return;
		}
	}
	if (isVertical)
	{
		if (subArea->area_info.width <= 300 && (subArea->area_info.xpos + subArea->area_info.width - superArea->area_info.xpos) * 100 < superWx * 35 && subArea == superArea->subArea)
		{
			marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_LEFT at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
			subArea->pos_mark = RELA_LEFT;
			return;
		}

		if (subArea->area_info.width <= 300 && relaRightX * 100 < superWx * 33)
		{
			marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_RIGHT at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
			subArea->pos_mark = RELA_RIGHT;
			return;
		}

		if (ismostright(subArea, superArea) && ((relaRightX * 100 <= superWx * 50 && subArea->area_info.width <= 514) || (subArea->area_info.width <= 250 && relaRightX * 100 <= superWx * 55)))
		{
			marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_RIGHT at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
			subArea->pos_mark = RELA_RIGHT;
			return;
		}
	}
	marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_MAIN at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
	subArea->pos_mark = RELA_MAIN;
}

/**
 * @brief 标记该分块的扄1�7有子分块的相对位罄1�7.
 * @author xunwu
 * @date 2011/07/08
 **/
static void areaNode_visit_markPos(html_area_t *aNode)
{
	for (html_area_t *subArea = aNode->subArea; subArea; subArea = subArea->nextArea)
	{
		if (subArea->isValid)
		{
			markAreaRelaPos(subArea, aNode);
		}
		else
		{
			marktreeprintfs(MARK_POS, "the area(id=%d) pos_mark is RELA_UNDEFINED for it is not valid at %s(%d)-%s\r\n", subArea->no, __FILE__, __LINE__, __FUNCTION__);
			subArea->pos_mark = RELA_UNDEFINED;
		}
	}
	for (html_area_t *subArea = aNode->subArea; subArea; subArea = subArea->nextArea)
	{
		if (subArea->subArea_num > 0)
		{
			areaNode_visit_markPos(subArea);
		}
	}
}

/**
 * @brief 对分块树标记每个分块相对于父分块的位罄1�7.
 *	RELA_HEADER,	相对位置在头郄1�7.
 * RELA_LEFT,	相对位置在左辄1�7.
 *	RELA_RIGHT,	相对位置在右辄1�7.
 *	RELA_FOOTER,	相对位置在底郄1�7.
 *	RELA_MAIN,	相对位置在中郄1�7.
 *	RELA_UNDEFINED	不可视分坄1�7.
 * @author xunwu
 * @date 2011/07/08
 **/
void area_markPos(area_tree_t *atree)
{
	atree->root->pos_mark = RELA_MAIN;
	areaNode_visit_markPos(atree->root);
}

