/**
 * @file css_parser.h

 * @date 2011/06/20
 * @version 1.0(create)
 * @brief CSS节点内存分配管理.
 *
 **/

#ifndef EASOU_CSS_NODEPOOL_H_
#define EASOU_CSS_NODEPOOL_H_

#include "css_dtd.h"
/**
 * @brief	init nodepool
 * @retval   success: return 1; fail: return -1.

 * @date 2011/06/20
 **/
int css_nodepool_init(css_nodepool_t *pool, size_t size);

/**
 * @brief reset the nodepool to have only one memery node and reset the pool as never used

 * @date 2011/06/20
 **/
void css_nodepool_reset(css_nodepool_t *pool);

/**
 * @brief destroy the nodepool

 * @date 2011/06/20
 **/
void css_nodepool_destroy(css_nodepool_t *pool);

/**
 * @brief	从nodepool获取size大小的内存.
 * @retval   成功返回内存地址,失败返回NULL.
 * @see

 * @date 2011/06/20
 **/
void *css_get_from_nodepool(css_nodepool_t *pool, size_t size);

#endif /*EASOU_CSS_NODEPOOL_H_*/
