/*
  Darts -- Double-ARray Trie System

  $Id: darts.cpp 1674 2008-03-22 11:21:34Z taku $;

  Copyright(C) 2001-2007 Taku Kudo <taku@chasen.org>
  All rights reserved.
*/
#include <string>

#include "base/flags.h"
#include "base/logging.h"
#include "base/es.h"
#include "util/darts/darts.h"
#include "file/file.h"
#include "file/simple_line_reader.h"
#include "base/stl_util-inl.h"

DEFINE_bool(common_search, true, "if not, use exact search");
//DEFINE_string(index_file, "util/darts/testdata/linux.words.idx", "");
DEFINE_string(key, "", "key to search");
DEFINE_string(word_file, "util/darts/testdata/linux.words", "");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  Darts::DoubleArray da;

  vector<string> lines;
  file::SimpleLineReader reader(FLAGS_word_file);
  reader.ReadLines(&lines);
  vector<const char *> keys;
  for (size_t i = 0; i < lines.size(); ++i) {
    keys.push_back(::strdup(lines[i].c_str()));
  }
  //  build
  CHECK_EQ(da.build(keys.size(), &keys[0], 0, 0), 0);

  //  if use index_file, we simply need to open it.
//  if (da.open(FLAGS_index_file.c_str())) {
//    LOG(ERROR) << "Error: cannot open " << FLAGS_index_file;
//    return -1;
//  }

  Darts::DoubleArray::result_pair_type  result_pair[1024];
  Darts::DoubleArray::key_type          inner_key[1024];
  strcpy(inner_key, FLAGS_key.c_str());

  CHECK(!FLAGS_key.empty()) << "key must be set";

  if (FLAGS_common_search) {
    size_t num = da.commonPrefixSearch(inner_key, result_pair,
        sizeof(result_pair));
    if (num == 0) {
      LOG(INFO) << inner_key << ": not found" << std::endl;
    } else {
      LOG(INFO) << inner_key << ": found, num=" << num << " ";
      for (size_t i = 0; i < num; ++i) {
        int index = result_pair[i].value;
        LOG(INFO) << "index: " << index << "\tlength:"
            << result_pair[i].length << "\tword:" << keys[index];
      }
    }
  } else {
    Darts::DoubleArray::result_pair_type  result_pair;
    da.exactMatchSearch(inner_key, result_pair);
    int index = result_pair.value;
    LOG(INFO) << "index: " << index << "\tlength:"
        << result_pair.length << "\tword:" << keys[index];
  }

  STLDeleteElements(&keys);
  return 0;
}
