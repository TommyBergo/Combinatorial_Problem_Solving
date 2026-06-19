#!/bin/bash

# Usage: ./run_checks.sh <checker_executable> <output_dir>
CHECKER=${1:-./checker}
INPUT_DIR=${2:-../out} 

if [[ ! -x "$CHECKER" ]]; then
    echo "Error: $CHECKER not found or is not executable."
    exit 1
fi

if [[ ! -d "$INPUT_DIR" ]]; then
    echo "Error: Directory $INPUT_DIR does not exist."
    exit 1
fi

echo "Running validation using $CHECKER on directory $INPUT_DIR..."
echo "------------------------------------------------"

failed_files=()

for file in "$INPUT_DIR"/*; do
    [[ -f "$file" ]] || continue

    output=$($CHECKER < "$file" 2>&1)

    if [[ "$output" == "OK" ]]; then
        echo "$(basename "$file"): OK"
    else
        echo "$(basename "$file"): ERROR"
        echo "  -> $output"
        failed_files+=("$(basename "$file")")
    fi
done

echo "------------------------------------------------"
if [ ${#failed_files[@]} -eq 0 ]; then
    echo "All files passed successfully!"
else
    echo "Validation failed for ${#failed_files[@]} file(s):"
    for f in "${failed_files[@]}"; do
        echo "  - $f"
    done
fi