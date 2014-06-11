/**
 * @file utils/simplehash.h
 **/

#ifndef  __SIMPLEHASH_H_
#define  __SIMPLEHASH_H_

#include <unistd.h>
#include "nodepool.h"

struct hashmap_t;

#define DEFAULT_HASHMAP_SIZE 521		  /**< better to be prime number       */

hashmap_t *hashmap_create(int size);
hashmap_t *hashmap_create();
void hashmap_destroy(hashmap_t *hm);
void hashmap_clean(hashmap_t *hm);
void hashmap_put(hashmap_t *hm, const char *key, void *data);

void hashmap_mod(hashmap_t *hm, const char *key, void *data);
void *hashmap_get(hashmap_t *hm, const char *key);

void *hashmap_get(hashmap_t *hm, const char *key, int key_len);
void hashmap_iter_begin(hashmap_t *hm);
void *hashmap_iter_next(hashmap_t *hm);
int hashmap_get_element_num(hashmap_t *hm);
int key_to_hash_bucket_index(const char *key, hashmap_t *hm);

int key_to_hash_bucket_index(const char *key, int key_len, hashmap_t *hm);
void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, void *value);
void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, int key_len,
                 void *value);
void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, void *value);
void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, int key_len,
                 void *value);
void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key);
void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key, int key_len);

bool check_in_hashmap(hashmap_t *hm, const char *key, void *value);

#endif  //__TOOLS/SIMPLEHASH_H_
