#!/bin/bash

# Usage: ./run_all.sh <input_dir> <solver> <output_dir>
INPUT_DIR=$1
SOLVER=${2:-./sdp}
OUTPUT_DIR=${3:-./out}
TIMEOUT=3600

if [ -z "$INPUT_DIR" ] || [ ! -d "$INPUT_DIR" ]; then
    echo "Usage: $0 <input_dir> [solver] [output_dir]"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

TIME_LOG=".time.tmp"

for input_path in "$INPUT_DIR"/*.inp; do
    [ -e "$input_path" ] || continue

    base=$(basename "$input_path" .inp)
    output_file="$OUTPUT_DIR/$base.out"

    if [ -f "$output_file" ]; then
        echo "Skipping $base: already processed."
        continue
    fi

    echo -n "Solving $base ... "

    /usr/bin/time -f "%e" -o "$TIME_LOG" timeout "$TIMEOUT" "$SOLVER" < "$input_path" > "$output_file" 2>/dev/null
    
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
done

rm -f "$TIME_LOG"