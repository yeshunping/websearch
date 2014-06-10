// Copyright 2014. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#include "util/prefix_tree/trie_node.h"

#include "base/es.h"

namespace util {
TrieNode::TrieNode() {
  count = 0;
}

TrieNode::TrieNode(unsigned int num) {
  count = num;
}

TrieNode::~TrieNode() {
  for (TrieNodeIterator it = children.begin(); it != children.end(); ++it) {
    delete it->second;
  }
}

TrieNode* TrieNode::get(char c) const {
  map<char, TrieNode*>::const_iterator it = children.find(c);
  if (it != children.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

TrieNode* TrieNode::put(char c) {
  map<char, TrieNode*>::iterator it = children.find(c);
  if (it == children.end()) {
    TrieNode* node = new TrieNode();
    children.insert(make_pair(c, node));
    return node;
  } else {
    return it->second;
  }
}

deque<char> TrieNode::edges() const {
  deque<char> keys;
  for (TrieNodeIterator it = children.begin(); it != children.end(); ++it) {
    keys.push_back(it->first);
  }

  return keys;
}

bool TrieNode::has(char c) const {
  return children.count(c) > 0;
}

unsigned int TrieNode::increment() {
  return ++count;
}

unsigned int TrieNode::decrement() {
  return count == 0 ? 0 : --count;
}

unsigned int TrieNode::get_count() const {
  return count;
}
}
