// Copyright (c) 2013, The Toft Authors. All rights reserved.
// Author: Ye Shunping <yeshunping@gmail.com>

#include "base/hash.h"
#include "base/benchmark.h"

const std::string test_str = "qwertyuiopasdfghjklmnbvcz";

static void Fingerprint64(int n) {
    for (int i = 0; i < n; i++) {
        base::Fingerprint(test_str);
    }
}

static void Fingerprint32(int n) {
    for (int i = 0; i < n; i++) {
        base::Fingerprint32(test_str);
    }
}

static void JenkinsOneAtATimeHash(int n) {
    for (int i = 0; i < n; i++) {
        base::JenkinsOneAtATimeHash(test_str);
    }
}


BENCHMARK(Fingerprint64)->ThreadRange(1, NumCPUs());
BENCHMARK(Fingerprint32)->ThreadRange(1, NumCPUs());
BENCHMARK(JenkinsOneAtATimeHash)->ThreadRange(1, NumCPUs());
