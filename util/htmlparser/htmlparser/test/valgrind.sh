#!/bin/bash

ulimit -c unlimited
export LD_LIBRARY_PATH=../../../publish_1.0.1/log/lib/log4cpp
valgrind --tool=memcheck --leak-check=full --log-file=vlog.log --trace-children=yes ./html_parser_test $1 $2 $3 $4
