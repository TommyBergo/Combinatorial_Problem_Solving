#!/bin/bash

# Usage: ./run_all.sh <solver> <input_dir> <output_dir> [timeout_seconds]
# Example: ./run_all.sh ./project_1 ./instances ./out 60

SOLVER=${1:-./project_1}
INPUT_DIR=${2:-./instances}
OUTPUT_DIR=${3:-./out}
TIMEOUT=${4:-60}

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

total=0
solved=0
skipped=0
failed=0

# sort -V (version sort) correctly orders sdp.0, sdp.1, ..., sdp.9, sdp.10, sdp.11 ...
for input_file in $(ls "$INPUT_DIR"/*.inp | sort -V); do
    # Get just the filename without path and extension
    base=$(basename "$input_file" .inp)
    output_file="$OUTPUT_DIR/$base.out"

    total=$((total + 1))

    # Skip if output file already exists
    if [ -f "$output_file" ]; then
        result=$(tail -n 1 "$output_file")
        echo "Skipping $base (already solved, converted: $result)"
        skipped=$((skipped + 1))
        solved=$((solved + 1))
        continue
    fi

    echo -n "Solving $base ... "

    # Run solver with timeout, reading from input file and writing to output file
    timeout "$TIMEOUT" "$SOLVER" < "$input_file" > "$output_file" 2>/dev/null

    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        # Print the last line of the output (number of converted streets)
        result=$(tail -n 1 "$output_file")
        echo "OK (converted: $result)"
        solved=$((solved + 1))
    elif [ $exit_code -eq 124 ]; then
        echo "TIMEOUT (${TIMEOUT}s exceeded)"
        # Remove incomplete output file
        rm -f "$output_file"
        failed=$((failed + 1))
    else
        echo "ERROR (exit code $exit_code)"
        rm -f "$output_file"
        failed=$((failed + 1))
    fi
done

echo ""
echo "Done: $solved/$total solved ($skipped skipped), $failed failed/timeout"