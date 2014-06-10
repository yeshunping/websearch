#ifndef _NODEPOOL_H
#define _NODEPOOL_H

#include <stddef.h>
#include <stdlib.h>

#define PARSE_BLOCK_SIZE 1000

#define NULL_GOTO(SRC, DES)  \
	if(SRC == NULL) goto DES;

/**ÄÚ´æ¿é*/
typedef struct _memblock_t
{
	struct _memblock_t *next;
	char mem[0];
} memblock_t;

/**ÄÚ´æ³Ø*/
typedef struct _nodepool_t
{
	int nodesize;
	int block_size;
	memblock_t *memlist;
	char *avail;
	char *end;
	char *freelist;
} nodepool_t;

int nodepool_init(nodepool_t *pool, size_t nsize, size_t block_size = PARSE_BLOCK_SIZE);

void nodepool_reset(nodepool_t *pool);

void nodepool_destroy(nodepool_t *pool);

void *nodepool_get(nodepool_t *pool);

int nodepool_put(nodepool_t *pool, void *node);

/**
 * @brief nodepool draw back as only the first mem-block used.
 * @author xunwu
 * @date 2011/06/27
 **/
void nodepool_draw_back(nodepool_t *pool);

#endif
