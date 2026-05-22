# Street Directionality Problem
## Tommaso Bergonzoni

This project addresses the Street Directionality Problem using Linear Programming via the IBM ILOG CPLEX library.
It is required to execute the code in a Linux environment, with the g++ compiler and CPLEX library installed.

## Content
The project is organized as follows:

- **out/**: Contains the output files for the instances that were solved optimally.
- **src/**: Contains the source code and utility scripts:
    - `sdp.cpp`: The C++ file containing the model.
    - `Makefile`: Script for compiling the project.
    - `run_all.sh`: A bash script that runs the `program` executable over all files in a directory to produce output files.
        
```bash
        ./run_all.sh <input_dir> <solver> <output_dir>
        ```
    - `run_checks.sh`: A script to verify each output file in the `out/` directory using the provided checker.
        
```bash
        ./run_checks.sh <checker_executable> <output_dir>
        ```
    - `checker.cc`: The sanity check utility provided by the professor.
-**Report.pdf**: A written report of this project. 

## Compilation Instructions
Before running `make`, make sure to update the `CPLEXSTUDIODIR` path inside the `Makefile` if your CPLEX installation directory is different.

To compile both the solver and the checker, navigate to the `src` directory and run:
```bash
make