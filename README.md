## Overview

`autograder` is a command-line tool designed to automatically grade C programming assignments. It compiles submissions, runs them against predefined test cases, and grades it(0-100).

## Usage

1. **Prepare Test Cases**  
    Place your input and expected output files in the same directory

2. **Run the Autograder**  
    Execute the autograder with the path to the C source file:
    ```bash
    ./autograder <c_filename> <args_filename> <in_filename> <out_filename> <timeout> 
    ```

3. **View Results**  
    After grading, a summary report will be displayed in the terminal.

## How It Works

- **Compilation:**  
  The autograder compiles the submitted C file using `gcc`. On compilation errors and the grading process stops for that submission.

- **Execution:**  
  For each test case, the compiled program is executed with the corresponding input file. The output is captured and compared to the expected output.

- **Grading:**  
  Each test case is assigned a score. The autograder checks for exact matches or, optionally, allows for whitespace differences. The total score is calculated as the sum of passed test cases.

- **Reporting:**  
  A detailed report is generated, showing which test cases passed or failed, compilation errors (if any), and the final score.


## Requirements

- GCC compiler
- Bash shell (for running scripts)
- Linux or macOS environment
