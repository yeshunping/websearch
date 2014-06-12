// Copyright 2013 Easou Inc. All Rights Reserved.
// Author: lucus_xu@staff.easou.com (Liqiu Xu)

#include <stdio.h>
#include <string>
#include <vector>

#include "base/ns.h"
#include "base/flags.h"
#include "file/file.h"
#include "util/htmlparser/xpath/xpath_extractor.h"

DEFINE_string(xpath, "", "");
DEFINE_string(file, "", "file should be gb18030 encoding");

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  string html;
  CHECK(file::File::ReadFileToString(FLAGS_file, &html))<< "Fail to read file:" << FLAGS_file;

  util::XpathExtractor extractor;
  vector<string> result;
  extractor.ExtractorXpath(html, FLAGS_xpath, &result);
  for (size_t i = 0; i < result.size(); ++i) {
    printf("%s\n", result[i].c_str());
  }

  return 0;
}
