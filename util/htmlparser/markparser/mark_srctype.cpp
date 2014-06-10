/*
 * easou_mark_srctype.cpp
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#include "easou_mark_com.h"
#include "easou_mark_innerinfo.h"
#include "easou_mark_srctype.h"

void tag_area_srctype(html_area_t * area , html_area_srctype_t type )
{
	unsigned int MASK = mark_type_to_mask[(int)type];
	area->srctype_mark._mark_bits |= MASK;
}

static bool mark_spec_srctype_on_area(area_tree_t *atree, html_area_srctype_t srctype)
{
	if(is_shutted_type(atree, srctype)){
//		Warn("srctype %s is shutted!", srctype_mark_handler[srctype].name);
		return true;
	}
	if(is_marked_srctype(atree, srctype)){
//		Warn("has marked type %s!", srctype_mark_handler[srctype].name);
		return true;
	}
	if(!IS_MARKED_POS(atree->mark_status) || !IS_COMPUTED_INFO(atree->mark_status)){
		Fatal((char*)"must mark pos and compute basis-info in advance!");
		assert(IS_MARKED_POS(atree->mark_status)&& IS_COMPUTED_INFO(atree->mark_status)) ;
	}
	set_marked_srctype(atree, srctype);
	SRCTYPE_MARK_T marker = get_marking_func(atree, srctype);
	if(marker){
		return marker(atree);
	}
	return true;
}

static bool mark_srctype_area_inner(area_tree_t *atree,mark_area_info_t * mark_info)
{
	for(html_area_srctype_t i_srctype = (html_area_srctype_t)(AREA_SRCTYPE_UNDEFINED + 1);
			i_srctype < AREA_SRCTYPE_LASTFLAG;
			i_srctype = (html_area_srctype_t)(i_srctype + 1)){
		if(is_selected_type(atree, i_srctype)){
			mark_spec_srctype_on_area(atree, i_srctype);
		}
	}

	return true;
}

/**
 * @brief 标注资源类型.
**/
bool mark_srctype_area(area_tree_t * atree )
{
	if(atree == NULL){
		Warn((char*)"atree is NULL" ) ;
//		return false ;
	}
	if(IS_MARKED_SRCTYPE(atree->mark_status)){
		return true ;
	}
	/*标注之前，必须有mark_info*/
	mark_area_info_t * mark_info = atree->mark_info ;
	assert(mark_info!=NULL) ;
	assert(IS_MARKED_POS(atree->mark_status)&& IS_COMPUTED_INFO(atree->mark_status)) ;
	bool ret = mark_srctype_area_inner(atree, mark_info);
	if(!ret){
		return false;
	}
	SET_MARKED_SRCTYPE(atree->mark_status) ;
	return true ;
}

/**
 * @brief 判断是否某种资源标注类型.
**/
bool inline is_srctype_mark(bits_field_t bits, html_area_srctype_t srctype)
{
	unsigned int MASK = mark_type_to_mask[(int)srctype];
	if(bits._mark_bits & MASK){
		return true;
	}
	return false;
}

/**
 * @brief 当前块是否某种资源类型.
**/
bool is_srctype_area(const html_area_t *area, html_area_srctype_t srctype)
{
	if(!is_marked_srctype(area->area_tree, srctype)){
		mark_spec_srctype_on_area(area->area_tree, srctype);
	}
	return is_srctype_mark(area->srctype_mark, srctype);
}

/**
 * @brief 当前块是否在某个资源类型块内. 当前块就是这种类型的情况也返回true;
**/
bool is_in_srctype_area(const html_area_t *area, html_area_srctype_t srctype)
{
	if(!is_marked_srctype(area->area_tree, srctype)){
		mark_spec_srctype_on_area(area->area_tree, srctype);
	}
	return is_srctype_mark(area->upper_srctype_mark, srctype);
}

/**
 * @brief 当前块是否包含某个资源类型块. 当前块就是这种类型的情况也返回true;
**/
bool is_contain_srctype_area(const html_area_t *area, html_area_srctype_t srctype)
{
	if(!is_marked_srctype(area->area_tree, srctype)){
		mark_spec_srctype_on_area(area->area_tree, srctype);
	}
	return is_srctype_mark(area->subtree_srctype_mark, srctype);
}


static bool build_mark_result_list(area_tree_t *area_tree, html_area_srctype_t srctype)
{
	if(is_build_srctype_list(area_tree, srctype)){
		return true;
	}

	mark_area_info_t *mark_info = area_tree->mark_info;

	int flag = AREA_VISIT_NORMAL;
	for(html_area_t *area = area_tree->root; area; area = html_area_iter_next(area, flag)){
		if(!is_contain_srctype_area(area, srctype)){
			flag = AREA_VISIT_SKIP;
		}
		else{
			if(is_srctype_area(area, srctype)){
				int ret = insert_area_on_list(area_tree->mark_info->srctype, srctype, area, &mark_info->area_pool);
				if(-1 == ret){
					goto FAIL;
				}
			}
			flag = AREA_VISIT_NORMAL;
		}
	}

	set_build_srctype_list(area_tree, srctype);

	return true;
FAIL:
	return false;
}

const area_list_t * get_srctype_mark_result(area_tree_t * atree , html_area_srctype_t srctype)
{
	assert(atree!=NULL) ;
	if(atree->mark_info == NULL
			|| srctype == AREA_SRCTYPE_UNDEFINED ){
		return NULL ;
	}

	if(!is_marked_srctype(atree, srctype)){
		mark_spec_srctype_on_area(atree, srctype);
	}

	if(!is_build_srctype_list(atree, srctype)){
		build_mark_result_list(atree, srctype);
	}

	if(atree->mark_info->srctype[srctype].num <=0 ){
		return NULL ;
	}

	return &(atree->mark_info->srctype[srctype]) ;
}

