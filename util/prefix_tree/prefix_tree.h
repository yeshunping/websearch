// Copyright 2014. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef UTIL_PREFIX_TREE_PREFIX_TREE_H_
#define UTIL_PREFIX_TREE_PREFIX_TREE_H_

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <utility>

#include "base/basictypes.h"
#include "util/darts/darts.h"
#include "base/logging.h"
#include "base/es.h"  // NOLINT

namespace util {
class PrefixTree {
 public:
  PrefixTree() {}
  ~PrefixTree() {}

  void AddString(const std::string& str);

  bool Build();
  //  if key is abcd, "ab", "abc", "abcd" will match
  bool CommonMatch(const std::string& key,
                   std::vector<std::string>* result) const;
  bool SuffixFirstCommonMatch(const std::string& key, std::string* out) const;
  bool SuffixCommonMatchAll(const std::string& key,
                            std::vector<std::string>* result) const;

  //  will return const char* , which is hold by PrefixTree
  //  DO NOT free them ,PrefixTree will free them itself
  bool CommonMatch(const std::string& key,
                   std::vector<const char*>* result);

  //  Return matched keys and their values
  bool CommonMatchKeyValue(const std::string& key,
                           std::vector<std::string*>* results);
  //  only whole word match will return key.
  //  if key is "abcd", only if "abcd" exsit in dict
  //  will this function return true
  bool ExactMatch(const std::string& key);
  bool ExactMatchValue(const std::string& key);
  bool ExactMatchKeyValue(const std::string& key, std::string* result);

  uint32 RecordNumber();
 private:
  Darts::DoubleArray double_array_;
  //  to keep all data is sorted
  std::vector<std::string> data_;
  std::vector<const char*> keys_;
DISALLOW_COPY_AND_ASSIGN(PrefixTree);
};
}

#endif  // UTIL_PREFIX_TREE_PREFIX_TREE_H_
