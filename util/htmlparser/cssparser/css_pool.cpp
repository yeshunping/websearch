/**
 * @file css_pool.cpp

 * @date 2011/07/15
 * @brief css_pool用于:分配多个CSS结构的空间,存放多个解析好的CSS结构.
 *  
 **/

#include <string.h>
#include "css_parser.h"
#include "css_pool.h"

/**
 * @brief 清空css_pool,使CSS结构回到未解析的状态.
 **/
void css_pool_clean(css_pool_t *css_pool)
{
	for (int i = 0; i < css_pool->used_css_num; i++)
	{
		css_clean(css_pool->css_array[i]);
	}
	hashmap_clean(css_pool->hm);
	css_pool->used_css_num = 0;
}

/**
 * @brief	获取css_pool中CSS数组的数量.
 **/
int get_css_pool_array_size(css_pool_t *css_pool)
{
	return sizeof(css_pool->css_array) / sizeof(css_t *);
}

/**
 * @brief	销毁css_pool,回收已分配的空间.
 **/
void css_pool_destroy(css_pool_t *css_pool)
{
	int array_size = get_css_pool_array_size(css_pool);

	for (int i = 0; i < array_size; i++)
	{
		if (css_pool->css_array[i])
		{
			css_destroy(css_pool->css_array[i]);
			css_pool->css_array[i] = NULL;
		}
	}
	if (css_pool->hm)
	{
		hashmap_destroy(css_pool->hm);
		css_pool->hm = NULL;
	}
	css_pool->alloc_css_num = 0;
	css_pool->used_css_num = 0;
}

/**
 * @brief 初始化css_pool,为CSS结构分配空间.
 * @param [in/out] css_pool   : css_pool_t*	已分配空间的css_pool.
 * @param [in/out] max_css_page_size   : int	css_page的大小,根据这个值为CSS结构分配空间.
 * @param [in/out] css_num   : int	分配空间的CSS结构数量.
 * @return  int 
 * @retval  -1:失败;1:成功.

 * @date 2011/06/21
 **/
int css_pool_init(css_pool_t *css_pool, int max_css_page_size, int css_num)
{
	if (css_num > MAX_CSS_NUM_IN_POOL)
	{
		Fatal((char*) "%s: css_num (%d) > MAX_CSS_NUM_IN_POOL (%d),  and is automatically ajusted to %d.", __FUNCTION__, css_num,
				MAX_CSS_NUM_IN_POOL, MAX_CSS_NUM_IN_POOL);
		css_num = MAX_CSS_NUM_IN_POOL;
	}
	int this_size = max_css_page_size;
	int this_num = css_num;
	// default parameter
	if (max_css_page_size <= 0)
	{
		this_size = CSS_DEFAULT_TEXT_SIZE;
	}
	if (css_num <= 0)
	{
		this_num = DEFAULT_CSS_NUM_IN_POOL;
	}
	// init pointer array
	memset(css_pool->css_array, 0, sizeof(css_pool->css_array));

	// alloc memory
	for (int i = 0; i < this_num; i++)
	{
		css_pool->css_array[i] = css_create(this_size);
		if (css_pool->css_array[i] == NULL)
		{
			goto FAIL;
		}
	}
	css_pool->hm = hashmap_create();
	if (css_pool->hm == NULL)
		goto FAIL;

	css_pool->alloc_css_num = this_num;
	css_pool->used_css_num = 0;
	return 1;
	FAIL: css_pool_destroy(css_pool);
	return -1;
}

/**
 * @brief 交换csspool中两个CSS的位置.
 **/
static void csspool_swap(css_pool_t *csspool, int i, int j)
{
	css_t *tmp_css = csspool->css_array[i];
	short tmp_order = csspool->order[i];
	csspool->css_array[i] = csspool->css_array[j];
	csspool->order[i] = csspool->order[j];
	csspool->css_array[j] = tmp_css;
	csspool->order[j] = tmp_order;
}

/**
 * @brief 将csspool中的CSS按order值从小到大排序.order值越大,优先级越高.
 **/
void css_pool_sort(css_pool_t *csspool)
{
	/**
	 * 基本有序时，使用插入排序.且已有序情况下不需要交换.
	 */
	for (int i = 1; i < csspool->used_css_num; ++i)
	{
		for (int j = i - 1; j >= 0; --j)
		{
			if (csspool->order[j] > csspool->order[j + 1])
			{
				csspool_swap(csspool, j, j + 1);
			}
			else
			{
				break;
			}
		}
	}
}
