#!/bin/bash

ulimit -c unlimited
export LD_LIBRARY_PATH=../../../publish_1.0.1/log/lib/log4cpp
./html_parser_test $1 $2 $3 $4
