/*
 * easou_ahtml_innerinfo.h
 *
 *  Created on: 2011-11-23
 *      Author: ddt
 */

#ifndef EASOU_AHTML_INNERINFO_H_
#define EASOU_AHTML_INNERINFO_H_
#include "easou_mark_switch.h"
#include "easou_mark_conf.h"

/**
* @brief 标注状态.
*/
enum {
	NOT_MARKED_YET = 0,
	HAS_MARKED = 1,
	HAS_BUILD_RESULT_LIST = 2
};

typedef struct _area_tree_internal_info_t {
	char marked_srctype[N_AREA_SRC_TYPE];		  	/**< mark status of srctype */
	char marked_func[N_AREA_FUNC_TYPE];		 		/**< mark status of func-type */
	char marked_sem[N_AREA_SEM_TYPE];		  		/**< mark status of sem-type */
	int srctype_marker[N_AREA_SRC_TYPE];		  	/**< corresponding marking-handler */
	int func_marker[N_AREA_FUNC_TYPE];		  		/**< corresponding marking-handler */
	int sem_marker[N_AREA_SEM_TYPE];		  		/**< corresponding marking-handler */
	int n_srctype_marker;		  					/**< number of srctype handlers */
	int n_func_marker;		  						/**< number of func-type handlers */
	int n_sem_marker;		  						/**< number of sem-type handlers */
	mark_srctype_handler_t *sort_srctype_marker;	/**< handle-function sorted by function-address */
	mark_func_handler_t *sort_func_marker;		  	/**< handle-function sorted by function-address */
	mark_sem_handler_t *sort_sem_marker;		  	/**< handle-function sorted by function-address */
//	unsigned lazy_mark:1;		  					/**< is lazy mark: mark spec type at need; lazy in default */
} area_tree_internal_info_t;

void area_tree_internal_info_destroy(area_tree_internal_info_t *aii);

void area_tree_internal_info_clean(area_tree_internal_info_t *aii);

area_tree_internal_info_t *area_tree_internal_info_create();


/**
 * @brief 是否已标注了特定的分块类型.
**/
bool is_marked_srctype(area_tree_t *area_tree, html_area_srctype_t type);
bool is_marked_func(area_tree_t *area_tree, html_area_func_t type);
bool is_marked_sem(area_tree_t *area_tree, html_area_sem_t type);

/**
 * @brief 设置已标注了特定的分块类型.
**/
void set_marked_srctype(area_tree_t *area_tree, html_area_srctype_t type);
void set_marked_func(area_tree_t *area_tree, html_area_func_t type);
void set_marked_sem(area_tree_t *area_tree, html_area_sem_t type);

/**
 * @brief 是否已创建了特定分块类型的结果链表.
**/
bool is_build_srctype_list(area_tree_t *area_tree, html_area_srctype_t type);
bool is_build_func_list(area_tree_t *area_tree, html_area_func_t type);
bool is_build_sem_list(area_tree_t *area_tree, html_area_sem_t type);

/**
 * @brief 设置已创建了特定的分块类型结果链表.
**/
void set_build_srctype_list(area_tree_t *area_tree, html_area_srctype_t type);
void set_build_func_list(area_tree_t *area_tree, html_area_func_t type);
void set_build_sem_list(area_tree_t *area_tree, html_area_sem_t type);

/**
 * @brief 根据分块类型获取对应的标注函数.
**/
SRCTYPE_MARK_T get_marking_func(area_tree_t *area_tree, html_area_srctype_t type);
FUNC_MARK_T get_marking_func(area_tree_t *area_tree, html_area_func_t type);
SEM_MARK_T get_marking_func(area_tree_t *area_tree, html_area_sem_t type);

/**
 * @brief 初始化mark时所需的handler.
**/
int init_mark_type_handler(area_tree_t *area_tree);


#endif /* EASOU_AHTML_INNERINFO_H_ */
