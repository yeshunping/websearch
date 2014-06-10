/**
 * @file utils/hashmap.cpp
 *
 **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "simplehashmap.h"

/**
 * @brief hashmap中的节点.
 */
typedef struct _hashmap_entry_t
{
	const char *key; /**< 节点对应的键值 */
	int key_len;
	void *data; /**< 节点对应的数据 */
	struct _hashmap_entry_t *next; /**< 用于hash表拉链 */
} hashmap_entry_t;

/**
 * @brief hashmap迭代结构.
 */
typedef struct _hashmap_iteractor_t
{
	int i_bucket; /**< 当前hash桶位置 */
	hashmap_entry_t *curr_entry; /**< 当前遍历到的节点 */
} hashmap_iterator_t;

/**
 * @brief hashmap.
 */
struct hashmap_t
{
	int size; /**< 桶个数 */
	int ele_num; /**< 存放的元素个数 */
	nodepool_t entry_nodepool; /**< 用于分配节点 */
	hashmap_iterator_t iter; /**< 迭代器 */
	hashmap_entry_t *bucket[0]; /**< HASH桶 */
};

/**
 * @brief 摧毁hashmap.
 **/
void hashmap_destroy(hashmap_t *hm)
{
	if (hm != NULL)
	{
		nodepool_destroy(&hm->entry_nodepool);
		free(hm);
	}
}

/**
 * @brief 创建hashmap.
 * @return  hashmap_t* 成功返回hashmap指针,失败返回NULL.
 **/
hashmap_t *hashmap_create(int size)
{
	hashmap_t *hm = (hashmap_t *) calloc(1, sizeof(hashmap_t) + sizeof(hashmap_entry_t *) * size);
	if (hm == NULL)
	{
		goto FAIL;
	}

	hm->size = size;
	if (0 == nodepool_init(&hm->entry_nodepool, sizeof(hashmap_entry_t)))
	{
		goto FAIL;
	}

	return hm;
	FAIL: hashmap_destroy(hm);

	return NULL;
}

hashmap_t *hashmap_create()
{
	return hashmap_create(DEFAULT_HASHMAP_SIZE);
}

/**
 * @brief hashmap清空.
 **/
void hashmap_clean(hashmap_t *hm)
{
	if (hm->ele_num > 0)
	{
		hm->ele_num = 0;

		for (int i = 0; i < hm->size; i++)
		{
			hm->bucket[i] = NULL;
		}

		nodepool_reset(&hm->entry_nodepool);
	}
}

static unsigned int SDBMHash(const char *str)
{
	unsigned int hash = 0;

	while (*str)
	{
		hash = (*str++) + 65599 * hash;
	}

	return (hash & 0x7FFFFFFF);
}

static unsigned int SDBMHash(const char *str, int len)
{
	unsigned int hash = 0;

	while (*str && len-- > 0)
	{
		hash = (*str++) + 65599 * hash;
	}

	return (hash & 0x7FFFFFFF);
}

#define	NAME_TO_HASHVALUE(name, mapsize)	(SDBMHash(name) % (mapsize))

/**
 * @brief 根据键值计算其在hash桶中的序号.
 **/
int key_to_hash_bucket_index(const char *key, hashmap_t *hm)
{
	return NAME_TO_HASHVALUE(key, hm->size);
}

int key_to_hash_bucket_index(const char *key, int key_len, hashmap_t *hm)
{
	return SDBMHash(key, key_len) % (hm->size);
}

/**
 * @brief 在已求得所放hash桶序号的前提下, 插入对应元素.
 **/
void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, void *value)
{
	hashmap_entry_t *e = (hashmap_entry_t *) nodepool_get(&hm->entry_nodepool);

	if (e)
	{
		e->key = key;
		e->data = value;
		e->next = hm->bucket[i_bucket];
		hm->bucket[i_bucket] = e;
		hm->ele_num++;
	}
}

void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, int key_len, void *value)
{
	hashmap_entry_t *e = (hashmap_entry_t *) nodepool_get(&hm->entry_nodepool);

	if (e)
	{
		e->key = key;
		e->key_len = key_len;
		e->data = value;
		e->next = hm->bucket[i_bucket];
		hm->bucket[i_bucket] = e;
		hm->ele_num++;
	}
}

void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, void *value)
{
	for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next)
	{
		if (strcmp(e->key, key) == 0)
		{
			e->data = value;
			return;
		}
	}
}

void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, int key_len, void *value)
{
	for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next)
	{
		if (key_len == e->key_len && strncmp(e->key, key, key_len) == 0)
		{
			e->data = value;
			return;
		}
	}
}

/**
 * @brief 在已求得所放hash桶位置的前提下, 获取对应元素.
 **/
void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key)
{
	for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next)
	{
		if (strcmp(e->key, key) == 0)
		{
			return e->data;
		}
	}

	return NULL;
}

void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key, int key_len)
{
	for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next)
	{
		if (key_len == e->key_len && strncmp(e->key, key, key_len) == 0)
		{
			return e->data;
		}
	}

	return NULL;
}

/**
 * @brief 向hashmap中存一个元素.
 *  不处理key值重复问题.
 **/
void hashmap_put(hashmap_t *hm, const char *key, void *value)
{
	unsigned short i_bucket = NAME_TO_HASHVALUE(key,hm->size);

	hashmap_put(hm, i_bucket, key, value);
}

void hashmap_mod(hashmap_t *hm, const char *key, void *value)
{
	unsigned short i_bucket = NAME_TO_HASHVALUE(key,hm->size);

	hashmap_mod(hm, i_bucket, key, value);
}

/**
 * @brief 按键值从hashmap中获取元素.
 **/
void *hashmap_get(hashmap_t *hm, const char *key)
{
	unsigned short i_bucket = NAME_TO_HASHVALUE(key, hm->size);

	return hashmap_get(hm, i_bucket, key);
}

void *hashmap_get(hashmap_t *hm, const char *key, int key_len)
{
	unsigned short i_bucket = SDBMHash(key, key_len) % (hm->size);

	return hashmap_get(hm, i_bucket, key, key_len);
}

/**
 * @brief 检查key是否在hashmap中, 若无则加入.
 * @return  bool 是否在hashmap中.
 **/
bool check_in_hashmap(hashmap_t *hm, const char *key, void *value)
{
	int i_bucket = key_to_hash_bucket_index(key, hm);
	void *data = hashmap_get(hm, i_bucket, key);
	if (data == NULL)
	{
		hashmap_put(hm, i_bucket, key, value);
		return false;
	}
	else
	{
		return true;
	}
}

/**
 * @brief 准备迭代hashmap中的元素.
 **/
void hashmap_iter_begin(hashmap_t *hm)
{
	hm->iter.i_bucket = -1;
	hm->iter.curr_entry = NULL;
}

/**
 * @brief hashmap迭代获取下一个元素.
 * @retval   返回元素指针. 若返回NULL,表明元素已迭代完.
 **/
void *hashmap_iter_next(hashmap_t *hm)
{
	if (hm->iter.curr_entry == NULL || hm->iter.curr_entry->next == NULL)
	{
		for (int i = hm->iter.i_bucket + 1; i < hm->size; i++)
		{
			if (hm->bucket[i])
			{
				hm->iter.i_bucket = i;
				hm->iter.curr_entry = hm->bucket[i];
				return hm->iter.curr_entry->data;
			}
		}
		return NULL;
	}
	else
	{
		hm->iter.curr_entry = hm->iter.curr_entry->next;
		return hm->iter.curr_entry->data;
	}
}

/**
 * @brief 获取hashmap中已插入的元素数量.
 **/
int hashmap_get_element_num(hashmap_t *hm)
{
	return hm->ele_num;
}

