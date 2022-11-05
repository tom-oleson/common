#!/bin/bash

CM_LIB_DIR=../
EXE=run_tests
clear
export LD_LIBRARY_PATH=${CM_LIB_DIR}:$LD_LIBRARY_PATH;gdb $PWD/$EXE
