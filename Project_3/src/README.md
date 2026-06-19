# Street Directionality Problem
## Tommaso Bergonzoni

This project addresses the Street Directionality Problem by encoding it into a Boolean Satisfiability (SAT) problem and solving it using the Kissat SAT solver.
It is required to execute the code in a Linux environment with a g++ compiler installed.

## Content
The project is organized as follows:

- **out/**: Contains the output files for the instances that were solved.
- **src/**: Contains the source code, solver, and utility scripts:
  - **kissat/**: The source code of the Kissat SAT solver (the only solver used in this project).
  - `sdp.cpp`: The C++ file containing the problem encoder, solver invoker, and decoder.
  - `Makefile`: Script for compiling both the Kissat solver and the main project.
  - `run_all.sh`: A bash script that runs the `program` executable over all files in a directory to produce output files.
```bash
    ./run_all.sh <input_dir> <solver> <output_dir>
```
  - `run_checks.sh`: A script to verify each output file in the `out/` directory using the provided checker.
```bash
    ./run_checks.sh <checker_executable> <output_dir>
```
  - `checker.cc`: The sanity check utility provided by the professor.
- **Report.pdf**: A written report of this project.

## Compilation Instructions
To compile the solver, the main program, and the checker, navigate to the `src` directory and run:

```bash
make
