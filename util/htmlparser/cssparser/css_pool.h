/**
 * @file css_pool.h

 * @date 2011/06/20
 * @version 1.0(create)
 * @brief css_pool用于:分配多个CSS结构的空间,存放多个解析好的CSS结构.
 *
 **/
#ifndef EASOU_CSS_POOL_H_
#define EASOU_CSS_POOL_H_

#include "css_parser.h"

#define MAX_CSS_NUM_IN_POOL 512
#define DEFAULT_CSS_NUM_IN_POOL	8		  /**< 默认分配空间的CSS结构数量  */

/**css的解析结果*/
typedef struct _css_pool_t
{
	css_t *css_array[MAX_CSS_NUM_IN_POOL]; /**< CSS数组指针 */
	short order[MAX_CSS_NUM_IN_POOL]; /**< 对应的CSS的序号，order数字越大优先级越高*/
	int alloc_css_num; /**< 已分配空间的CSS结构数量  */
	int used_css_num; /**< 已使用(即装有解析后的结构)的CSS数量*/
	hashmap_t *hm;
} css_pool_t;

/**
 * @brief 清空css_pool,使CSS结构回到未解析的状态.

 * @date 2011/06/21
 **/
void css_pool_clean(css_pool_t *css_pool);

/**
 * @brief	获取css_pool中CSS数组的数量.

 * @date 2011/06/21
 **/
int get_css_pool_array_size(css_pool_t *css_pool);

/**
 * @brief	销毁css_pool,回收已分配的空间.

 * @date 2011/06/20
 **/
void css_pool_destroy(css_pool_t *css_pool);

/**
 * @brief 初始化css_pool,为CSS结构分配空间.
 * @param [in/out] css_pool   : css_pool_t*	已分配空间的css_pool.
 * @param [in/out] max_css_page_size   : int	css_page的大小,根据这个值为CSS结构分配空间.
 * @param [in/out] css_num   : int	分配空间的CSS结构数量.
 * @return  int
 * @retval  -1:失败;1:成功.

 * @date 2011/06/20
 **/
int css_pool_init(css_pool_t *css_pool, int max_css_page_size, int css_num);

/**
 * @brief 将csspool中的CSS按order值从小到大排序.order值越大,优先级越高.

 * @date 2011/06/20
 **/
void css_pool_sort(css_pool_t *css_pool);

#endif /*EASOU_CSS_POOL_H_*/
