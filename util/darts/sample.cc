// Copyright 2014. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#include <iostream>
#include <string>
#include <vector>

#include "base/logging.h"
#include "base/es.h"
#include "util/darts/darts.h"

int main(int argc, char **argv) {
  vector<const char*> str;
  str.push_back("ALGOL");
  str.push_back("ANSI");
  str.push_back("ARCO");
  str.push_back("ARPA");
  str.push_back("ARPANET");
  str.push_back("ASCII");

  Darts::DoubleArray da;
  da.build(str.size(), &str[0]);

  Darts::DoubleArray::result_pair_type result_pair;
  Darts::DoubleArray::key_type key[1024] = "ALGOL";
  da.exactMatchSearch(key, result_pair);
  LOG(INFO)<< " " << result_pair.value << ":" << result_pair.length;
}
