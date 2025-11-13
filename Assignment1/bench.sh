#!/bin/bash

# Usage:
#   ./run_bench.sh ./q8 1000 10000 100000 1000000
#
# The program should print:
#   <some value>
#   <time in microseconds>

PROGRAM=$1
shift
FLAGS=("$@")

RUNS=10

echo "Program: $PROGRAM"
echo "Number of runs per flag: $RUNS"
echo "-------------------------------------------"

for flag in "${FLAGS[@]}"; do
    min_time=999999999999   # large number

    for ((i=1; i<=RUNS; i++)); do
        # Capture only the second line (microseconds)
        time_us=$($PROGRAM "$flag" | sed -n '2p')

        # check numeric
        if [[ "$time_us" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            # Compare as floating point
            comp=$(awk -v a="$time_us" -v b="$min_time" 'BEGIN {print (a < b)}')
            if [[ "$comp" -eq 1 ]]; then
                min_time=$time_us
            fi
        else
            echo "Warning: non-numeric time detected: $time_us"
        fi
    done

    echo "Flag $flag → Lowest time: $min_time µs"
done

echo "-------------------------------------------"
