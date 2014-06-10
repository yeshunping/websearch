// Copyright (c) 2013, The base Authors. All rights reserved.
// Author: Ye Shunping <yeshunping@gmail.com>

#include <algorithm>

#include "base/benchmark.h"

#include "base/flags.h"

extern int base::nbenchmarks;
extern base::Benchmark* base::benchmarks[];

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, false);
    for (int i = 0; i < base::nbenchmarks; i++) {
        base::Benchmark* b = base::benchmarks[i];
        for (int j = b->threadlo; j <= b->threadhi; j++)
            for (int k = std::max(b->lo, 1); k <= std::max(b->hi, 1); k <<= 1)
                RunBench(b, j, k);
    }
}
