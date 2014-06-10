// Copyright 2013. All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#ifndef FILE_SIMPLE_LINE_READER_H_
#define FILE_SIMPLE_LINE_READER_H_

#include <stdio.h>
#include <vector>
#include <string>

#include "base/basictypes.h"
#include "base/callback.h"

namespace file {

class SimpleLineReader {
 public:
  SimpleLineReader(const std::string& filename,
                   bool skip_empty_line,
                   const std::string& skip_comment_prefix);
  SimpleLineReader(const std::string& filename,
                   bool skip_empty_line);
  explicit SimpleLineReader(const std::string& filename);
  ~SimpleLineReader();

  bool ReadLines(std::vector<std::string>* out);
  bool ProcessLines(base::ResultCallback1<bool, const std::string&>* callback);

 private:
  bool PushLine(const std::string& line);

  FILE* fp_;
  bool skip_empty_line_;
  std::string skip_comment_prefix_;
  std::vector<std::string> lines_;
  DISALLOW_COPY_AND_ASSIGN(SimpleLineReader);
};
}
#endif  // FILE_SIMPLE_LINE_READER_H_
