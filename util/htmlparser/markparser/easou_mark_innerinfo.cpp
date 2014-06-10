/*
 * easou_ahtml_innerinfo.cpp
 *
 *  Created on: 2011-11-23
 *      Author: ddt
 */
#include "easou_mark_innerinfo.h"

#define ALLOC_FAIL_THEN_GOTO(obj,flag) do {\
	if(NULL==(obj)) {\
		goto flag;\
	}\
} while (0)

void area_tree_internal_info_destroy(area_tree_internal_info_t *aii)
{
	if(aii){
		if(aii->sort_srctype_marker){
			free(aii->sort_srctype_marker);
			aii->sort_srctype_marker = NULL;
		}
		if(aii->sort_func_marker){
			free(aii->sort_func_marker);
			aii->sort_func_marker = NULL;
		}
		if(aii->sort_sem_marker){
			free(aii->sort_sem_marker);
			aii->sort_sem_marker = NULL;
		}
		free(aii);
	}
}

void area_tree_internal_info_clean(area_tree_internal_info_t *aii)
{
	if(aii){
		memset(aii->marked_srctype, 0, sizeof(aii->marked_srctype));
		memset(aii->marked_func, 0, sizeof(aii->marked_func));
		memset(aii->marked_sem, 0, sizeof(aii->marked_sem));
	}
}

area_tree_internal_info_t *area_tree_internal_info_create()
{
	area_tree_internal_info_t *aii = (area_tree_internal_info_t *)calloc(1, sizeof(area_tree_internal_info_t));
	ALLOC_FAIL_THEN_GOTO(aii, FAIL);
	return aii;
FAIL:
	area_tree_internal_info_destroy(aii);
	return NULL;
}


/**
 * @brief 是否已标注了特定的资源分块类型.
**/
bool is_marked_srctype(area_tree_t *area_tree, html_area_srctype_t type)
{
	return area_tree->internal_info->marked_srctype[type] > NOT_MARKED_YET;
}

void set_marked_srctype(area_tree_t *area_tree, html_area_srctype_t type)
{
	area_tree_internal_info_t *aii = area_tree->internal_info;
	int i_marking = aii->srctype_marker[type];
	SRCTYPE_MARK_T marker = aii->sort_srctype_marker[i_marking].type_handler;
	for(i_marking--; i_marking >= 0; i_marking--){
		if(aii->sort_srctype_marker[i_marking].type_handler != marker){
			break;
		}
	}
	/** 同一标记函数对应的不同类型也进行标记 */
	for(i_marking++; i_marking < aii->n_srctype_marker; i_marking++){
		mark_srctype_handler_t handler = aii->sort_srctype_marker[i_marking];
		if(handler.type_handler == marker){
			aii->marked_srctype[handler.srctype] = HAS_MARKED;
		}
		else{
			break;
		}
	}
}

bool is_build_srctype_list(area_tree_t *area_tree, html_area_srctype_t type)
{
	return area_tree->internal_info->marked_srctype[type] == HAS_BUILD_RESULT_LIST;
}

void set_build_srctype_list(area_tree_t *area_tree, html_area_srctype_t type)
{
	area_tree->internal_info->marked_srctype[type] = HAS_BUILD_RESULT_LIST;
}

/**
 * @brief 是否已标注了特定的功能分块类型.
**/
bool is_marked_func(area_tree_t *area_tree, html_area_func_t type)
{
	return area_tree->internal_info->marked_func[type] > NOT_MARKED_YET;
}

void set_marked_func(area_tree_t *area_tree, html_area_func_t type)
{
	area_tree_internal_info_t *aii = area_tree->internal_info;
	int i_marking = aii->func_marker[type];
	FUNC_MARK_T marker = aii->sort_func_marker[i_marking].type_handler;
	for(i_marking--; i_marking >= 0; i_marking--){
		if(aii->sort_func_marker[i_marking].type_handler != marker){
			break;
		}
	}
	/** 同一标记函数对应的不同类型也进行标记 */
	for(i_marking++; i_marking < aii->n_func_marker; i_marking++){
		mark_func_handler_t handler = aii->sort_func_marker[i_marking];
		if(handler.type_handler == marker){
			aii->marked_func[handler.func_type] = HAS_MARKED;
		}
		else{
			break;
		}
	}
}

bool is_build_func_list(area_tree_t *area_tree, html_area_func_t type)
{
	return area_tree->internal_info->marked_func[type] == HAS_BUILD_RESULT_LIST;
}

void set_build_func_list(area_tree_t *area_tree, html_area_func_t type)
{
	area_tree->internal_info->marked_func[type] = HAS_BUILD_RESULT_LIST;
}

/**
 * @brief 是否已标注了特定的语义分块类型.
**/
bool is_marked_sem(area_tree_t *area_tree, html_area_sem_t type)
{
	return area_tree->internal_info->marked_sem[type] > NOT_MARKED_YET;
}

void set_marked_sem(area_tree_t *area_tree, html_area_sem_t type)
{
	area_tree_internal_info_t *aii = area_tree->internal_info;
	int i_marking = aii->sem_marker[type];
	SEM_MARK_T marker = aii->sort_sem_marker[i_marking].type_handler;
	for(i_marking--; i_marking >= 0; i_marking--){
		if(aii->sort_sem_marker[i_marking].type_handler != marker){
			break;
		}
	}
	/** 同一标记函数对应的不同类型也进行标记 */
	for(i_marking++; i_marking < aii->n_sem_marker; i_marking++){
		mark_sem_handler_t handler = aii->sort_sem_marker[i_marking];
		if(handler.type_handler == marker){
			aii->marked_sem[handler.type] = HAS_MARKED;
		}
		else{
			break;
		}
	}
}

bool is_build_sem_list(area_tree_t *area_tree, html_area_sem_t type)
{
	return area_tree->internal_info->marked_sem[type] == HAS_BUILD_RESULT_LIST;
}

void set_build_sem_list(area_tree_t *area_tree, html_area_sem_t type)
{
	area_tree->internal_info->marked_sem[type] = HAS_BUILD_RESULT_LIST;
}

SRCTYPE_MARK_T get_marking_func(area_tree_t *area_tree, html_area_srctype_t type)
{
	area_tree_internal_info_t *aii = area_tree->internal_info;
//	Debug("marking srctype[%s]", srctype_mark_handler[type].name);
	return aii->sort_srctype_marker[aii->srctype_marker[type]].type_handler;
}

FUNC_MARK_T get_marking_func(area_tree_t *area_tree, html_area_func_t type)
{
	area_tree_internal_info_t *aii = area_tree->internal_info;
//	Debug("marking func-type[%s]", func_mark_handler[type].name);
	return aii->sort_func_marker[aii->func_marker[type]].type_handler;
}

SEM_MARK_T get_marking_func(area_tree_t *area_tree, html_area_sem_t type)
{
	area_tree_internal_info_t *aii = area_tree->internal_info;
//	Debug("marking sem-type[%s]", sem_mark_handler[type].name);
	return aii->sort_sem_marker[aii->sem_marker[type]].type_handler;
}


static void sort_array(void *array, int elem_size, int elem_num, int(*compar)(const void *, const void *))
{
	char tmp[elem_size];
	for(int i = 1; i < elem_num; ++i){
		for(int j = i-1; j >= 0; --j){
			char *elem_j = (char *)array + j * elem_size;
			char *elem_j_next = (char *)array + (j+1) * elem_size;
			if(compar(elem_j, elem_j_next) > 0){
				memcpy(tmp, elem_j, elem_size);
				memcpy(elem_j, elem_j_next, elem_size);
				memcpy(elem_j_next, tmp, elem_size);
			}
			else{
				break;
			}
		}
	}
}

static int srctype_marker_cmp(const void *a, const void *b)
{
	SRCTYPE_MARK_T p = ((const mark_srctype_handler_t *)a)->type_handler;
	SRCTYPE_MARK_T q = ((const mark_srctype_handler_t *)b)->type_handler;
	if(p > q){
		return 1;
	}
	else if(p == q){
		return 0;
	}
	else{
		return -1;
	}
}

static int func_marker_cmp(const void *a, const void *b)
{
	FUNC_MARK_T p = ((const mark_func_handler_t *)a)->type_handler;
	FUNC_MARK_T q = ((const mark_func_handler_t *)b)->type_handler;
	if(p > q){
		return 1;
	}
	else if(p == q){
		return 0;
	}
	else{
		return -1;
	}
}

static int sem_marker_cmp(const void *a, const void *b)
{
	SEM_MARK_T p = ((const mark_sem_handler_t *)a)->type_handler;
	SEM_MARK_T q = ((const mark_sem_handler_t *)b)->type_handler;
	if(p > q){
		return 1;
	}
	else if(p == q){
		return 0;
	}
	else{
		return -1;
	}
}

static int init_srctype_mark_handler(area_tree_t *area_tree)
{
	area_tree_internal_info_t *ii = area_tree->internal_info;
	if(NULL==ii->sort_srctype_marker){ //shuangwei add is null  20120331
		ii->sort_srctype_marker = (mark_srctype_handler_t *)calloc(N_SRCTYPE, sizeof(mark_srctype_handler_t));
			ALLOC_FAIL_THEN_GOTO(ii->sort_srctype_marker, FAIL);
			ii->n_srctype_marker = N_SRCTYPE;

			for(int i = 0; i < ii->n_srctype_marker; i++){
				ii->sort_srctype_marker[i] = srctype_mark_handler[i];
			}

			sort_array(ii->sort_srctype_marker, sizeof(mark_srctype_handler_t), ii->n_srctype_marker, srctype_marker_cmp);

			// init
			for(int i = 0; i < N_AREA_SRC_TYPE; i++){
				ii->srctype_marker[i] = -1;
			}

			for(int i = 0; i < ii->n_srctype_marker; i++){
				ii->srctype_marker[ii->sort_srctype_marker[i].srctype] = i;
			}

			// check
			for(int i = 0; i < N_AREA_SRC_TYPE; i++){
				if(-1 == ii->srctype_marker[i]){
					fprintf(stderr, "[%s] has no handler!\n", srctype_mark_handler[i].name);
					assert(0);
				}
			}
	}


	return 1;
FAIL:
	return -1;
}

static int init_func_mark_handler(area_tree_t *area_tree)
{
	area_tree_internal_info_t *ii = area_tree->internal_info;
	if(NULL==ii->sort_func_marker){
		ii->sort_func_marker = (mark_func_handler_t *)calloc(N_FUNC, sizeof(mark_func_handler_t));
			ALLOC_FAIL_THEN_GOTO(ii->sort_func_marker, FAIL);
			ii->n_func_marker = N_FUNC;

			for(int i = 0; i < ii->n_func_marker; i++){
				ii->sort_func_marker[i] = func_mark_handler[i];
			}

			sort_array(ii->sort_func_marker, sizeof(mark_func_handler_t), ii->n_func_marker, func_marker_cmp);

			// init
			for(int i = 0; i < N_AREA_FUNC_TYPE; i++){
				ii->func_marker[i] = -1;
			}

			for(int i = 0; i < ii->n_func_marker; i++){
				ii->func_marker[ii->sort_func_marker[i].func_type] = i;
			}
	}


	// check
//	for(int i = 0; i < N_AREA_FUNC_TYPE; i++){
//		if(-1 == ii->func_marker[i]){
//			fprintf(stderr, "[%s] has no handler!\n", func_mark_handler[i].name);
//			assert(0);
//		}
//	}

	return 1;
FAIL:
	return -1;
}

static int init_sem_mark_handler(area_tree_t *area_tree)
{
	area_tree_internal_info_t *ii = area_tree->internal_info;
	if(NULL==ii->sort_sem_marker){
		ii->sort_sem_marker = (mark_sem_handler_t *)calloc(N_SEM, sizeof(mark_sem_handler_t));
			ALLOC_FAIL_THEN_GOTO(ii->sort_sem_marker, FAIL);
			ii->n_sem_marker = N_SEM;

			for(int i = 0; i < ii->n_sem_marker; i++){
				ii->sort_sem_marker[i] = sem_mark_handler[i];
			}

			sort_array(ii->sort_sem_marker, sizeof(mark_sem_handler_t), ii->n_sem_marker,
					sem_marker_cmp);

			// init
			for(int i = 0; i < N_AREA_SEM_TYPE; i++){
				ii->sem_marker[i] = -1;
			}

			for(int i = 0; i < ii->n_sem_marker; i++){
				ii->sem_marker[ii->sort_sem_marker[i].type] = i;
			}
	}


	// check
//	for(int i = 0; i < N_AREA_SEM_TYPE; i++){
//		if(-1 == ii->sem_marker[i]){
//			fprintf(stderr, "[%s] has no handler!\n", sem_mark_handler[i].name);
//			assert(0);
//		}
//	}

	return 1;
FAIL:
	return -1;
}

int init_mark_type_handler(area_tree_t *area_tree)
{
	if(-1 == init_srctype_mark_handler(area_tree)){
		goto FAIL;
	}
	if(-1 == init_func_mark_handler(area_tree)){
		goto FAIL;
	}
	if(-1 == init_sem_mark_handler(area_tree)){
		goto FAIL;
	}
	return 1;
FAIL:
	return -1;
}

