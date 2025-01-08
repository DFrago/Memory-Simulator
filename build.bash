#/bin/bash

echo "building test"
gcc -o main main.c ApplicationMemory.c DRAM.c DRAM_Cache.c VirtualMemory.c Performance.c -Wall
compResult=$?
if [ $compResult -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

echo "running test"
./main

echo "results"
echo ""
echo main
tail -11 memory_trace_3_main.txt

