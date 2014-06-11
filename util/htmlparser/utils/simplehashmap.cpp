/**
 * @file utils/hashmap.cpp
 *
 **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "simplehashmap.h"

typedef struct _hashmap_entry_t {
  const char *key;
  int key_len;
  void *data;
  struct _hashmap_entry_t *next;
} hashmap_entry_t;

typedef struct _hashmap_iteractor_t {
  int i_bucket;
  hashmap_entry_t *curr_entry;
} hashmap_iterator_t;

struct hashmap_t {
  int size;
  int ele_num;
  nodepool_t entry_nodepool;
  hashmap_iterator_t iter;
  hashmap_entry_t *bucket[0];
};

void hashmap_destroy(hashmap_t *hm) {
  if (hm != NULL) {
    nodepool_destroy(&hm->entry_nodepool);
    free(hm);
  }
}

hashmap_t *hashmap_create(int size) {
  hashmap_t *hm = (hashmap_t *) calloc(
      1, sizeof(hashmap_t) + sizeof(hashmap_entry_t *) * size);
  if (hm == NULL) {
    goto FAIL;
  }

  hm->size = size;
  if (0 == nodepool_init(&hm->entry_nodepool, sizeof(hashmap_entry_t))) {
    goto FAIL;
  }

  return hm;
  FAIL: hashmap_destroy(hm);

  return NULL;
}

hashmap_t *hashmap_create() {
  return hashmap_create(DEFAULT_HASHMAP_SIZE);
}

void hashmap_clean(hashmap_t *hm) {
  if (hm->ele_num > 0) {
    hm->ele_num = 0;

    for (int i = 0; i < hm->size; i++) {
      hm->bucket[i] = NULL;
    }

    nodepool_reset(&hm->entry_nodepool);
  }
}

static unsigned int SDBMHash(const char *str) {
  unsigned int hash = 0;

  while (*str) {
    hash = (*str++) + 65599 * hash;
  }

  return (hash & 0x7FFFFFFF);
}

static unsigned int SDBMHash(const char *str, int len) {
  unsigned int hash = 0;

  while (*str && len-- > 0) {
    hash = (*str++) + 65599 * hash;
  }

  return (hash & 0x7FFFFFFF);
}

#define	NAME_TO_HASHVALUE(name, mapsize)	(SDBMHash(name) % (mapsize))

int key_to_hash_bucket_index(const char *key, hashmap_t *hm) {
  return NAME_TO_HASHVALUE(key, hm->size);
}

int key_to_hash_bucket_index(const char *key, int key_len, hashmap_t *hm) {
  return SDBMHash(key, key_len) % (hm->size);
}

void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, void *value) {
  hashmap_entry_t *e = (hashmap_entry_t *) nodepool_get(&hm->entry_nodepool);

  if (e) {
    e->key = key;
    e->data = value;
    e->next = hm->bucket[i_bucket];
    hm->bucket[i_bucket] = e;
    hm->ele_num++;
  }
}

void hashmap_put(hashmap_t *hm, int i_bucket, const char *key, int key_len,
                 void *value) {
  hashmap_entry_t *e = (hashmap_entry_t *) nodepool_get(&hm->entry_nodepool);

  if (e) {
    e->key = key;
    e->key_len = key_len;
    e->data = value;
    e->next = hm->bucket[i_bucket];
    hm->bucket[i_bucket] = e;
    hm->ele_num++;
  }
}

void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, void *value) {
  for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next) {
    if (strcmp(e->key, key) == 0) {
      e->data = value;
      return;
    }
  }
}

void hashmap_mod(hashmap_t *hm, int i_bucket, const char *key, int key_len,
                 void *value) {
  for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next) {
    if (key_len == e->key_len && strncmp(e->key, key, key_len) == 0) {
      e->data = value;
      return;
    }
  }
}

void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key) {
  for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next) {
    if (strcmp(e->key, key) == 0) {
      return e->data;
    }
  }

  return NULL;
}

void *hashmap_get(hashmap_t *hm, int i_bucket, const char *key, int key_len) {
  for (hashmap_entry_t *e = hm->bucket[i_bucket]; e != NULL; e = e->next) {
    if (key_len == e->key_len && strncmp(e->key, key, key_len) == 0) {
      return e->data;
    }
  }

  return NULL;
}

void hashmap_put(hashmap_t *hm, const char *key, void *value) {
  unsigned short i_bucket = NAME_TO_HASHVALUE(key,hm->size);

  hashmap_put(hm, i_bucket, key, value);
}

void hashmap_mod(hashmap_t *hm, const char *key, void *value) {
  unsigned short i_bucket = NAME_TO_HASHVALUE(key,hm->size);

  hashmap_mod(hm, i_bucket, key, value);
}

void *hashmap_get(hashmap_t *hm, const char *key) {
  unsigned short i_bucket = NAME_TO_HASHVALUE(key, hm->size);

  return hashmap_get(hm, i_bucket, key);
}

void *hashmap_get(hashmap_t *hm, const char *key, int key_len) {
  unsigned short i_bucket = SDBMHash(key, key_len) % (hm->size);

  return hashmap_get(hm, i_bucket, key, key_len);
}

bool check_in_hashmap(hashmap_t *hm, const char *key, void *value) {
  int i_bucket = key_to_hash_bucket_index(key, hm);
  void *data = hashmap_get(hm, i_bucket, key);
  if (data == NULL) {
    hashmap_put(hm, i_bucket, key, value);
    return false;
  } else {
    return true;
  }
}

void hashmap_iter_begin(hashmap_t *hm) {
  hm->iter.i_bucket = -1;
  hm->iter.curr_entry = NULL;
}

void *hashmap_iter_next(hashmap_t *hm) {
  if (hm->iter.curr_entry == NULL || hm->iter.curr_entry->next == NULL) {
    for (int i = hm->iter.i_bucket + 1; i < hm->size; i++) {
      if (hm->bucket[i]) {
        hm->iter.i_bucket = i;
        hm->iter.curr_entry = hm->bucket[i];
        return hm->iter.curr_entry->data;
      }
    }
    return NULL;
  } else {
    hm->iter.curr_entry = hm->iter.curr_entry->next;
    return hm->iter.curr_entry->data;
  }
}

int hashmap_get_element_num(hashmap_t *hm) {
  return hm->ele_num;
}

