// Copyright 2013. All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#include "file/simple_line_reader.h"
#include <string>

#include "base/logging.h"
#include "base/ns.h"
#include "file/file.h"
#include "thirdparty/gtest/gtest.h"

TEST(File, simple) {
  LOG(INFO) << "start testing for simple_line_reader";

  string name = "/tmp/test.txt";
  string data("line 1\n");
  data.append("line 2\n");
  data.append("line 3\n");
  data.append("line 4\n");
  file::File::WriteStringToFile(data, name);
  file::SimpleLineReader reader(name, true);
  vector<string> out;
  reader.ReadLines(&out);
  EXPECT_EQ(4UL, out.size());
  for (size_t i = 0; i < out.size(); ++i) {
    LOG(INFO) << out[i];
  }
  LOG(INFO) << "test end...";
}
