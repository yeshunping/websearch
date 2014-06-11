// Copyright 2013. All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#include <string>
#include <vector>

#include "base/logging.h"
#include "base/string_util.h"
#include "base/ns.h"
#include "file/file.h"
#include "thirdparty/gtest/gtest.h"

TEST(File, simple) {
  string name = "/tmp/test.txt";
  string data("test data for testing");

  file::File::WriteStringToFile(data, name);
  string out;
  file::File::ReadFileToString(name, &out);
  EXPECT_EQ(data, out);
}

TEST(File, WriteLinesToFile) {
  string name = "/tmp/test_WriteLinesToFile.txt";
  vector<string> vec;
  for (int i = 0; i < 10; ++i) {
    vec.push_back(StringPrintf("line %d", i));
  }
  file::File::WriteLinesToFile(vec, name);
  vector<string> out;
  file::File::ReadFileToLines(name, &out);
  EXPECT_EQ(vec, out);
}

TEST(File, FileSize) {
  string name = "/tmp/test_FileSize.txt";
  string content = "hello";
  file::File::WriteStringToFile(content, name);
  size_t size = 0;
  EXPECT_TRUE(file::File::FileSize(name, &size));
  EXPECT_EQ(size, 5UL);
}
