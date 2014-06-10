#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "nodepool.h"

using namespace EA_COMMON;

static inline void nodepool_renew(nodepool_t *pool)
{
	pool->avail = pool->memlist->mem;
	pool->end = pool->memlist->mem + pool->block_size;
	pool->freelist = NULL;
}

static int nodepool_alloc(nodepool_t *pool)
{
	memblock_t *m = NULL;
	if ((m = (memblock_t *) malloc(sizeof(memblock_t) + pool->block_size)) == NULL)
	{
		Fatal("malloc error!");
		return 0;
	}
	m->next = pool->memlist;
	pool->memlist = m;
	nodepool_renew(pool);
	return 1;
}

int nodepool_init(nodepool_t *pool, size_t node_size, size_t block_node_num)
{
	memset(pool, 0, sizeof(nodepool_t));
	pool->nodesize = node_size;
	pool->block_size = pool->nodesize * block_node_num;
	if (!nodepool_alloc(pool))
	{
		return 0;
	}
	return 1;
}

static void memlist_free(memblock_t *memlist)
{
	memblock_t *cur = memlist;
	while (cur)
	{
		memblock_t *next = cur->next;
		free(cur);
		cur = next;
	}
}

void nodepool_reset(nodepool_t *pool)
{
	assert(pool->memlist);
	do
	{
		memblock_t *p = pool->memlist->next;
		if (!p)
			break;
		free(pool->memlist);
		pool->memlist = p;
	} while (1);
	nodepool_renew(pool);
}

/**
 * @brief nodepool draw back as only the first mem-block used.
 * @author xunwu
 * @date 2011/06/27
 **/
void nodepool_draw_back(nodepool_t *pool)
{
	nodepool_reset(pool);
	pool->avail = pool->end;
}

void nodepool_destroy(nodepool_t *pool)
{
	memlist_free(pool->memlist);
	memset(pool, 0, sizeof(nodepool_t));
}

static void *nodepool_get_from_freelist(nodepool_t *pool)
{
//	Debug("reuse a put back node.");
	char *node = pool->freelist;
	pool->freelist = *(char **) node;
	return node;
}

void *nodepool_get(nodepool_t *pool)
{
	if (pool->avail >= pool->end)
	{
		if (pool->freelist)
		{
			return nodepool_get_from_freelist(pool);
		}
		if (!nodepool_alloc(pool))
		{
			return NULL;
		}
	}
	void *node = pool->avail;
	pool->avail += pool->nodesize;
	return node;
}

int nodepool_put(nodepool_t *pool, void *node)
{
	if (pool->nodesize >= (int) sizeof(char *))
	{
		*((char **) node) = pool->freelist;
		pool->freelist = (char *) node;
//		Debug("put back a node.");
	}
	return 1;
}

