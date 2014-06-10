/*
 * easou_mark_sem.cpp
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#include "easou_mark_com.h"
#include "easou_mark_innerinfo.h"
#include "easou_mark_sem.h"


/**
 * @brief 标注各种语义分块类型.实际的分发操作在此处进行
**/
static int start_mark_sem_updown(html_area_t *area, void *data)
{
	if(!area->isValid){
		return AREA_VISIT_SKIP;
	}
	area->upper_sem_mark = area->sem_mark;
	html_area_t *upper = area->parentArea;

	if(upper){
		area->upper_sem_mark._mark_bits |= upper->upper_sem_mark._mark_bits;
	}
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 对当前块标记语义类型;同时标注祖先块具有type类型后裔后裔

**/
void tag_area_sem(html_area_t *area, html_area_sem_t type)
{
	unsigned int MASK = mark_type_to_mask[(int)type];
	area->sem_mark._mark_bits |= MASK;
	for(html_area_t *upper = area; upper; upper = upper->parentArea){
		upper->subtree_sem_mark._mark_bits |= MASK;
	}
	areatree_visit(area, start_mark_sem_updown, NULL, NULL);
}

/**
 * @brief 标注各种语义分块类型. 该接口只在非lazy模式, 或已选择了某些类型时才调用.
**/
void clear_sem_area_tag(html_area_t *area, html_area_sem_t type)
{
	unsigned int MASK = mark_type_to_mask[(int)type];

	area->sem_mark._mark_bits &= ~MASK;
}

/**
 * @brief 对特定的语义分块类型进行标注.
**/
static bool mark_spec_sem_on_area(area_tree_t *atree, html_area_sem_t sem)
{
	if(is_shutted_type(atree, sem)){
//		Warn("sem-type %s is shutted!", sem_mark_handler[sem].name);
		return true;
	}
	if(is_marked_sem(atree, sem)){
//		Warn("has marked type %s!", sem_mark_handler[sem].name);
		return true;
	}
	if(!IS_MARKED_POS(atree->mark_status) || !IS_COMPUTED_INFO(atree->mark_status)){
		Fatal((char*)"must mark pos and compute basis-info in advance!");
		assert(IS_MARKED_POS(atree->mark_status)&& IS_COMPUTED_INFO(atree->mark_status)) ;
	}

	/**
	 * Set 'marked' in advance, to avoid cycling call.
	 */
	set_marked_sem(atree, sem);
	SEM_MARK_T marker = get_marking_func(atree, sem);
	if(marker){
		return marker(atree);
	}
	return true;
}

static bool mark_sem_area_inner(area_tree_t *atree)
{
	for(html_area_sem_t i_sem = (html_area_sem_t)(AREA_SEM_UNDEFINED + 1);
			i_sem < AREA_SEM_LASTFLAG;
			i_sem = (html_area_sem_t)(i_sem + 1)){
		if(is_selected_type(atree, i_sem)){
			mark_spec_sem_on_area(atree, i_sem);
		}
	}
	return true;
}

/**
 * @brief 标注各种语义分块类型.
 *  该接口只在非lazy模式, 或已选择了某些类型时才调用.
**/
bool mark_sem_area(area_tree_t *atree)
{
	assert(atree!=NULL && atree->mark_info != NULL ) ;
	if(IS_MARKED_SEM(atree->mark_status)){
		return true ;
	}
	assert(IS_MARKED_SRCTYPE(atree->mark_status) && IS_MARKED_POS(atree->mark_status)) ;
	int ret = mark_sem_area_inner(atree);
	if(ret == false){
		return false;
	}
	SET_MARKED_SEM(atree->mark_status) ;
	return true ;
}

/**
 * @brief 判断是否某种语义标注类型.
**/
inline bool is_sem_mark(bits_field_t bits, html_area_sem_t sem)
{
	unsigned int MASK = mark_type_to_mask[(int)sem];
	if(bits._mark_bits & MASK){
		return true;
	}
	return false;
}

/**
 * @brief 当前块是否某种语义类型.
**/
bool is_sem_area(const html_area_t *area, html_area_sem_t sem)
{
	if(!is_marked_sem(area->area_tree, sem)){
		mark_spec_sem_on_area(area->area_tree, sem);
	}
	return is_sem_mark(area->sem_mark, sem);
}

/**
 * @brief 当前块是否某种语义类型.
**/
bool is_in_sem_area(const html_area_t *area, html_area_sem_t sem)
{
	if(!is_marked_sem(area->area_tree, sem)){
		mark_spec_sem_on_area(area->area_tree, sem);
	}
	return is_sem_mark(area->upper_sem_mark, sem);
}

/**
 * @brief 当前块是否包含某个语义类型块.
 *	当前块就是这种类型的情况也返回true;
**/
bool is_contain_sem_area(const html_area_t *area, html_area_sem_t sem)
{
	if(!is_marked_sem(area->area_tree, sem)){
		mark_spec_sem_on_area(area->area_tree, sem);
	}
	return is_sem_mark(area->subtree_sem_mark, sem);
}

static bool build_mark_result_list(area_tree_t *area_tree, html_area_sem_t sem)
{
	if(is_build_sem_list(area_tree, sem)){
		return true;
	}

	mark_area_info_t *mark_info = area_tree->mark_info;

	int flag = AREA_VISIT_NORMAL;
	for(html_area_t *area = area_tree->root; area; area = html_area_iter_next(area, flag)){
		if(!is_contain_sem_area(area, sem)){
			flag = AREA_VISIT_SKIP;
		}
		else{
			if(is_sem_area(area, sem)){
				int ret =
					insert_area_on_list(area_tree->mark_info->sem, sem, area, &mark_info->area_pool);
				if(-1 == ret){
					goto FAIL;
				}
			}
			flag = AREA_VISIT_NORMAL;
		}
	}

	set_build_sem_list(area_tree, sem);

	return true;
FAIL:
	return false;
}

/**
 * @brief 获取某种语义类型的标注结果，如果没有结果返回NULL
**/
const area_list_t * get_sem_mark_result(area_tree_t * atree , html_area_sem_t sem)
{
	assert(atree!=NULL) ;
	if(atree->mark_info == NULL || sem == AREA_SEM_UNDEFINED ){
		return NULL ;
	}

	if(!is_marked_sem(atree, sem)){
		mark_spec_sem_on_area(atree, sem);
	}

	if(!is_build_sem_list(atree, sem)){
		build_mark_result_list(atree, sem);
	}

	if(atree->mark_info->sem[sem].num <=0 ){
		return NULL ;
	}

	return &(atree->mark_info->sem[sem]) ;
}
