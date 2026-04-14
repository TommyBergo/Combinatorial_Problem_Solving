#!/bin/bash

# Usage: ./run_all.sh <solver> <input_dir> <output_dir>
SOLVER=${1:-./project_1}
INPUT_DIR=${2:-./instances}
OUTPUT_DIR=${3:-./out}
TIMEOUT=1800

mkdir -p "$OUTPUT_DIR"

total=0
solved=0
failed=0

echo "Starting evaluation (Max timeout: ${TIMEOUT}s per instance)"
echo "--------------------------------------------------------"

for input_file in $(ls "$INPUT_DIR"/*.inp | sort -V); do
    base=$(basename "$input_file" .inp)
    output_file="$OUTPUT_DIR/$base.out"
    total=$((total + 1))

    echo -n "Solving $base ... "

    TIME_LOG=".time.tmp"
    
    /usr/bin/time -f "%e" -o "$TIME_LOG" timeout "$TIMEOUT" "$SOLVER" < "$input_file" > "$output_file" 2>/dev/null
    
    exit_code=$?
    exec_time=$(cat "$TIME_LOG")

    if [ $exit_code -eq 0 ]; then
        result=$(tail -n 1 "$output_file")
        echo "OK | Time: ${exec_time}s | Converted: $result"
        solved=$((solved + 1))
    elif [ $exit_code -eq 124 ]; then
        echo "TIMEOUT (Limit ${TIMEOUT}s reached)"
        rm -f "$output_file"
        failed=$((failed + 1))
    else
        echo "ERROR (Exit code $exit_code)"
        rm -f "$output_file"
        failed=$((failed + 1))
    fi
done

rm -f .time.tmp
echo "--------------------------------------------------------"
echo "Summary: $solved/$total solved, $failed failed/timeout"