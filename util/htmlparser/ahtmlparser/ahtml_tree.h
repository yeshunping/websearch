/**
 * @file ahtml_tree.h

 * @date 2011/06/27
 * @version 1.0 
 * @brief 在vtree的基础上进行分块操作
 *  
 **/

#ifndef  __EASOU_AHTML_TREE_H
#define  __EASOU_AHTML_TREE_H

#include "util/htmlparser/ahtmlparser/ahtml_area.h"

typedef struct _atree_baseinfo_t atree_baseinfo_t;

/**
 * @brief 预先声明
 */
typedef struct _mark_area_info_t	mark_area_info_t;
typedef struct _area_baseinfo_mgr_t	area_baseinfo_mgr_t;
typedef struct _area_uid_binfo_mgr_t	area_uid_binfo_mgr_t;
typedef struct _function_switch_t	function_switch_t;
typedef struct _area_tree_internal_info_t area_tree_internal_info_t;
/**
 * @brief 分块树的结构．
 */
typedef struct _area_tree_t {
	html_vtree_t *hp_vtree;       /**< 分块树对应的vtree  */
	html_area_t *root;	  		  /**< 分块树的根节点  */
	unsigned int area_num;	      /**< 分块节点数量    */
	nodepool_t np;		          /**< 分块树的节点池  */
	area_config_t config;	      /**< 分块树的配置    */
	unsigned int mark_status;     /**< 树的标注状态    */
//	area_uid_binfo_mgr_t *area_uid_binfo_mgr;/**< */
	area_baseinfo_mgr_t * area_binfo_mgr ;   /**< 初始化时为空，需要时加载，内部用 */
	mark_area_info_t * mark_info ;           /**< 初始化时为空，需要时加载, 存储标注结果信息 */
	function_switch_t *function_switch;		 /**< 功能开关 */
	area_tree_internal_info_t *internal_info;/**内部用的mark信息*/
	atree_baseinfo_t *base_info;
	int max_depth;//tree max depth
}area_tree_t;


/**
 * @brief	创建一棵分块树.
 * @param [in] cfg   : area_config_t*	用于配置分块树的分块粒度.可为NULL.
 * @return  area_tree_t*	已分配空间的分块树.

 * @date 2011/07/05
 **/
area_tree_t *area_tree_create(area_config_t *cfg);

/**
 * @brief 设置分块树的分块粒度.

 * @date 2011/07/05
 **/
void area_tree_setConfig(area_tree_t *atree,area_config_t *cfg);

/**
 * @brief 删除分块树.

 * @date 2011/07/05
 **/
void area_tree_del(area_tree_t *atree);

/**
 * @brief 清除一棵分块树上的分块信息.

 * @date 2011/07/05
 **/
void area_tree_clean(area_tree_t *atree);

/**
 * @brief 在VTREE上划分分块树.
 * @param [out] atree   : area_tree_t*	分块树.
 * @param [in] vtree   : const html_vtree_t*	待划分的VTREE.
 * @param [in] base_url : 网页原始url
 * @return  int 
 * @retval  -1:分块出错;1:成功.

 * @date 2011/07/05
 **/
int area_partition(area_tree_t *atree, html_vtree_t *vtree, const char *base_url);

/**
 * @brief 划分一个分块.
 * @param [in] area   : html_area_t*	待划分的分块.
 * @param [in] cfg   : const area_config_t*	分块的粒度配置.
 * @param [in/out] np   : nodepool_t*	分块节点池.
 * @param [in] depth   : unsigned int	待划分的分块的深度.`
 * @return  int 
 * @retval   -1:分块出错.1:成功.

 * @date 2011/07/05
 **/
int areaNode_divide(html_area_t *area,const area_config_t *cfg,nodepool_t *np, unsigned int depth);

/*块树的先续遍历函数定义*/
typedef int (* FUNC_START_T)(html_area_t * ,void * ) ;
/*块树的后续遍历函数定义*/
typedef int (* FUNC_FINISH_T)(html_area_t * ,void * ) ;

#define AREA_VISIT_ERR	(-1)
#define AREA_VISIT_NORMAL  1
#define AREA_VISIT_FINISH  2
#define AREA_VISIT_SKIP 3

/**
 * @brief 分块树的遍历函数，同 html_tree_visit 类似
 * @param [in/out] atree   : area_tree_t*
 * @param [in/out] start   : FUNC_START_T
 * @param [in/out] finish   : FUNC_FINISH_T
 * @param [in/out] data   : void*
 * @return  bool 
 * @retval   成功 true ，失败false

 * @date 2011/07/05
 **/
bool areatree_visit(area_tree_t * atree, FUNC_START_T start,  FUNC_FINISH_T finish, void * data ) ;

/**
 * @brief 重载函数，见上面说明
 * @param [in/out] arearoot   : html_area_t*
 * @param [in/out] start   : FUNC_START_T
 * @param [in/out] finish   : FUNC_FINISH_T
 * @param [in/out] data   : void*
 * @return  int 
 * @retval   失败0 ，成功>0

 * @date 2011/07/05
 **/
int areatree_visit(html_area_t * arearoot, FUNC_START_T start,  FUNC_FINISH_T finish, void * data);

/**
 * print area tree
 */
void printAtree(area_tree_t * atree);

void printArea(html_area_t * area,int level);

void printSingleArea(html_area_t * area);
#endif  //__EASOU_AHTML_TREE_H

