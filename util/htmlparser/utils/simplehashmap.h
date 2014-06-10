/**
 * @file utils/simplehash.h
 **/

#ifndef  __SIMPLEHASH_H_
#define  __SIMPLEHASH_H_

#include <unistd.h>
#include "nodepool.h"

struct hashmap_t;
/**< 简单hashmap */

#define DEFAULT_HASHMAP_SIZE 521		  /**< better to be prime number       */

/**
 * @brief 创建hashmap.
 * @return  hashmap_t* 成功返回hashmap指针,失败返回NULL.
 **/
hashmap_t *hashmap_create(int size);
hashmap_t *hashmap_create();

/**
 * @brief 摧毁hashmap.
 **/
void hashmap_destroy(hashmap_t *hm);

/**
 * @brief hashmap清空.
 **/
void hashmap_clean(hashmap_t *hm);

/**
 * @brief 向hashmap中存一个元素.
 *  不处理key值重复问题.
 **/
void hashmap_put(hashmap_t *hm, const char *key, void *data);

void hashmap_mod(hashmap_t *hm, const char *key, void *data);

/**
 * @brief 按键值从hashmap中获取元素.
 **/
void *hashmap_get(hashmap_t *hm, const char *key);

void *hashmap_get(hashmap_t *hm, const char *key, int key_len);

/**
 * @brief 准备迭代hashmap中的元素.
 **/
void hashmap_iter_begin(hashmap_t *hm);

/**
 * @brief hashmap迭代获取下一个元素.
 * XXX: 迭代过程线程不安全.
 * @retval   返回元素指针. 若返回NULL,表明元素已迭代完.
 **/
void *hashmap_iter_next(hashmap_t *hm);

/**
 * @brief 获取hashmap中已插入的元素数量.
 **/
int hashmap_get_element_num(hashmap_t *hm);

/**
 * @brief 根据键值计算其在hash桶中的序号.
 **/
int key_to_hash_bucket_index(const char *key, hashmap_t *hm);

int key_to_hash_bucket_index(const char *key, int key_len, hashmap_t *hm);

/**
 * @brief 在已求得所放hash桶序号的前提下, 插入对应元素.
 *  XXX: for advanced using.
 **/
void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, void *value);

void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, int key_len, void *value);

/**
 * @brief 修改hash表中的data字段
 * @author sue
 * @date 2013/04/09
 */
void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, void *value);

void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, int key_len, void *value);

/**
 * @brief 在已求得所放hash桶位置的前提下, 获取对应元素.
 *  XXX: for advanced using.
 **/
void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key);

void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key, int key_len);

/**
 * @brief 检查key是否在hashmap中, 若无则加入.
 * @return  bool 是否在hashmap中.
 **/
bool check_in_hashmap(hashmap_t *hm, const char *key, void *value);

#endif  //__TOOLS/SIMPLEHASH_H_
