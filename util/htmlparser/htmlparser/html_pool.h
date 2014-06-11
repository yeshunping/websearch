/**
 * @file pool.h

 * @date 2011/08/03
 * @version 1.0
 * @brief Memory pool
 *  
 **/

#ifndef  __POOL_H_
#define  __POOL_H_

#include <stddef.h>
#include "queue.h"

/**
 * @brief block list head
 **/
SLIST_HEAD(block_chain_t, block_t);

/**
 * @brief large object
 **/
struct large_t
{
	size_t lg_size; /* size */
	SLIST_ENTRY(large_t) lg_entries; /* list entry */
};
/**
 * @brief large object list head
 **/
SLIST_HEAD(large_chain_t, large_t);

/**
 * @brief memory pool
 **/
struct mem_pool_t
{
	unsigned char *mp_last; /* current usable memory pointer */
	unsigned char *mp_end; /* current memory bound */
	size_t mp_max; /* max size can be allocated from pool */
	size_t mp_fragment; /* block fragment */
	struct block_chain_t mp_chain; /* blocks */
	struct large_chain_t mp_large; /* large objects (>mp_max)*/
	struct slab_t *mp_slab; /* slab backend, optional */
};

/**
 * @brief free list object
 **/
struct slab_item_t
{
	struct slab_item_t *slb_next; /* free list entry */
};
/**
 * @brief slab
 **/
struct slab_t
{
	struct mem_pool_t *slb_pool; /* pool backend, optional */
	size_t slb_size; /* object size */
	struct slab_item_t *slb_first; /* free list head */
	int slb_free; /* free list object count */
};

/**
 * @brief block
 **/
struct block_t
{
	unsigned char *blk_last; /* available memory pointer */
	unsigned char *blk_end; /* bound */
	SLIST_ENTRY(block_t) blk_entries; /* block list entry */
};

/**
 * @brief create pool
 **/
struct mem_pool_t* mem_pool_create(struct slab_t*, size_t);
/**
 * @brief destroy pool
 **/
void mem_pool_destroy(struct mem_pool_t*);
/**
 * @brief alloc from pool
 **/
void* palloc(struct mem_pool_t*, size_t);
/**
 * @brief memory pool inspector type
 **/
typedef void (*mem_pool_inspector_t)(struct mem_pool_t*, const char*, int);
/**
 * @brief inspect memory pool
 **/
void mem_pool_inspect(struct mem_pool_t*, mem_pool_inspector_t);

/**
 * @brief create slab
 **/
struct slab_t *slab_create(struct mem_pool_t*, size_t);
/**
 * @brief destroy slab
 **/
void slab_destroy(struct slab_t*);
/**
 * @brief allocate object from slab
 **/
void* salloc(struct slab_t*);
/**
 * @brief free object to slab
 **/
void sfree(struct slab_t*, void*);
/**
 * @brief slab gc
 * @note free half free list, only for malloc
 **/
void slab_gc(struct slab_t*);

/**
 * 复位内存池
 */
void mem_pool_reset(struct mem_pool_t *self);

/**
 * @brief Define a slab
 **/
#define SLAB_DEFINE(name, type) \
    static inline struct type* salloc_##name(struct slab_t *slab) { \
        return (struct type*)salloc((slab)); \
    } \
    static inline void sfree_##name(struct slab_t *slab, struct type *obj) { \
        sfree((slab), (obj)); \
    }

#endif  /* __POOL_H_ */

