// Copyright 2014. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#include <string>
#include <vector>
#include <utility>

#include "util/prefix_tree/prefix_tree.h"
#include "base/flags.h"
#include "base/logging.h"
#include "base/es.h"
#include "thirdparty/gtest/gtest.h"

TEST(PrefixTreeUnittest, test1) {
  util::PrefixTree tree;
  tree.AddString("a");
  tree.AddString("yes");
  tree.AddString("zlib");
  tree.AddString("ab");
  tree.AddString("abd");
  tree.AddString("abcd");
  tree.AddString("abcde");
  tree.AddString("aaaa");

  EXPECT_TRUE(tree.Build());
  EXPECT_EQ(tree.RecordNumber(), 8UL);
  string key = "abcde";
  LOG(INFO) << "CommonMatch result:";
  vector<string> results;
  EXPECT_TRUE(tree.CommonMatch(key, &results));
  for (size_t i = 0; i < results.size(); ++i) {
    LOG(INFO) << results[i];
  }

  vector<const char*> ptr_results;
  EXPECT_TRUE(tree.CommonMatch(key, &ptr_results));
  for (size_t i = 0; i < ptr_results.size(); ++i) {
    LOG(INFO) << ptr_results[i];
  }

  EXPECT_TRUE(tree.ExactMatch(key));
  EXPECT_TRUE(tree.ExactMatchValue(key));

  string bad_key = "yesp";
  EXPECT_FALSE(tree.ExactMatch(bad_key));
}

TEST(PrefixTreeUnittest, chinese) {
  util::PrefixTree tree;
  tree.AddString("下载");
  tree.AddString("下载软件");
  tree.AddString("下载电影");
  tree.AddString("下载qq");
  tree.AddString("下载各1�7");

  EXPECT_TRUE(tree.Build());
  EXPECT_EQ(tree.RecordNumber(), 5UL);
  string key = "下载电影";
  LOG(INFO) << "CommonMatch result:";
  vector<string> results;
  EXPECT_TRUE(tree.CommonMatch(key, &results));
  for (size_t i = 0; i < results.size(); ++i) {
    LOG(INFO) << results[i];
  }

  EXPECT_TRUE(tree.ExactMatchValue("下载电影"));

  EXPECT_TRUE(tree.ExactMatchValue("下载软件"));
}


TEST(PrefixTreeUnittest, suffix_trie) {
  util::PrefixTree tree;
  tree.AddString("下载");
  tree.AddString("软件");
  tree.AddString("电影");
  tree.AddString("qq");
  tree.AddString("avi");

  EXPECT_TRUE(tree.Build());
  string key = "下载电影";
}

TEST(PrefixTreeUnittest, SuffixFirstCommonMatch) {
  {
    util::PrefixTree tree;
    tree.AddString("aabc");
    tree.AddString("bc");
    EXPECT_TRUE(tree.Build());
    string out;
    EXPECT_TRUE(tree.SuffixFirstCommonMatch("abcd", &out));
    LOG(INFO) << out;
  }
  {
    util::PrefixTree tree;
    tree.AddString("aabc");
    EXPECT_TRUE(tree.Build());
    string out;
    EXPECT_FALSE(tree.SuffixFirstCommonMatch("abcd", &out));
  }
}
