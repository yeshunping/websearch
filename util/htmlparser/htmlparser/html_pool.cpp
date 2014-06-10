/**
 * @file pool.cpp
 * @author xunwu
 * @date 2011/08/03
 * @version 1.0
 * @brief Memory pool implementation
 *  
 **/

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "easou_html_pool.h"
#include "easou_html_dom.h"
#include "debuginfo.h"

/**
 * @brief create block for pool
 **/
static struct block_t* balloc(struct slab_t*, size_t);
/**
 * @brief free block of pool
 **/
static void bfree(struct slab_t*, struct block_t*);

/**
 * @brief create pool
 **/
struct mem_pool_t* mem_pool_create(struct slab_t *slab, size_t size)
{
	struct mem_pool_t *pool = NULL;
	struct block_t *blk = NULL;
	size = (size + (sizeof(intptr_t) - 1)) & -sizeof(intptr_t); /* round up */
	if (size < sizeof(*blk) + sizeof(*pool))
	{
		return NULL;
	}

	blk = balloc(slab, size);
	if (blk == NULL)
	{
		return NULL;
	}

	pool = (struct mem_pool_t*) blk->blk_last;
	assert(pool);
	pool->mp_slab = slab;
	blk->blk_last += sizeof(*pool);
	pool->mp_max = size - sizeof(struct block_t);
	SLIST_INIT(&pool->mp_chain);
	SLIST_INSERT_HEAD(&pool->mp_chain, blk, blk_entries);
	SLIST_INIT(&pool->mp_large);
	pool->mp_last = blk->blk_last;
	pool->mp_end = blk->blk_end;
	pool->mp_fragment = 0;
	return pool;
}

/**
 * @brief destroy pool
 **/
void mem_pool_destroy(struct mem_pool_t *self)
{
	struct large_chain_t *larges = NULL;
	struct large_t *large = NULL;
	struct block_chain_t *chain = NULL;
	struct block_t *blk = NULL;

	if (self == NULL)
	{
		return;
	}

	larges = &self->mp_large;
	while (!SLIST_EMPTY(larges))
	{
		large = SLIST_FIRST(larges);
		SLIST_REMOVE_HEAD(larges, lg_entries);
		free(large);
	}

	chain = &self->mp_chain;
	while (!SLIST_EMPTY(chain))
	{
		blk = SLIST_FIRST(chain);
		SLIST_REMOVE_HEAD(chain, blk_entries);
		if (SLIST_NEXT(blk, blk_entries) == NULL)
		{
			/*
			 * Since the chain (aka, list head) is in the last block,
			 * after free, chain will be unavailable,
			 * then access SLIST_EMPTY leads to a segment error
			 */
			bfree(self->mp_slab, blk);
			break;
		}
		else
		{
			bfree(self->mp_slab, blk);
		}
	}
}

/**
 * @brief alloc object from pool
 **/
void* palloc(struct mem_pool_t *self, size_t size)
{
	unsigned char *p = NULL;
	struct block_t *blk = NULL;
	struct large_t *large = NULL;

	assert(self);
	assert(size > 0);
	size = (size + (sizeof(intptr_t) - 1)) & -sizeof(intptr_t); /* round up */
	if (size <= self->mp_max)
	{
		p = self->mp_last;
		self->mp_last += size;
		if (self->mp_last > self->mp_end)
		{
			self->mp_fragment += self->mp_end - (self->mp_last - size);
			blk = balloc(self->mp_slab, self->mp_max + sizeof(struct block_t));
			if (blk == NULL)
			{
				return NULL;
			}
			SLIST_INSERT_HEAD(&self->mp_chain, blk, blk_entries);
			p = blk->blk_last;
			self->mp_last = p + size;
			self->mp_end = blk->blk_end;
		}
		return p;
	}
	else
	{
		large = (struct large_t*) malloc(size + sizeof(struct large_t));
		if (g_EASOU_DEBUG)
		{
//							char info[1000]={0};
//							sprintf(info,"malloc %d",sizeof(struct large_t) );
//							easouprintf(info);
		}
		if (large == NULL)
		{
			return NULL;
		}
		else
		{
			p = (unsigned char*) large + sizeof(struct large_t);
			SLIST_INSERT_HEAD(&self->mp_large, large, lg_entries);
			return p;
		}
	}
}

/**
 * @brief create block for pool
 **/
static struct block_t* balloc(struct slab_t *slab, size_t size)
{
	struct block_t *blk = NULL;

	assert(size >= sizeof(*blk));
	if (slab)
	{
		assert(size == slab->slb_size);
		blk = (struct block_t*) salloc(slab);
		if (blk == NULL)
		{
			return NULL;
		}
	}
	else
	{
		blk = (struct block_t*) malloc(size);
		if (g_EASOU_DEBUG == DEBUG_MEMERY)
		{
			char info[1000] =
			{ 0 };
			sprintf(info, "malloc %lu", size);
			easouprintf(info);
		}

		if (blk == NULL)
		{
			return NULL;
		}
		memset(blk, 0, sizeof(*blk));
	}

	blk->blk_end = (unsigned char*) blk + size;
	blk->blk_last = (unsigned char*) blk + sizeof(*blk);
	return blk;
}

/**
 * @brief free block of pool
 **/
static void bfree(struct slab_t *slab, struct block_t *blk)
{
	if (slab)
	{
		sfree(slab, blk);
	}
	else
	{
		free(blk);
	}
}

/**
 * @brief inspect memory pool
 **/
void mem_pool_inspect(struct mem_pool_t *self, mem_pool_inspector_t inspector)
{
	inspector(self, "fragment", self->mp_fragment);
	inspector(self, "unused", self->mp_fragment + (self->mp_end - self->mp_last));
}

/**
 * @brief create slab
 **/
struct slab_t* slab_create(struct mem_pool_t *pool, size_t size)
{
	struct slab_t *slab = NULL;
	size = (size + (sizeof(intptr_t) - 1)) & -sizeof(intptr_t); /* round up */
	if (pool)
	{
		slab = (struct slab_t*) palloc(pool, sizeof(*slab));
	}
	else
	{
		slab = (struct slab_t*) malloc(sizeof(*slab));
		if (g_EASOU_DEBUG == DEBUG_MEMERY)
		{
			char info[1000] =
			{ 0 };
			sprintf(info, "malloc %lu", sizeof(*slab));
			easouprintf(info);
		}
	}
	if (slab == NULL)
	{
		return NULL;
	}
	memset(slab, 0, sizeof(*slab));
	size = (size + (sizeof(intptr_t) - 1)) & -sizeof(intptr_t); /* round up */
	slab->slb_size = size;
	slab->slb_pool = pool;
	slab->slb_first = NULL;
	slab->slb_free = 0;
	return slab;
}

/**
 * @brief destroy slab
 **/
void slab_destroy(struct slab_t *slab)
{
	struct slab_item_t *si = NULL;
	struct slab_item_t *next = NULL;
	if (!slab)
	{
		return;
	}
	if (!slab->slb_pool)
	{
		for (si = slab->slb_first; si; si = next)
		{
			next = si->slb_next;
			free(si);
		}
	}
	slab->slb_free = 0;
	slab->slb_size = 0;
	slab->slb_first = NULL;
	if (!slab->slb_pool)
	{
		free(slab);
	}
	else
	{
		slab->slb_pool = NULL;
	}
}

/**
 * @brief alloc object from slab
 **/
void* salloc(struct slab_t *slab)
{
	struct slab_item_t *si = NULL;
	void *ptr = NULL;
	assert(slab);
	if (slab->slb_first && slab->slb_free > 0)
	{
		assert(slab->slb_free > 0);
		slab->slb_free--;
		si = slab->slb_first;
		slab->slb_first = si->slb_next;
		memset(si, 0, slab->slb_size);
		return si;
	}
	else
	{
		if (slab->slb_pool)
		{
			ptr = palloc(slab->slb_pool, slab->slb_size);
		}
		else
		{
			ptr = malloc(slab->slb_size);
			if (g_EASOU_DEBUG == DEBUG_MEMERY)
			{
				char info[1000] =
				{ 0 };
				sprintf(info, "malloc %lu", slab->slb_size);
				easouprintf(info);
			}
		}
		if (ptr == NULL)
		{
			return NULL;
		}
		memset(ptr, 0, slab->slb_size);
		return ptr;
	}
}

/**
 * @brief free object to slab
 **/
void sfree(struct slab_t *slab, void *ptr)
{
	struct slab_item_t *si = NULL;
	assert(slab);
	if (ptr == NULL)
	{

	}
	else
	{
		si = (struct slab_item_t*) ptr;
		if (slab->slb_first)
		{
			si->slb_next = slab->slb_first;
		}
		else
		{
			si->slb_next = NULL;
		}
		slab->slb_first = si;
		slab->slb_free++;
	}
}

/**
 * @brief free half free list
 * @note only for malloc slab
 **/
void slab_gc(struct slab_t *slab)
{
	struct slab_item_t *si = NULL;
	struct slab_item_t *mem = NULL;
	int cnt = 0;
	assert(slab);
	if (slab->slb_pool || slab->slb_free < 2)
	{
		return;
	}
	si = slab->slb_first;
	for (cnt = slab->slb_free / 2; cnt > 1; cnt--)
	{
		si = si->slb_next;
	}
	while (si->slb_next)
	{
		mem = si->slb_next;
		si->slb_next = mem->slb_next;
		free(mem);
	}
	slab->slb_free /= 2;
}

/**
 * 复位时不释放初始空间
 */
void mem_pool_reset(struct mem_pool_t *self)
{
	struct large_chain_t *larges = NULL;
	struct large_t *large = NULL;
	struct block_chain_t *chain = NULL;
	struct block_t *blk = NULL;

	if (self == NULL)
	{
		return;
	}

	larges = &self->mp_large;
	while (!SLIST_EMPTY(larges))
	{
		large = SLIST_FIRST(larges);
		SLIST_REMOVE_HEAD(larges, lg_entries);
		free(large);
	}

	chain = &self->mp_chain;
	while (!SLIST_EMPTY(chain) && SLIST_NEXT(SLIST_FIRST(chain),blk_entries))
	{
		blk = SLIST_FIRST(chain);
		SLIST_REMOVE_HEAD(chain, blk_entries);
		if (SLIST_NEXT(blk, blk_entries) == NULL)
		{
			/*
			 * Since the chain (aka, list head) is in the last block,
			 * after free, chain will be unavailable,
			 * then access SLIST_EMPTY leads to a segment error
			 */
			bfree(self->mp_slab, blk);
			break;
		}
		else
		{
			bfree(self->mp_slab, blk);
		}
	}
	blk = SLIST_FIRST(chain);
	SLIST_INIT(&self->mp_large);
	self->mp_last = blk->blk_last;
	self->mp_end = blk->blk_end;
	self->mp_fragment = 0;
}
