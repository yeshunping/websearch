/**
 * mark_srctype_others.cpp
 * Description: 标记其它资源类型
 *  Created on: 2011-11-24
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "easou_mark_baseinfo.h"
#include "easou_mark_inner.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_conf.h"
#include "debuginfo.h"

/**
 * @brief 判断area是否为外部块
 */
static bool is_srctype_out_area(html_area_t * area, mark_area_info_t * g_info)
{
	assert(area !=NULL && g_info !=NULL);
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo;
	int total_area = paoi->extern_info.extern_area + paoi->inter_info.in_area + paoi->pic_info.pic_area + paoi->link_info.link_area + paoi->text_info.text_area;
	//策略一 交互标签个数
	if (paoi->inter_info.input_num != 0 || paoi->inter_info.select_num != 0)
	{
		marktreeprintfs(MARK_SRCTYPE, "the area(id=%d) is not for there is input or select tag in the area at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return false;
	}
	//策略二 面积
	if (100 * paoi->extern_info.extern_area >= 95 * total_area && total_area > 0)
	{
		if (area->valid_subArea_num == 0)
		{
			marktreeprintfs(MARK_SRCTYPE, "the area(id=%d) is at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
		else if (paoi->extern_info.extern_area == total_area)
		{
			marktreeprintfs(MARK_SRCTYPE, "the area(id=%d) is at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
	}
	marktreeprintfs(MARK_SRCTYPE, "the area(id=%d) is not by default at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
	return false;
}

/*文本信息块，该块以提供文字信息为主*/
static bool is_srctype_textinfo_area(html_area_t * area, mark_area_info_t * g_info)
{
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo;
	int total_area = paoi->extern_info.extern_area + paoi->inter_info.in_area + paoi->pic_info.pic_area + paoi->link_info.link_area + paoi->text_info.text_area;
	int real_cont_size = paoi->text_info.con_size - paoi->text_info.no_use_con_num;
	real_cont_size -= paoi->link_info.anchor_size;
	int real_cont_area = paoi->text_info.text_area - paoi->link_info.link_area;
	if (real_cont_size < 0)
		real_cont_size = 0;

	//1.  不包括 交互块，外部块
	if (paoi->extern_info.extern_area > 0 || paoi->inter_info.in_area > 0)
	{
		marktreeprintfs(MARK_SRCTYPE, " judge it not text area, because extern area or interaction area is >0  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	float pic_txt_rate = 0.0;
	if (real_cont_size)
		pic_txt_rate = (float) (paoi->pic_info.link_pic_area) / real_cont_area;
	//图片块 算文本块比例
	if (paoi->pic_info.link_pic_area > 0 || pic_txt_rate > 0.10)
	{
		marktreeprintfs(MARK_SRCTYPE, " judge it not text area, because extern area or interaction area is >0  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}
	//链接占比
	float link_txt_rate = 0.0;
	if (real_cont_area)
		link_txt_rate = (float) paoi->link_info.link_area / real_cont_area;
	//文本太少,链接太多
	if (real_cont_size < 4 || real_cont_area <= 0 || paoi->link_info.num >= 10&&link_txt_rate>0.2)
	{
		marktreeprintfs(MARK_SRCTYPE, " judge it not text area, because real content is too little at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}


	if ((paoi->link_info.num <= 1 && link_txt_rate > 0.2) || (paoi->link_info.num <= 3 && link_txt_rate > 0.1) || (paoi->link_info.num < 10 && link_txt_rate > 0.05))
	{
		marktreeprintfs(MARK_SRCTYPE, " judge it not text area, because link num >0  and content size is < 60 at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	float score1 = ((float) real_cont_size / paoi->text_info.con_size);
	float score2 = ((float) real_cont_area / total_area);
	if (score1 > 0.6 && score2 > 0.1 && paoi->text_info.con_size <= 60)
		return true;
	else if (score1 > 0.5 && score2 > 0.1 && paoi->text_info.con_size > 60)
		return true;
	marktreeprintfs(MARK_SRCTYPE, " judge it not text area, because it is  at last at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	return false;
}

static int start_visit_for_mark_srctype(html_area_t * area, void * data)
{
	assert(area!=NULL);
	if (MARK_SRCTYPE == g_EASOU_DEBUG)
	{
		printline();
		printNode(area->begin->hpNode);
	}
	if (area->isValid == false)
	{
		marktreeprintfs(MARK_SRCTYPE, "skip the area(id=%d) for it is not valid at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	mark_area_info_t * mark_info = (mark_area_info_t *) data;
	if (area->valid_subArea_num>0&&is_too_big_area(area, mark_info, LEVEL_SRCTYPE))
	{
		marktreeprintfs(MARK_SRCTYPE, "the area(id=%d) is not for it is too big at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	if (is_too_up_area(area, mark_info, LEVEL_SRCTYPE))
	{
		marktreeprintfs(MARK_SRCTYPE, "the area(id=%d) is not for it is too up at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	if (is_srctype_out_area(area, (mark_area_info_t *) data))
	{
		tag_area_srctype(area, AREA_SRCTYPE_OUT);
	}
	if (is_srctype_textinfo_area(area, (mark_area_info_t *) data))
	{
		tag_area_srctype(area, AREA_SRCTYPE_TEXT);
	}
	if (area->srctype_mark._mark_bits != 0)
	{
		/** This make srctypes has a little dependence on each other. */
		marktreeprintfs(MARK_SRCTYPE, " mark src type complete  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	if (is_too_little_area(area, mark_info, LEVEL_SRCTYPE))
	{
		marktreeprintfs(MARK_SRCTYPE, " the area visit skip for the area it too litte at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	if (is_too_down_area(area, mark_info, LEVEL_SRCTYPE))
	{
		marktreeprintfs(MARK_SRCTYPE, " the area visit skip for the area it too down at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	return AREA_VISIT_NORMAL;
}

static void tag_clean_sub_area(html_area_t * area, unsigned int sub_type_and_mask)
{
	for (html_area_t *sub_area = area->subArea; sub_area != NULL; sub_area = sub_area->nextArea)
	{
		if (sub_area->isValid == false)
			continue;

		sub_area->srctype_mark._mark_bits &= ~sub_type_and_mask;
	}
}

static void tag_recall_mark(html_area_t * area, unsigned int sub_type_and_mask)
{
	if (sub_type_and_mask != 0 && area->valid_subArea_num > 1)
	{
		area->srctype_mark._mark_bits |= sub_type_and_mask;
		tag_clean_sub_area(area, sub_type_and_mask);
		marktreeprintfs(MARK_SRCTYPE, " the parent area add  src type(%x) of the sub area,and the sub area delete the type at %s(%d)-%s\r\n", sub_type_and_mask, __FILE__, __LINE__, __FUNCTION__);
		return;
	}
//	Debug("recall %u_%u %s\n", area->depth,area->order ,"NONE") ;
}

static int finish_visit_for_mark_srctype(html_area_t * area, void * data)
{
	assert(area!=NULL && data!=NULL);
	//此处可以增加召回策略
	if (area->isValid == false)
	{
		return AREA_VISIT_NORMAL;
	}

	static const unsigned int INVERT_LINK_MASK = ~(mark_type_to_mask[AREA_SRCTYPE_LINK]);
	static const unsigned int INVERT_HUB_MASK = ~(mark_type_to_mask[AREA_SRCTYPE_HUB]);

	/** 索引块和链接块不以这种方式合并 */
	if ((area->srctype_mark._mark_bits & INVERT_LINK_MASK & INVERT_HUB_MASK) != 0)
	{
		return AREA_VISIT_NORMAL;
	}

	unsigned int sub_type_and_mask = 0xFFFFFFFF & INVERT_LINK_MASK & INVERT_HUB_MASK;

	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (subarea->isValid)
		{
			sub_type_and_mask &= subarea->srctype_mark._mark_bits; //get commen type
		}
	}
	marktreeprintfs(MARK_SRCTYPE, " the area is link or hub ,add  src type(%x) of the sub area at %s(%d)-%s\r\n", sub_type_and_mask, __FILE__, __LINE__, __FUNCTION__);
	tag_recall_mark(area, sub_type_and_mask);
	return AREA_VISIT_NORMAL;
}

static int start_mark_updown(html_area_t *area, void *data)
{
	if (!area->isValid)
		return AREA_VISIT_SKIP;

	area->upper_srctype_mark = area->srctype_mark;

	html_area_t *upper = area->parentArea;

	if (upper)
	{
		area->upper_srctype_mark._mark_bits |= upper->upper_srctype_mark._mark_bits;
	}

	return AREA_VISIT_NORMAL;
}

static int finish_mark_updown(html_area_t *area, void *data)
{
	area->subtree_srctype_mark._mark_bits |= area->srctype_mark._mark_bits;
	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (subarea->isValid)
		{
			area->subtree_srctype_mark._mark_bits |= subarea->subtree_srctype_mark._mark_bits;
		}
	}
	return AREA_VISIT_NORMAL;
}

bool mark_other_srctypes(area_tree_t * area_tree)
{
	bool ret = areatree_visit(area_tree, start_visit_for_mark_srctype, finish_visit_for_mark_srctype, area_tree->mark_info);
	areatree_visit(area_tree, start_mark_updown, finish_mark_updown, NULL);
	return ret;
}

