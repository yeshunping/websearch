/*
 * easou_mark_func.cpp
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#include "easou_mark_com.h"
#include "easou_mark_innerinfo.h"
#include "easou_mark_func.h"


static int start_mark_func_updown(html_area_t *area, void *data)
{
	if(!area->isValid){
		return AREA_VISIT_SKIP;
	}
	area->upper_func_mark = area->func_mark;
	html_area_t *upper = area->parentArea;
	if(upper){
		if(upper->upper_func_mark._mark_bits == area->upper_func_mark._mark_bits){
			return AREA_VISIT_SKIP;
		}
		area->upper_func_mark._mark_bits |= upper->upper_func_mark._mark_bits;
	}
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 对当前块标记功能类型.
**/
void tag_area_func(html_area_t * area, html_area_func_t type)
{
	unsigned int MASK = mark_type_to_mask[(int)type];
	area->func_mark._mark_bits |= MASK;
	area->subtree_func_mark._mark_bits |= MASK;
	for(html_area_t *upper = area->parentArea; upper; upper = upper->parentArea){
		upper->subtree_func_mark._mark_bits |= MASK;
	}
	areatree_visit(area, start_mark_func_updown, NULL, NULL);
}

void clear_func_area_tag(html_area_t *area, html_area_func_t type)
{
	unsigned int MASK = mark_type_to_mask[(int)type];
	area->func_mark._mark_bits &= ~MASK;
	area->upper_func_mark._mark_bits &= ~MASK;
	area->subtree_func_mark._mark_bits &= ~MASK;
}

/**
 * @brief 对特定的块进行功能标注
**/
bool mark_spec_func_on_area(area_tree_t *atree, html_area_func_t func)
{
	if(is_shutted_type(atree, func)){
//		Warn("func-type %s is shutted!", func_mark_handler[func].name);
		return true;
	}
	if(is_marked_func(atree, func)){
//		Warn("has marked type %s!", func_mark_handler[func].name);
		return true;
	}

	if(!IS_MARKED_POS(atree->mark_status) || !IS_COMPUTED_INFO(atree->mark_status)){
		Fatal((char*)"must mark pos and compute basis-info in advance!");
		assert(IS_MARKED_POS(atree->mark_status)&& IS_COMPUTED_INFO(atree->mark_status)) ;
	}
	set_marked_func(atree, func);
	FUNC_MARK_T marker = get_marking_func(atree, func);
	if(marker){
		return marker(atree);
	}
	return true;
}

static bool mark_func_area_inner(area_tree_t *atree, mark_area_info_t *mark_info)
{
	for(html_area_func_t i_func = (html_area_func_t)(AREA_FUNC_UNDEFINED + 1);
			i_func < AREA_FUNC_LASTFLAG;
			i_func = (html_area_func_t)(i_func + 1)){
		if(is_selected_type(atree, i_func)){
			mark_spec_func_on_area(atree, i_func);
		}
	}

	return true;
}

/**
 * @brief 标记功能类型.
**/
bool mark_func_area(area_tree_t * atree )
{
	assert(atree != NULL );
	if(IS_MARKED_FUNC(atree->mark_status)){
		return true ;
	}
	mark_area_info_t * mark_info = atree->mark_info ;
	assert(mark_info !=NULL ) ;
	assert(IS_MARKED_SRCTYPE(atree->mark_status) && IS_MARKED_POS(atree->mark_status)) ;
	int ret = mark_func_area_inner(atree, mark_info);
	if(ret == false){
		return false;
	}
	SET_MARKED_FUNC(atree->mark_status) ;
	return true ;
}

/**
 * @brief 判断是否某种功能标注类型.
**/
bool inline is_func_mark(bits_field_t bits, html_area_func_t func)
{
	unsigned int MASK = mark_type_to_mask[(int)func];
	if(bits._mark_bits & MASK){
		return true;
	}
	return false;
}

/**
 * @brief 当前块是否某种功能类型.
**/
bool is_func_area(const html_area_t *area, html_area_func_t func)
{
	if(!is_marked_func(area->area_tree, func)){
		mark_spec_func_on_area(area->area_tree, func);
	}
	return is_func_mark(area->func_mark, func);
}

/**
 * @brief 当前块是否在某个功能类型块内.
 *	当前块就是这种类型的情况也返回true;
**/
bool is_in_func_area(const html_area_t *area, html_area_func_t func)
{
	if(!is_marked_func(area->area_tree, func)){
		mark_spec_func_on_area(area->area_tree, func);
	}
	return is_func_mark(area->upper_func_mark,func);
}

/**
 * @brief 当前块是否包含某个功能类型块. 当前块就是这种类型的情况就返回true;
**/
bool is_contain_func_area(const html_area_t *area, html_area_func_t func)
{
	if(!is_marked_func(area->area_tree, func)){
		mark_spec_func_on_area(area->area_tree, func);
	}
	return is_func_mark(area->subtree_func_mark,func);
}


static bool build_mark_result_list(area_tree_t *area_tree, html_area_func_t func)
{
	if(is_build_func_list(area_tree, func)){
		return true;
	}
	mark_area_info_t *mark_info = area_tree->mark_info;
	int flag = AREA_VISIT_NORMAL;
	for(html_area_t *area = area_tree->root; area; area = html_area_iter_next(area, flag)){
		if(!is_contain_func_area(area, func)){
			flag = AREA_VISIT_SKIP;
		}
		else{
			if(is_func_area(area, func)){
				int ret = insert_area_on_list(area_tree->mark_info->func, func, area, &mark_info->area_pool);
				if(-1 == ret){
					goto FAIL;
				}
			}
			flag = AREA_VISIT_NORMAL;
		}
	}
	set_build_func_list(area_tree, func);

	return true;
FAIL:
	return false;
}

/**
 * @brief 获取某种功能类型的标注结果，如果没有结果返回NULL
**/
const area_list_t * get_func_mark_result(area_tree_t * atree , html_area_func_t func)
{
	assert(atree!=NULL) ;
	if(atree->mark_info == NULL || func == AREA_FUNC_UNDEFINED ){
		return NULL ;
	}
	if(!is_marked_func(atree, func)){
		mark_spec_func_on_area(atree, func);
	}
	if(!is_build_func_list(atree, func)){
		build_mark_result_list(atree, func);
	}
	if(atree->mark_info->func[func].num <=0 ){
		return NULL ;
	}
	return &(atree->mark_info->func[func]) ;
}
