#!/usr/bin/bash

$PROGRAM_NAME=$1

g++ `pkg-config opencv --cflags` $PROGRAM_NAME.cpp -o $PROGRAM_NAME.o `pkg-config opencv --libs` -pg
if [[ $? -ne 0 ]]; then
    echo "Compilation failed"
    exit 1
fi

./$PROGRAM_NAME.o

gprof ./$PROGRAM_NAME.o
