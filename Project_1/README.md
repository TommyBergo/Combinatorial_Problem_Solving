# Street Directionality Problem
## Tommaso Bergonzoni

This project addresses the Street Directionality Problem using Constraint Programming via the Gecode library.
It is required to execute the code in a Linux environment, with the g++ compiler and Gecode library installed

## Content
The project is organized into the following directories:

- **out/**: Contains the output files for the instances that were solved optimally.
- **src/**: Contains the source code and utility scripts:
    - `sdp.cpp`: The single C++ source file containing the Gecode model.
    - `Makefile`: Script for compiling the project.
    - `run_all.sh`: A bash script that runs the `sdp` executable over all files in a directory to produce output files.
    - `run_checks.sh`: A script to verify each output file in the `out/` directory using the provided checker.
    - `checker.cpp`: The sanity check utility provided by the professor.

## Compilation Instructions
To compile both the solver (`sdp`) and the checker, navigate to the `src` directory and run:
```bash
make

