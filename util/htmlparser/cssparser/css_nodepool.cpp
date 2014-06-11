
#include <stdio.h>
#include <stdlib.h>

#include "base/logging.h"
#include "css_nodepool.h"

/**
 * @brief 分配新的内存管理节点.
 * @param [in] size   : size_t	新内存块大小.
 * @return  css_mem_node_t*	新的内存管理节点.
 * @retval	失败返回NULL.

 * @date 2011/06/20
 **/
static css_mem_node_t *css_new_mem_node(size_t size) {
  /* alloc mem node */
  css_mem_node_t *pnode = (css_mem_node_t *) calloc(sizeof(css_mem_node_t), 1);
  if (NULL == pnode) {
    LOG(ERROR) << "malloc error!";
    goto FAIL_CNMN;
  }
  /* alloc mem */
  pnode->p_mem = calloc(size, 1);
  if (NULL == pnode->p_mem) {
    LOG(ERROR) << "malloc error!";
    goto FAIL_CNMN;
  }
  pnode->mem_size = size;
  pnode->next = NULL;
  return pnode;

  FAIL_CNMN: if (pnode != NULL) {
    if (pnode->p_mem != NULL) {
      free(pnode->p_mem);
      pnode->p_mem = NULL;
    }
    free(pnode);
    pnode = NULL;
  }
  return NULL;
}

/**
 * @brief give required size of mem to nodepool.
 * @param [in] pool   : css_nodepool_t*	节点管理池.
 * @param [in] size   : size_t	新内存大小.
 * @return  int
 * @retval  -1:失败;1:成功.

 * @date 2011/06/20
 **/
static int css_nodepool_request(css_nodepool_t *pool, size_t size) {
  /* alloc mem node */
  css_mem_node_t *pnode = css_new_mem_node(size);
  if (NULL == pnode) {
    return -1;
  }
  /* set nodepool */
  pnode->next = pool->mem_node_list;
  pool->mem_node_list = pnode;
  pool->p_curr_mem = pnode->p_mem;
  pool->p_curr_mem_size = pnode->mem_size;
  pool->p_pool_avail = pnode->p_mem;
  return 1;
}

/**
 * @brief	init nodepool
 * @retval   success: return 1; fail: return -1.

 * @date 2011/11/10
 **/
int css_nodepool_init(css_nodepool_t *pool, size_t size) {
  pool->mem_node_list = NULL;
  return css_nodepool_request(pool, size);
}

/**
 * @brief reset the nodepool to have only one memery node and reset the pool as never used

 * @date 2011/06/21
 **/
void css_nodepool_reset(css_nodepool_t *pool) {
  css_mem_node_t *keep_node = NULL;
  css_mem_node_t *node = NULL;
  keep_node = pool->mem_node_list;
  node = keep_node;
  /* free extra mem and only keep one mem node*/
  while (node->next != NULL) {
    keep_node = node->next;
    if (node->p_mem != NULL) {
      free(node->p_mem);
      node->p_mem = NULL;
    }
    free(node);
    node = keep_node;
  }
  keep_node->next = NULL;
  /* reset the nodepool */
  pool->mem_node_list = keep_node;
  pool->p_curr_mem = keep_node->p_mem;
  pool->p_curr_mem_size = keep_node->mem_size;
  pool->p_pool_avail = keep_node->p_mem;
}

/**
 * @brief destroy the nodepool

 * @date 2011/11/10
 **/
void css_nodepool_destroy(css_nodepool_t *pool) {
  css_mem_node_t *node = pool->mem_node_list;
  while (node != NULL) {
    css_mem_node_t *next = node->next;
    if (node->p_mem != NULL) {
      free(node->p_mem);
      node->p_mem = NULL;
    }
    free(node);
    node = next;
  }
  pool->mem_node_list = NULL;
  pool->p_curr_mem = NULL;
  pool->p_curr_mem_size = 0;
  pool->p_pool_avail = NULL;
}

/**
 * @brief	从nodepool获取size大小的内存.
 * @retval   成功返回内存地址,失败返回NULL.

 * @date 2011/11/10
 **/
void *css_get_from_nodepool(css_nodepool_t *pool, size_t size) {
  void *ret_mem = NULL;
  if (size > pool->p_curr_mem_size) {
    LOG(ERROR) << "required size > current mem pool size";
    return NULL;
  }
  if ((char *) (pool->p_pool_avail) + size
      <= (char *) (pool->p_curr_mem) + pool->p_curr_mem_size) {
    ret_mem = pool->p_pool_avail;
    pool->p_pool_avail = (char *) (pool->p_pool_avail) + size;
  } else {
    if (css_nodepool_request(pool, pool->p_curr_mem_size) < 0)
      return NULL;
    if ((char *) (pool->p_pool_avail) + size
        <= (char *) (pool->p_curr_mem) + pool->p_curr_mem_size) {
      ret_mem = pool->p_pool_avail;
      pool->p_pool_avail = (char *) pool->p_pool_avail + size;
    } else
      return NULL;
  }

  return ret_mem;
}
