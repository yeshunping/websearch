/**
 * @file mark_srctype_interaction.cpp
 * @description:交互块识别
 * @author chen_hu@staff.easou.com
 * @date 2012-08-22
*/
#include "easou_mark_baseinfo.h"
#include "easou_mark_inner.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_conf.h"
#include "debuginfo.h"

static bool is_srctype_interaction_area(html_area_t * area , mark_area_info_t * g_info)
{
	if(area->baseinfo->text_info.cn_num > 50)
	{
		marktreeprintfs(MARK_INTERACTION, "the area(id=%d) is not for it has too many words at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return false;
	}
	area_baseinfo_t * a_info = area->baseinfo ;//分块基本信息
	int total_area = a_info->extern_info.extern_area + a_info->inter_info.in_area
			+ a_info->pic_info.pic_area + a_info->link_info.link_area + a_info->text_info.text_area ;
//	int valid_textSize = a_info->text_info.con_size - a_info->text_info.no_use_con_size;
//	if(valid_textSize > 200)
//	{
//		marktreeprintfs(MARK_INTERACTION, "area(id=%d) is not interaction area for it has too many words at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
//		return false;
//	}
	if( a_info->inter_info.in_area *100 >= total_area*95 && total_area >0 )
	{//15*15
		marktreeprintfs(MARK_INTERACTION, "area(id=%d) is interaction area at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true ;
	}
	if(a_info->link_info.num >=5 )
	{
		marktreeprintfs(MARK_INTERACTION, "judge it not interaction area(id=%d), because link num (%d)>5 at %s(%d)-%s\r\n", area->no, a_info->link_info.num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	if(a_info->inter_info.is_have_form && a_info->inter_info.spec_word_num == true)
	{
		marktreeprintfs(MARK_INTERACTION, "judge it is interaction area(id=%d), because have form  at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	int text_node_num = a_info->text_info.con_num ;//当前块纯文本节点个数
	int input_select_num = a_info->inter_info.input_num + a_info->inter_info.select_num;//input标签个数+select标签个数
	int input_num = a_info->inter_info.input_num;
	if(input_select_num >= 2 && input_select_num >= text_node_num)
	{
		marktreeprintfs(MARK_INTERACTION, "judge it is interaction area(id=%d),because input_select_num >=2 and input_select_num >= text_node_num,at at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true ;
	}
	if( a_info->inter_info.input_radio_num >= 3
		&& a_info->inter_info.input_radio_txtlen >= 10)
	{
		marktreeprintfs(MARK_INTERACTION, "judge it is interaction area(id=%d) by input tag,at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true ;
	}
	int textareanum = a_info->inter_info.textarea_num;
	if((a_info->inter_info.select_num >= 1 || a_info->inter_info.textarea_num >= 1 || a_info->inter_info.input_num >= 1) && a_info->inter_info.spec_word_num == true)
	{
		marktreeprintfs(MARK_INTERACTION, "judge it is interaction area(id=%d) by select tag and textarea,at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true ;
	}
	if(a_info->inter_info.select_num >= 1 || a_info->inter_info.textarea_num >= 1)
	{
		marktreeprintfs(MARK_INTERACTION, "judge it is interaction area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	if(a_info->inter_info.input_num >=1 && a_info->inter_info.in_area > 0)
	{
		marktreeprintfs(MARK_INTERACTION, "judge it is interaction area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	//召回一些js生成的交互块
	int inter_num = a_info->inter_info.cursor_num + a_info->inter_info.input_num + a_info->inter_info.is_have_form + a_info->inter_info.script_num;
	if(a_info->text_info.con_num == 1 && a_info->inter_info.spec_word_num)
	{
		int limit_word_num = 20;
		if(inter_num == 0)
		{
			limit_word_num = 10;
		}
		if(a_info->text_info.con_size < limit_word_num)
		{
			marktreeprintfs(MARK_INTERACTION, "the area(id=%d) is for it maybe generate by js at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}
	int wx = area->area_info.width;
	int hx = area->area_info.height;
	//召回一些分享块
	if(a_info->inter_info.spec_word_num && a_info->inter_info.script_num > 0 && a_info->link_info.other_num > 0 && a_info->text_info.con_num <= a_info->link_info.other_num + 1)
	{
		marktreeprintfs(MARK_INTERACTION, "the area(id=%d) is for it maybe share area at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	//识别一些评论块
	if (area->begin == area->end && hx <= 200 && hx <= 800)
	{
		vstruct_info_t *struct_info = area->begin->struct_info;
		int leaf_num = struct_info->valid_leaf_num;
		int fixed_pic_num = area->baseinfo->pic_info.size_fixed_num;
		int interaction_tag_num = area->begin->struct_info->interaction_tag_num;
		int spec_word_num = area->baseinfo->inter_info.spec_word_num;
		if(interaction_tag_num > 0 && spec_word_num > 0 && leaf_num - fixed_pic_num - interaction_tag_num - spec_word_num <= 2)
		{
			marktreeprintfs(MARK_INTERACTION, "the area(id=%d) is at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}
	marktreeprintfs(MARK_INTERACTION, "judge it is not interaction area(id=%d), because not fit at last at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
	return false ;
}

static int start_visit_for_mark_srctype(html_area_t * area , void * data )
{
	assert(area!=NULL);
	if(area->isValid == false )
	{
		marktreeprintfs(MARK_INTERACTION,"the area(id=%d) visit skip for the area is not valid t at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
	mark_area_info_t * mark_info = (mark_area_info_t *)data ;
	if(is_too_big_area(area , mark_info , LEVEL_SRCTYPE )) {
		marktreeprintfs(MARK_INTERACTION,"mark src type over for the area(id=%d) it too big at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL ;
	}
//	if( is_too_up_area(area , mark_info , LEVEL_SRCTYPE )) {
//		marktreeprintfs(MARK_INTERACTION,"mark src type over for the area(id=%d) it too up at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
//		return AREA_VISIT_NORMAL ;
//	}

	if(is_shutted_type(mark_info->area_tree, AREA_SRCTYPE_INTERACTION)){
	}
	else{
		if( is_srctype_interaction_area (area , (mark_area_info_t * )data ) ) {
			tag_area_srctype(area,AREA_SRCTYPE_INTERACTION );
			return VISIT_SKIP_CHILD;
		}
	}
/*
	if(area->srctype_mark._mark_bits != 0){
	//	printf(MARK_SRCTYPE," mark src type complete  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}
*/
/*
	if(is_too_little_area(area , mark_info , LEVEL_SRCTYPE )){
//		Debug("too_little_%d_%d" ,area->depth , area->order  ) ;
	//	printf(MARK_SRCTYPE," the area visit skip for the area it too litte at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
*/
	if( is_too_down_area(area , mark_info , LEVEL_SRCTYPE )){
		marktreeprintfs(MARK_INTERACTION," the area(id=%d) visit skip for the area it too down at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
	return AREA_VISIT_NORMAL ;
}

static int finish_visit_for_mark_srctype(html_area_t * area , void * data )
{
	assert(area!=NULL && data!=NULL ) ;

	if(area->isValid == false ){
		return AREA_VISIT_NORMAL ;
	}
	return AREA_VISIT_NORMAL ;
}

static int start_mark_updown(html_area_t *area, void *data)
{
	if(!area->isValid)
		return AREA_VISIT_SKIP;

	area->upper_srctype_mark = area->srctype_mark;//当前块的资源类型标注
	if(!area->baseinfo->inter_info.is_have_form)
	{
		html_area_t *upper = area->parentArea;//得到当前块的父分块
		if(upper){
			marktreeprintfs(MARK_INTERACTION,"the area(id=%d) the same with upper at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			area->upper_srctype_mark._mark_bits |= upper->upper_srctype_mark._mark_bits;
		}
	}

	return AREA_VISIT_NORMAL;
}

static int finish_mark_updown(html_area_t *area, void *data)
{
	area->subtree_srctype_mark._mark_bits |= area->srctype_mark._mark_bits;
	if(!area->baseinfo->inter_info.is_have_form)
        {
		for(html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
		{
			if(subarea->isValid)
			{
				marktreeprintfs(MARK_INTERACTION,"the area(id=%d) the same with sub area at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
				area->subtree_srctype_mark._mark_bits |= subarea->subtree_srctype_mark._mark_bits;
			}
		}
	}	
	return AREA_VISIT_NORMAL;
}

bool mark_srctype_interaction(area_tree_t * area_tree)
{
	bool ret = areatree_visit(area_tree ,
			start_visit_for_mark_srctype ,
			NULL ,
			area_tree->mark_info);

	areatree_visit(area_tree, start_mark_updown, finish_mark_updown, NULL);
	return ret;
}
