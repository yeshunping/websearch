/**
 * @file easou_vstruct_profiler.h
 * @author xunwu
 * @date 2011/06/27
 * @version 1.0 
 * @brief for analyze sub-tree structure
 *  
 **/

#ifndef  __EASOU_VSTRUCT_PROFILER_H_
#define  __EASOU_VSTRUCT_PROFILER_H_

#include "easou_vhtml_dtd.h"

typedef struct _html_vnode_t html_vnode_t;
typedef struct _html_vtree_t html_vtree_t;

/**
 * @brief 子树结构信息.
 */
typedef struct
{
	int max_depth; /**< 当前子树叶节点的最大深度 */
	int valid_child_num; /**< 有效的子节点数量 */
	int valid_node_num; /**< 当前子树的有效节点数量,忽略空白文本节点 */
	int valid_leaf_num; /**< 当前子树的有效叶子节点数量 */
	int hr_num; /**< 当前子树的hr标签个数 */
	int interaction_tag_num; /**< 当前子树input/select/textArea/button标签个数 */

	unsigned int is_repeat_with_sibling :1; /**< 是否重复子结构,即与兄弟子树结构相似 */
	unsigned int is_self_repeat :1; /**< 是否重复父结构,即它的子结构"主要"由相互重复的子树组成 */

	char self_similar_value; /**< 对于重复父结构才有此值:当前子树的自相似度(0~100)*/
	int repeat_type_num; /**< 对于重复父结构才有此值:其子结点间重复"类型"的数量 */

	/** 以下字段对于重复子结构才有意义 */
	int repeat_num; /**< 与它相似的兄弟子树的数量+1 */
	int similar_sign; /**< 非重复结构similar_sign=-1;
	 similar_sign>=0时,兄弟节点相似当且仅当similar_sign相同 */
	char align_percent; /**< 与相似的兄弟子树的平均节点对齐比例(0~100) */
	char similar_value; /**< 与相似的兄弟子树的相似度,可理解为相似的概率(0~100) */
	html_vnode_t *prev_similar_vnode; /**< 上一个相似兄弟节点 */
	html_vnode_t *next_similar_vnode; /**< 下一个相似兄弟节点 */

	/** below is only for inner use */
	html_vnode_t *align_vnode_for_trace; /**< 用于求对齐节点时记录回溯信息 */
	unsigned int is_best_aligned_for_trace :1; /**< 用于求对齐节点时记录回溯信息 */
} vstruct_info_t;

enum
{
	ADD_NORMAL_STRUCT_INFO = 0x1, /**< 计算普通结构信息 */
	ADD_REPEAT_STRUCT_INFO = 0x1 << 1, /**< 计算重复结构信息 */
	ADD_ALL_STRUCT_INFO = (unsigned) -1 /**< 计算所有结构信息 */
};

/**
 * @brief 对VTREE的每个节点添加子树结构信息.
 * @param [in&out] html_vtree   : html_vtree_t* 已解析完成的VTREE.
 * @param [in] flag   : unsigned int 用于控制功能的开关.
 * @return  int -1:出错; 1:成功.
 * @author xunwu
 * @date 2011/06/27
 **/
int vhtml_struct_prof(html_vtree_t *html_vtree, unsigned int flag = ADD_ALL_STRUCT_INFO);

/**
 * @brief 最大公共子树信息.
 */
typedef struct
{
	int align_node_cnt; /**< 两子树对齐的节点树 */
	int similar_depth; /**< 对齐节点的最大深度 */
} max_common_tree_info_t;

/**
 * @brief 遍历最大公共子树时提供的回调函数.
 * @param [in] vnode : 第一个子树中的节点.
 * @param [in] align_vnode : 第二个子树中与vnode对齐的节点.
 * @param [in & out] data   : 用户自定义数据.
 * @retval  
 * 			VISIT_NORMAL: 正常遍历公共子树.
 * 			VISIT_SKIP_CHILD: 当前节点的子树不再遍历.
 * 			VISIT_ERROR: 访问出错.
 * 			VISIT_FINISH: 访问中止.
 * @author xunwu
 * @date 2011/06/27
 **/
typedef int (*COMMON_TREE_TRAVEL_FUNC)(html_vnode_t *vnode, html_vnode_t *align_vnode, void *data);

/**
 * @brief 遍历最大公共子树.
 * @param [in] vnode   : html_vnode_t* 待对齐的子树1.
 * @param [in] another_vnode   : html_vnode_t* 待对齐的子树2.
 * @param [in] vtree   : html_vtree_t* 子树1所在的vtree.
 * @param [in] another_vtree   : html_vtree_t* 子树2所在的vtree.
 * @param [in] start_travel_common_tree   : COMMON_TREE_TRAVEL_FUNC 先序遍历公共子树函数.
 * @param [in] finish_travel_common_tree   : COMMON_TREE_TRAVEL_FUNC 后序遍历公共子树函数.
 * @param [in & out] data   : void* 用户自定义数据.
 * @return  max_common_tree_info_t 两子树的最大公共子树的对齐信息.
 * @author xunwu
 * @date 2011/06/27
 **/
max_common_tree_info_t travel_max_common_tree(html_vnode_t *vnode, html_vnode_t *another_vnode, html_vtree_t *vtree,
		html_vtree_t *another_vtree, COMMON_TREE_TRAVEL_FUNC start_travel_common_tree, COMMON_TREE_TRAVEL_FUNC finish_travel_common_tree,
		void *data);

/** XXX: BELOW IS FOR INNER USE ONLY : */

/**
 * @brief 删除为结构信息分配的内存.
 * @author xunwu
 * @date 2011/06/27
 **/
void vhtml_struct_prof_del(html_vtree_t * html_vtree);

#endif  //__EASOU_VSTRUCT_PROFILER_H_
