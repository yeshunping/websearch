// Copyright 2014. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef UTIL_PREFIX_TREE_TRIE_NODE_H_
#define UTIL_PREFIX_TREE_TRIE_NODE_H_

#include <map>
#include <deque>

#include "base/basictypes.h"

namespace util {

class TrieNode {
 public:
  TrieNode();
  TrieNode(unsigned int);
  ~TrieNode();

  TrieNode* get(char c) const;
  TrieNode* put(char c);
  std::deque<char> edges() const;
  bool has(char c) const ;
  unsigned int increment();
  unsigned int decrement();
  unsigned int get_count() const;

 private:
  std::map<char, TrieNode*> children;
  unsigned int count;

  DISALLOW_COPY_AND_ASSIGN(TrieNode);
};

typedef std::map<char, TrieNode*>::const_iterator TrieNodeIterator;
}  // namespace util
#endif  // UTIL_PREFIX_TREE_TRIE_NODE_H_
