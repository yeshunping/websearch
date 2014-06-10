/*
 * easou_ahtml_areainfo.h
 *
 *  Created on: 2011-11-18
 *      Author: ddt
 */

#ifndef EASOU_AHTML_MARKINFO_H_
#define EASOU_AHTML_MARKINFO_H_

#include "nodepool.h"
#include "easou_url.h"
#include "easou_ahtml_area.h"
#include "easou_mark_switch.h"

/**atree的 mark info定义*/
/**
 * @brief 分块列表节点．
 */
typedef struct _area_list_node_t
{
	html_area_t* area ;
	struct _area_list_node_t* next ;
}area_list_node_t ;

/**
 * @brief 分块列表结构
 */
typedef struct _area_list_t
{
	area_list_node_t* head;
	area_list_node_t* tail;
	int num ;
}area_list_t ;

#define MAX_TITLE_LEN 512

/**
* @brief mark后的信息
**/
typedef struct _mark_area_info_t
{
	area_tree_t  * area_tree ;   				/**atree*/
	char page_url[MAX_URL_LEN] ;				/**页面url*/
	char page_url_pat[MAX_URL_LEN];				/**页面url*/
	char base_href[MAX_URL_LEN] ;				/**页面baseurl*/
	char tag_title[MAX_TITLE_LEN];				/**页面tag title*/
	char anchor[MAX_TITLE_LEN];					/**反链锚文*/
	area_list_t  srctype[N_AREA_SRC_TYPE] ;		/**资源类型块*/
	area_list_t  func[N_AREA_FUNC_TYPE]  ;		/**功能类型块*/
	area_list_t  sem[N_AREA_SEM_TYPE]  ;		/**语义类型块*/
	nodepool_t area_pool ;						/**节点池*/
} mark_area_info_t;

/**
 * @brief 创建mark info
**/
mark_area_info_t *mark_info_create();

/**
 * @brief 清空mark info
**/
void mark_info_clean(mark_area_info_t * mai);

/**
 * @brief 销毁mark info
**/
void mark_info_destroy(mark_area_info_t * mai);

/**
 * @brief 清空mark info
**/
void mark_info_reset(mark_area_info_t * mai);

#endif /* EASOU_AHTML_AREAINFO_H_ */
