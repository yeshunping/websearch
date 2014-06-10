/*
 * @file mark_srctype_picture.cpp
 * @description:图片块识别
 * @author chen_hu@staff.easou.com
 * @date 2012-08-28
*/
#include "easou_mark_baseinfo.h"
#include "easou_mark_inner.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_conf.h"
#include "debuginfo.h"

static bool is_srctype_picture_area(html_area_t * area , mark_area_info_t * g_info)
{
	area_baseinfo_t * picInfo = area->baseinfo ;
	int total_area = picInfo->extern_info.extern_area + picInfo->inter_info.in_area
			+ picInfo->pic_info.pic_area + picInfo->link_info.link_area + picInfo->text_info.text_area ;
	if(total_area == 0 )
	{
		marktreeprintfs(MARK_PIC," judge it not pic area(id=%d), because total_area == 0 at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//策略一，不能含有外部块，否则认为粒度不合理
	if(picInfo->extern_info.extern_area!=0) 
	{
		marktreeprintfs(MARK_PIC," judge it not pic area(id=%d), because extern_area != 0  at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false ;
	}

	//策略二，不能含有交互标签，否则认为粒度不合理
	if(picInfo->inter_info.input_num!=0 || picInfo->inter_info.select_num !=0 ) 
	{
		marktreeprintfs(MARK_PIC," judge it not pic area(id=%d), because there are input or select tag  at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//策略三，图片数量不能为0
	if(picInfo->pic_info.pic_num == 0 ) 
	{
		marktreeprintfs(MARK_PIC," judge it not pic area(id=%d), because pic number == 0  at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//策略四，图片链接认为不是图片块
/*	if(picInfo->pic_info.pic_num == picInfo->pic_info.link_pic_num
	|| picInfo->pic_info.link_pic_area *10 >= 6 * picInfo->pic_info.pic_area ) 
	{
		marktreeprintfs(MARK_PIC," judge it not pic area, because it is picture link  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false  ;
	}
*/
	if(picInfo->pic_info.pic_num >= 1 && picInfo->text_info.text_area == 0)
	{
		return true;
	}
	//策略五 图片平均面积太小，认为不是图片块，召回意义不大
	if(picInfo->pic_info.pic_area/picInfo->pic_info.pic_num <= 1000 ) {
		marktreeprintfs(MARK_PIC," judge it not pic area(id=%d), because single pic is too small at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false ;
	}

	//策略6 刨除上面的策略，如果图片面积占全部面积的0.7以上，可以认为是图片块
	if(picInfo->pic_info.pic_area*10 >= total_area * 7 ) {
		marktreeprintfs(MARK_PIC," judge it is pic area(id=%d), because picnum > 0 at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true ;
	}
	if(picInfo->pic_info.pic_num >= 1)
	{
		marktreeprintfs(MARK_PIC," judge it is pic area(id=%d), because picnum > 0 at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	marktreeprintfs(MARK_PIC," judge it not pic area(id=%d), because it is  at last at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
	return false ;
}

static int start_visit_for_mark_srctype(html_area_t * area , void * data )
{
	assert(area!=NULL) ;
	
	if(area->isValid == false )
	{
		marktreeprintfs(MARK_PIC,"the area visit skip for the area(id=%d) is not valid t at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
	int valid_textSize = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	mark_area_info_t * mark_info = (mark_area_info_t *)data ;
	if(valid_textSize > 60)
	{
		marktreeprintfs(MARK_PIC,"the area(id=%d) is not for it has too many words at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL ;
	}
//	if(is_too_big_area(area , mark_info , LEVEL_SRCTYPE )) {
//		marktreeprintfs(MARK_PIC," mark src type over for the area(id=%d) it too big at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
//		return AREA_VISIT_NORMAL ;
//	}
	if( is_too_up_area(area , mark_info , LEVEL_SRCTYPE )) {
		marktreeprintfs(MARK_PIC," mark src type over for the area(id=%d) it too up at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL ;
	}

	if( is_too_up_area(area , mark_info , LEVEL_SRCTYPE )) {
		marktreeprintfs(MARK_PIC," mark src type over for the area(id=%d) it too up at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL ;
	}

	if(is_shutted_type(mark_info->area_tree, AREA_SRCTYPE_PIC)){
	}
	else{
		if(is_srctype_picture_area(area ,(mark_area_info_t * )data )) {
			tag_area_srctype(area,AREA_SRCTYPE_PIC);
			marktreeprintfs(MARK_PIC,"the area(id=%d) it is at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		}
	}
/*
	if(area->srctype_mark._mark_bits != 0){
		//This make srctypes has a little dependence on each other. 
		marktreeprintfs(MARK_PIC," mark src type complete  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}
*/
/*	if(is_too_little_area(area , mark_info , LEVEL_SRCTYPE )){
//		Debug("too_little_%d_%d" ,area->depth , area->order  ) ;
		marktreeprintfs(MARK_PIC," the area visit skip for the area it too litte at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
*/
	if( is_too_down_area(area , mark_info , LEVEL_SRCTYPE )){
		marktreeprintfs(MARK_PIC," the area visit skip for the area(id=%d) it too down at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
	return AREA_VISIT_NORMAL ;
}

static int start_mark_updown(html_area_t *area, void *data)
{
	if(!area->isValid)
		return AREA_VISIT_SKIP;

	area->upper_srctype_mark = area->srctype_mark;

	html_area_t *upper = area->parentArea;

	if(upper){
		area->upper_srctype_mark._mark_bits |= upper->upper_srctype_mark._mark_bits;
	}

	return AREA_VISIT_NORMAL;
}

static int finish_mark_updown(html_area_t *area, void *data)
{
	area->subtree_srctype_mark._mark_bits |= area->srctype_mark._mark_bits;
	for(html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea){
		if(subarea->isValid){
			area->subtree_srctype_mark._mark_bits |= subarea->subtree_srctype_mark._mark_bits;
			marktreeprintfs(MARK_PIC,"the area(id=%d) the same whith sub area(id=%d) at %s(%d)-%s\r\n", area->no, subarea->no, __FILE__,__LINE__,__FUNCTION__);
		}
	}
	return AREA_VISIT_NORMAL;
}

bool mark_srctype_picture(area_tree_t * area_tree)
{
	bool ret = areatree_visit(area_tree ,
			start_visit_for_mark_srctype ,
			NULL ,
			area_tree->mark_info);

	areatree_visit(area_tree, NULL, finish_mark_updown, NULL);
	return ret;
}
