#!/bin/bash

# Usage: ./run_all.sh <input_file> <solver> <output_dir>
INPUT_FILE=$1
SOLVER=${2:-./sdp}
OUTPUT_DIR=${3:-./out}
TIMEOUT=60

if [ -z "$INPUT_FILE" ]; then
    echo "Usage: $0 <input_file> [solver] [output_dir]"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

base=$(basename "$INPUT_FILE" .inp)
output_file="$OUTPUT_DIR/$base.out"

echo -n "Solving $base ... "

TIME_LOG=".time.tmp"

/usr/bin/time -f "%e" -o "$TIME_LOG" timeout "$TIMEOUT" "$SOLVER" < "$INPUT_FILE" > "$output_file" 2>/dev/null

exit_code=$?
exec_time=$(cat "$TIME_LOG" 2>/dev/null)

if [ $exit_code -eq 0 ]; then
    result=$(tail -n 1 "$output_file")
    echo "OK | Time: ${exec_time}s | Converted: $result"
elif [ $exit_code -eq 124 ]; then
    echo "TIMEOUT (Limit ${TIMEOUT}s reached)"
    rm -f "$output_file"
else
    echo "ERROR (Exit code $exit_code)"
    rm -f "$output_file"
fi

rm -f .time.tmp