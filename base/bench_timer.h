// Copyright 2013 Easou Inc. All Rights Reserved.
// Author: shunping_ye@staff.easou.com (Shunping Ye)


#include <stdio.h>

#include <iostream>
#include <string>

#include "base/time.h"

namespace base {
class BenchTimer {
 public:
  BenchTimer(const std::string& test_obj, int repeat_times)
    : test_obj_(test_obj), repeat_times_(repeat_times) {
    begin_time_ = base::GetTimeInMs();
  }

  ~BenchTimer() {
    int64 end_time = base::GetTimeInMs();
    int64 used_time = end_time - begin_time_;
    if (used_time == 0) {
      LOG(INFO) << "used_time == 0 ms, set it to 1";
      used_time = 1;
    }
    std::cout << "benchmark end for [ " << test_obj_
              << " ] ,time used:" << used_time << "(Ms)"
              << ", repeat_times:" << repeat_times_
              << ", performance:" << repeat_times_ * 1000 / used_time << " (times/sec)\n\n";
    fflush(stdout);
  }
 private:
  int64 begin_time_;
  std::string test_obj_;
  int repeat_times_;
};
}  // namespace base
