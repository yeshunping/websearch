// Copyright 2013. All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#include "file/simple_line_reader.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "base/es.h"

namespace file {
SimpleLineReader::SimpleLineReader(const string& filename,
                                   bool skip_empty_line,
                                   const string& skip_comment_prefix) {
  fp_ = fopen(filename.c_str(), "r");
  CHECK(fp_ != NULL)<< "fail to open file:" << filename;
  skip_empty_line_ = skip_empty_line;
  skip_comment_prefix_ = skip_comment_prefix;
}

SimpleLineReader::SimpleLineReader(const string& filename,
                                   bool skip_empty_line) {
  fp_ = fopen(filename.c_str(), "r");
  CHECK(fp_ != NULL)<< "fail to open file:" << filename;
  skip_empty_line_ = skip_empty_line;
}

SimpleLineReader::SimpleLineReader(const string& filename) {
  fp_ = fopen(filename.c_str(), "r");
  CHECK(fp_ != NULL)<< "fail to open file:" << filename;
  skip_empty_line_ = true;
}

SimpleLineReader::~SimpleLineReader() {
  fclose(fp_);
}

bool SimpleLineReader::ProcessLines(
    base::ResultCallback1<bool, const string&>* callback) {
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, fp_)) != -1) {
    if (read == 0 || (read == 1 && *line == '\n')) {
      if (!skip_empty_line_) {
        if (!callback->Run(string(line))) {
          break;
        }
      }
    } else {
      string str(line);
      str.resize(str.length() - 1);
      if (!callback->Run(str)) {
        break;
      }
    }
  }
  free(line);
  return true;
}

bool SimpleLineReader::PushLine(const string& line) {
  if (line.empty() && skip_empty_line_) {
    return true;
  }
  if (!skip_comment_prefix_.empty() &&
      StartsWithASCII(line, skip_comment_prefix_, false)) {
    return true;
  }
  lines_.push_back(line);
  return true;
}

bool SimpleLineReader::ReadLines(vector<string>* out) {
  base::ResultCallback1<bool, const string&>* callback =
      base::NewPermanentCallback(this, &SimpleLineReader::PushLine);
  ProcessLines(callback);
  delete callback;
  out->swap(lines_);
  return true;
}
}
