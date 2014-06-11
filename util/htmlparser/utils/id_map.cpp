/**
 * @file mem-mgr/id_map.cpp

 * @date 2011/06/27
 * @version 1.0 
 * @brief 对每个或大多数id值都要存取信息的情况下所使用的id map.
 *  
 **/
#include <assert.h>
#include <stdlib.h>
#include "log.h"
#include "id_map.h"

using namespace EA_COMMON;

/**
 * @brief 释放内存块链表.

 * @date 2011/06/27
**/
static void map_unit_list_free(map_unit_t *list)
{
	while(list){
		map_unit_t *next = list->next;
		free(list);
		list = next;
	}
}

/**
 * @brief 重置内存管理结构.

 * @date 2011/06/27
**/
void idmap_reset(idmap_t *idmap)
{
	if(idmap->head){
		map_unit_list_free(idmap->head->next);
		idmap->head->next = NULL;
	}
	idmap->tail = idmap->head;
	nodepool_draw_back(&idmap->nodepool);
}

/**
 * @brief 分配新的内存并挂在链表上.
 * @retval  -1:出错;1:成功.

 * @date 2011/06/27
**/
static int idmap_alloc(idmap_t *idmap)
{
	int bsize = sizeof(map_unit_t) + sizeof(void *) * idmap->map_unit_size;
	map_unit_t *mu = (map_unit_t *)calloc(bsize, 1);
	NULL_GOTO(mu, ERR);
	if(idmap->tail){
		idmap->tail->next = mu;
	}
	else{
		idmap->head = mu;
	}
	idmap->tail = mu;
	
	return 1;
ERR:
	return -1;
}

/**
 * @brief 销毁属性存取内存管理结构.

 * @date 2011/06/27
**/
void idmap_destroy(idmap_t *idmap)
{
	if(idmap){
		map_unit_list_free(idmap->head);
		nodepool_destroy(&idmap->nodepool);
		free(idmap);
	}
}

/**
 * @brief 将最初分配并长期保持的idmap的指针与nodepool中的节点对应起来，它们的初始数量必须相同。

 * @date 2011/07/12
**/
static void idmap_set_initial_nodes(idmap_t *idmap)
{
	for(int i = 0; i < idmap->map_unit_size; i++){
		assert(idmap->nodepool.avail < idmap->nodepool.end);
		idmap->head->nodemap[i] = nodepool_get(&idmap->nodepool);
	}
}

/**
 * @brief 创建属性存取的内存管理结构.
 * @param [in] node_size   : int	需要存取的节点的大小.
 * @param [in] init_node_num   : int	预先分配的节点数量.
 * @return  idmap_t*	失败返回NULL,否则返回创建的结构.

 * @date 2011/06/27
**/
idmap_t *idmap_create(int node_size, int init_node_num)
{
	idmap_t *idmap = (idmap_t *)calloc(1, sizeof(idmap_t));
	NULL_GOTO(idmap,ERR);
	idmap->map_unit_size = init_node_num;
	if(idmap_alloc(idmap) == -1){
		Fatal("create idmap error!");
		goto ERR;
	}
	if(nodepool_init(&idmap->nodepool, node_size, idmap->map_unit_size) == 0){
		goto ERR;
	}
	idmap_set_initial_nodes(idmap);

	return idmap;
ERR:
	idmap_destroy(idmap);
	return NULL;
}

/**
 * @brief 根据id值获取对应的节点.
 * @param [in] idmap   : idmap_t*	节点存取结构.
 * @param [in] id   : int	想要获取的id值.
 * @return  void*	失败返回NULL,否则返回对应的节点.

 * @date 2011/06/27
**/
void *idmap_get_node(idmap_t *idmap, int id)
{
	if(id < 0){
		Fatal("Illegal id, id should >= 0.");
		goto ERR;
	}

	if(id < idmap->map_unit_size){
		return idmap->head->nodemap[id];
	}
	{
		map_unit_t *mu = idmap->head;
		do{
			mu = mu->next;
			id -= idmap->map_unit_size;
			if(NULL == mu){
				if(idmap_alloc(idmap) == -1){
					goto ERR;
				}
				mu = idmap->tail;
			}
		} while (id >= idmap->map_unit_size);
		if(NULL == mu->nodemap[id]){
			mu->nodemap[id] = nodepool_get(&idmap->nodepool);
		}
		return mu->nodemap[id];
	}
ERR:
	return NULL;
}


/**
 * @brief 类型相关属性存取内存管理结构销毁.

 * @date 2011/06/27
**/
void id_spec_map_destroy(id_spec_map_t *ism)
{
	if(ism){
		nodepool_destroy(&ism->map_nodepool);
		nodepool_destroy(&ism->usr_nodepool);
		free(ism);
	}
}

/**
 * @brief 类型相关属性存取内存管理结构创建.
 * @param [in] node_size   : int	需要存取的节点大小
 * @param [in] init_node_num   : int	预先分配的节点数量
 * @param [in] bucket_size   : int	HASH桶大小
 * @return  id_spec_map_t*

 * @date 2011/06/27
**/
id_spec_map_t *id_spec_map_create(int node_size, int init_node_num, int bucket_size)
{
	int alloc_size = sizeof(id_spec_map_t) + sizeof(map_node_t *) * bucket_size;
	id_spec_map_t *ism = (id_spec_map_t *) calloc(1, alloc_size);
	NULL_GOTO(ism,ERR);
	ism->bucket_size = bucket_size;
	if(nodepool_init(&ism->map_nodepool, sizeof(map_node_t), init_node_num) == 0) {
		goto ERR;
	}
	if(nodepool_init(&ism->usr_nodepool, node_size, init_node_num) == 0){
		goto ERR;
	}
	return ism;
ERR:
	id_spec_map_destroy(ism);
	return NULL;
}

/**
 * @brief 类型相关属性存取结构重置.

 * @date 2011/06/27
**/
void id_spec_map_clean(id_spec_map_t *ism)
{
	for(int i = 0; i < ism->bucket_size; i++){
		ism->bucket[i] = NULL;
	}
	ism->node_num = 0;
	nodepool_reset(&ism->map_nodepool);
	nodepool_reset(&ism->usr_nodepool);
}

#define SIGN_MASK	(0x7fffffff)
#define ID_TO_MAP_INDEX(id,map)	((id & SIGN_MASK) % ism->bucket_size) /**< 将ID转化为HASH桶的对应位置 */

/**
 * @brief 向类型相关属性存取结构中添加一个键值==id的节点.
 * @return  void* 	返回插入的节点指针,失败返回NULL.

 * @date 2011/06/27
**/
void *id_spec_map_add_node(id_spec_map_t *ism, int id)
{
	int index = ID_TO_MAP_INDEX(id,ism);
	/** 如果该id对应的节点已存在，则返回该节点. */
	for(map_node_t *mn = ism->bucket[index]; mn; mn = mn->next){
		if(mn->id == id){
			return mn->usr_node;
		}
	}
	++ ism->node_num;
	map_node_t * mnode = (map_node_t *)nodepool_get(&ism->map_nodepool);
	void *unode = nodepool_get(&ism->usr_nodepool);
	if(NULL == mnode || NULL == unode){
		goto ERR;
	}
	mnode->usr_node = unode;
	mnode->id = id;
	mnode->next = ism->bucket[index];
	ism->bucket[index] = mnode;
	return unode;
ERR:
	return NULL;
}

/**
 * @brief 删除一个键值==id的节点.

 * @date 2011/06/27
**/
void id_spec_map_free_node(id_spec_map_t *ism, int id)
{
	int index = ID_TO_MAP_INDEX(id, ism);
	-- ism->node_num;
	map_node_t **bucket = ism->bucket;
	map_node_t **prev = bucket + index;
	for(map_node_t *mn = bucket[index]; mn; mn = mn->next){
		if(mn->id == id){
			nodepool_put(&ism->usr_nodepool, mn->usr_node);
			*prev = mn->next;
			nodepool_put(&ism->map_nodepool, mn);
		}
		prev = &mn->next;
	}
}

/**
 * @brief 根据id值获取对应的节点.
 * @return  void*	若该节点存在返回该节点指针,否则返回NULL.

 * @date 2011/06/27
**/
void *id_spec_map_get_node(id_spec_map_t *ism, int id)
{
	int index = ID_TO_MAP_INDEX(id, ism);
	map_node_t **bucket = ism->bucket;
	for(map_node_t *mn = bucket[index]; mn; mn = mn->next){
		if(mn->id == id){
			return mn->usr_node;
		}
	}
	return NULL;
}

/**
 * @brief 获取当前的节点数量.

 * @date 2011/06/27
**/
int id_spec_map_get_node_num(id_spec_map_t *ism)
{
	return ism->node_num;
}

