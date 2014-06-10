/*
 * easou_ahtml_markinfo.cpp
 *
 *  Created on: 2011-11-18
 *      Author: ddt
 */
#include "easou_mark_markinfo.h"

#define DEFAULT_MARK_AREA_NUM  32

/**
 * @brief 创建mark info
**/
mark_area_info_t *mark_info_create()
{
	mark_area_info_t *mark_info = (mark_area_info_t *)calloc(1,sizeof(mark_area_info_t));
	if(mark_info == NULL ){
		return NULL;
	}
	int ret = nodepool_init(&mark_info->area_pool , sizeof(area_list_node_t) , DEFAULT_MARK_AREA_NUM );
	if(ret == 0){
		mark_info_destroy(mark_info);
		return NULL;
	}
	return mark_info;
}

/**
 * @brief 清空mark info
**/
void mark_info_clean(mark_area_info_t * mai)
{
	if(mai!=NULL){
		nodepool_t tmp = mai->area_pool;
		memset(mai,0,sizeof(mark_area_info_t));
		mai->area_pool = tmp;
		nodepool_reset(&mai->area_pool) ;
	}
}

/**
 * @brief 销毁mark info
**/
void mark_info_destroy(mark_area_info_t * mai)
{
	if(mai != NULL ) {
		nodepool_destroy(&mai->area_pool) ;
		free(mai) ;
	}
}

/**
 * @brief 清空mark info
**/
void mark_info_reset(mark_area_info_t * mai)
{
	if(mai!=NULL){
		nodepool_t tmp = mai->area_pool;
		memset(mai,0,sizeof(mark_area_info_t));
		mai->area_pool = tmp;
		nodepool_reset(&mai->area_pool) ;
	}
}


