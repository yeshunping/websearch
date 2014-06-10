/**
 * @file mem-mgr/id_access.h
 * @author xunwu
 * @date 2011/06/27
 * @version 1.0
 * @brief 通过唯一的顺序编号存取信息.有类型无关存取和类型相关存取两种方式.
 **/

#ifndef  __ID_MAP_H_
#define  __ID_MAP_H_

#include "nodepool.h"

/**
* @brief 一个内存块节点.
*/
typedef struct _map_unit_t {
	struct _map_unit_t *next;
	void *nodemap[0];
} map_unit_t;

/**
* @brief 属性存取内存管理结构.
*/
typedef struct _idmap_t{
	map_unit_t *head;		  /**< 内存块链表头指针       */
	map_unit_t *tail;		  /**< 内存块链表尾指针       */
	int map_unit_size;		  /**< 一个内存块的大小       */
	nodepool_t nodepool;	  /**< 用于存储用户插入的节点       */
}idmap_t;

/**
* @brief 用于hash结构的节点.
*/
typedef struct _map_node_t{
	struct _map_node_t *next;		  /**< 用于构成节点链表       */
	void *usr_node;		  /**< 指向用户插入的节点       */
	int id;		  /**< 用户插入节点对应的id       */
} map_node_t;

/**
* @brief 类型相关的属性存取内存管理结构.
*/
typedef struct _id_spec_map_t{
	nodepool_t map_nodepool;		  /**< 用于hash结构的节点池       */
	nodepool_t usr_nodepool;		  /**<用于管理用户节点 */
	int bucket_size;		  /**<HASH的桶大小 */
	int node_num;		  /**< 已插入的节点数量 */
	map_node_t *bucket[0];		  /**< HASH桶 */
} id_spec_map_t;

/**
 * @brief 重置内存管理结构.
 * @author xunwu
 * @date 2011/06/27
**/
void idmap_reset(idmap_t *idmap);

/**
 * @brief 创建属性存取的内存管理结构.
 * @param [in] node_size   : int	需要存取的节点的大小.
 * @param [in] init_node_num   : int	预先分配的节点数量.
 * @return  idmap_t*	失败返回NULL,否则返回创建的结构.
 * @author xunwu
 * @date 2011/06/27
**/
idmap_t *idmap_create(int node_size, int init_node_num);

/**
 * @brief 销毁属性存取内存管理结构.
 * @author xunwu
 * @date 2011/06/27
**/
void idmap_destroy(idmap_t *idmap);

/**
 * @brief 根据id值获取对应的节点.
 * @param [in] idmap   : idmap_t*	节点存取结构.
 * @param [in] id   : int	想要获取的id值.
 * @return  void*	失败返回NULL,否则返回对应的节点.
 * @author xunwu
 * @date 2011/06/27
**/
void *idmap_get_node(idmap_t *idmap, int id);

#define DEFAULT_BUCKET_SIZE	113		  /**< 默认的HASH桶大小,一个较小的素数 */

/**
 * @brief 类型相关属性存取内存管理结构创建.
 * @param [in] node_size   : int	需要存取的节点大小
 * @param [in] init_node_num   : int	预先分配的节点数量
 * @param [in] bucket_size   : int	HASH桶大小
 * @return  id_spec_map_t* 
 * @author xunwu
 * @date 2011/06/27
**/
id_spec_map_t *id_spec_map_create(int node_size, int init_node_num, int bucket_size=DEFAULT_BUCKET_SIZE);

/**
 * @brief 类型相关属性存取内存管理结构销毁.
 * @author xunwu
 * @date 2011/06/27
**/
void id_spec_map_destroy(id_spec_map_t *ism);

/**
 * @brief 类型相关属性存取结构重置.
 * @author xunwu
 * @date 2011/06/27
**/
void id_spec_map_clean(id_spec_map_t *ism);

/**
 * @brief 向类型相关属性存取结构中添加一个键值==id的节点.如果该id对应的节点已存在，则返回该节点.
 * @return  void* 	返回插入的节点指针,失败返回NULL.
 * @author xunwu
 * @date 2011/06/27
**/
void *id_spec_map_add_node(id_spec_map_t *ism, int id);

/**
 * @brief 根据id值获取对应的节点.
 * @return  void*	若该节点存在返回该节点指针,否则返回NULL.
 * @author xunwu
 * @date 2011/06/27
**/
void *id_spec_map_get_node(id_spec_map_t *ism, int id);

/**
 * @brief 删除一个键值==id的节点.
 * @author xunwu
 * @date 2011/06/27
**/
void id_spec_map_free_node(id_spec_map_t *ism, int id);

/**
 * @brief 获取当前的节点数量.
 * @author xunwu
 * @date 2011/06/27
**/
int id_spec_map_get_node_num(id_spec_map_t *ism);

#endif  //__MEM-MGR/__ID_MAP_H_

