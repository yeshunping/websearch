// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#include "base/logging.h"
#include "base/es.h"
#include "base/time.h"
#include "base/flags.h"
#include "file/simple_line_reader.h"
#include "web_util/url_parser/url_parser.h"

DEFINE_int32(repeat_times, 5000, "");

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  int64 begin_time= base::GetTimeInMs();
  string fname = "web_util/url_parser/testdata/hosts.txt";
  vector<string> hosts;
  file::SimpleLineReader reader(fname);
  reader.ReadLines(&hosts);
  string domain;
  for (int i = 0; i < FLAGS_repeat_times; ++i) {
    for (size_t j = 0; j < hosts.size(); ++j) {
      domain.clear();
      web_util::url::GetDomainFromHost(hosts[j], &domain);
    }
  }
  int64 end_time= base::GetTimeInMs();
  int total_time = end_time - begin_time;
  int performance = FLAGS_repeat_times * hosts.size() * 1000 / total_time;
  LOG(INFO) << "total times:" << total_time
            << ", call times:" << FLAGS_repeat_times * hosts.size()
            << ", performance:" << performance << " /second";
  return 0;
}
