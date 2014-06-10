#!/bin/bash

ulimit -c unlimited
export LD_LIBRARY_PATH=../../depends/cssserver/lib32
./vhtml_parser_test $1 $2 $3 $4 $5 $6 $7 $8
