// Copyright 2014. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef UTIL_PREFIX_TREE_TRIE_H_
#define UTIL_PREFIX_TREE_TRIE_H_

#include <string>
#include <deque>

#include "base/basictypes.h"
#include "util/prefix_tree/trie_node.h"

namespace util {
class Trie {
 public:
  Trie();
  ~Trie();

  void insert(const std::string& s);
  void remove(const std::string& s);

  bool has(const std::string& s) const;
  std::deque<std::string> search(const std::string& s) const;
  std::deque<std::string> search(const std::string& s, unsigned int cap) const;

 private:
  TrieNode* root;

  TrieNode* reach(const std::string& str) const;
  void search_recursively(const std::string& s,
                          TrieNode* node,
                          std::deque<std::string>* results ,
                          unsigned int cap) const;

  DISALLOW_COPY_AND_ASSIGN(Trie);
};
}  // namespace util
#endif  // UTIL_PREFIX_TREE_TRIE_H_
