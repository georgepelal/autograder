/*
 * This program is an automated grading tool for C assignments.
 * It compiles a given C source file, runs the resulting executable with specified arguments and input,
 * and compares its output to a reference output file using a diff tool.
 * The program enforces a timeout for execution and checks for abnormal termination or memory errors.
 */

/*
 * Grading is based on four components:
 * - Compilation: Deductions for warnings, failure for errors.
 * - Termination: Deductions for abnormal termination or timeout.
 * - Output: Percentage of matching bytes between program output and reference output.
 * - Memory Access: Deductions for memory-related errors (e.g., segmentation faults).
 * The final score is the sum of these components, with a minimum of zero.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>

// Timer handler for SIGALRM 
static void timer_handler(int signum){
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 6) {
        perror("Usage: ./autograder <c_filename> <args_filename> <in_filename> <out_filename> <timeout>\n");
        return 1;
    }
    // Declare variables for the input arguments
    char c_filename[strlen(argv[1]) + 1];
    char args_filename[strlen(argv[2]) + 1];
    char in_filename[strlen(argv[3]) + 1];
    char out_filename[strlen(argv[4]) + 1];
    int timeout = atoi(argv[5]);
    // Copy the input arguments to the variables
    strcpy(c_filename, argv[1]);
    strcpy(args_filename, argv[2]);
    strcpy(in_filename, argv[3]);
    strcpy(out_filename, argv[4]);

    // Extract the filename without the .c extension
    char filename[strlen(c_filename) - 1];
    strncpy(filename, c_filename, strlen(c_filename) - 2);
    filename[strlen(c_filename) - 2] = '\0';

    // Create the error file name by appending ".err" to the filename
    char errorFilename[strlen(filename) + 5];
    strcpy(errorFilename, filename);
    strcat(errorFilename, ".err");

    // Create the executable path by appending "./" to the filename
    char exec_path[1024];
    snprintf(exec_path, sizeof(exec_path), "./%s", filename);
    
    // Structure to store grading results
    struct {
        int compilation;
        int termination;
        int output;
        int Memory;
        int score;
    } userGrade = {0,0,0,0,0};

    // Fork a child process to compile the C file
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Error forking process for compilation");
        return 255; // Return 255 on error
    
    }
    if (pid1 == 0){
        // Child process: compile the C file using gcc
        int errorfile = open(errorFilename,O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR );
        if (errorfile < 0) {
            perror("Error opening output file");
            exit(255);
        }
        if (dup2(errorfile, STDERR_FILENO) < 0) { // Redirect stderr to error file
            perror("Error redirecting stdout to error file");
            exit(255);
        }
        if (close(errorfile) < 0) {
            perror("Error closing error file");
            exit(255);
        
        }
        // Execute the gcc command to compile the C file
        if (execlp("gcc", "gcc","-Wall",c_filename, "-o", filename, NULL) < 0) {
            perror("Error executing gcc");
            exit(255);
        }
    }
    int status = -1;
    if (waitpid(pid1, &status, 0) < 0) {
        perror("Error waiting for child process");
        return 255; // Return 255 on error
    }
    status = WEXITSTATUS(status); 
    if (status == 255){
        return 255;
    }

    // Check if the compilation was successful by reading the error file
    FILE *errorfile = fopen(errorFilename, "r");
    if (errorfile == NULL) {
        perror("Error opening error file");
        return 255;
    }
    
    // Read the error file to check for errors and warnings
    char line[1024];
    int errors = 0, warnings = 0;
    while (fgets(line, sizeof(line), errorfile)) {
        if(strstr(line,filename)){
            if (strstr(line, "warning:")) {
                warnings += 1;
            }else {
                if (strstr(line, "error:")) {
                    errors += 1;
                }
            }
        }
    }
    if (fclose(errorfile) < 0) {
        perror("Error closing error file");
        return 255;
    
    }
    for (int i = 0; i < warnings; i++) {
        userGrade.compilation -= 5; // Deduct 5 points for each warning
    }

    // Exit if there are compilation errors
    if (errors > 0) {
        userGrade.compilation = -100;
    }else{
        // Read the arguments from the args file
        char *arguments[10000];
        char args_buffer[10000];
        char *token;
        int argcount = 1;
        arguments[0] = exec_path; 
        // Open the args file and read its contents
        int argsfd = open(args_filename, O_RDONLY);
        if (argsfd < 0) {
            perror("error opening args file");
            return 255;
        }
        if (read(argsfd, args_buffer, sizeof(args_buffer)) < 0) {
            perror("Error reading args file");
            close(argsfd);
            return 255;
        }
        if (close(argsfd) < 0) {
            perror("Error closing args file");
            return 255;
        }
        // Split the arguments by spaces
        token = strtok(args_buffer, " \n");
        while (token != NULL) {
            arguments[argcount] = token;
            argcount++;
            token = strtok(NULL, " \n");
        }
        arguments[argcount] = NULL; // Null-terminate the arguments array
        
        // Create a pipe to communicate between the child processes 2 and 3
        int pipe2_3[2];
        pipe(pipe2_3); 
        // Set up a signal handler for the timer
        struct sigaction action = {{0}};
        action.sa_handler = timer_handler;
        sigaction(SIGALRM, &action, NULL);
        // Set up the timer for the timeout
        struct itimerval timer = {{0}};
        timer.it_value.tv_sec = timeout; // Set the timeout value
        timer.it_value.tv_usec = 0;
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGALRM);  
        sigprocmask(SIG_BLOCK, &mask, NULL); // Block SIGALRM
        setitimer(ITIMER_REAL, &timer, NULL); // Start the timer
        // Fork a process to run the executable
        pid_t pid2 = fork();
        if (pid2 < 0) {
            perror("Error forking process");
            return 255;
        
        }
        if (pid2 == 0) {
            if (close(pipe2_3[0]) < 0) { // Close the read end of the pipe in the child process
                perror("Error closing pipe read end");
                exit(255);
            }
            // Redirect stdin to the input file and stdout to the pipe
            int in_file = open(in_filename, O_RDONLY);
            if (in_file < 0) {
                perror("Error opening input file");
                exit(255);
            }
            if (dup2(in_file, STDIN_FILENO) < 0) {// Redirect stdin to the input file
                perror("Error redirecting stdin to input file");
                exit(255);
            }

            if (close(in_file) < 0) {
                perror("Error closing input file");
                exit(255);
            }
            if (dup2(pipe2_3[1], STDOUT_FILENO) < 0){ // Redirect stdout to the pipe
                perror("Error redirecting stdout to pipe");
                exit(255);
            
            }
            if (close(pipe2_3[1]) < 0) {
                perror("Error closing pipe write end");
                exit(255);
            }

            // Execute the compiled program with arguments from args_filename
            
            if (execvp(exec_path, arguments) < 0) {
                perror("Error executing the compiled program");
                exit(255);
            }
        }
        // Fork a process to run ./diff
        pid_t pid3 = fork();
        if (pid3 < 0) {
            perror("Error forking process");
            kill(pid2, SIGKILL);
            return 255;
        }
        if (pid3 == 0) {
            if (close(pipe2_3[1]) < 0) { // Close the write end of the pipe in the child process
                perror("Error closing pipe write end");
                exit(255);
            }
            if (dup2(pipe2_3[0], STDIN_FILENO) < 0) { // Redirect stdin to the pipe
                perror("Error redirecting stdin to pipe");
                exit(255);
            
            }
            if (close(pipe2_3[0]) < 0) {// Close the read end of the pipe in the child process
                perror("Error closing pipe read end");
                exit(255);
            }

            // Execute the diff program
            if (execlp("./diff","./diff",  out_filename , NULL) < 0) {
                perror("Error executing diff");
                exit(255);
            }
        }
        if (close(pipe2_3[1]) < 0) { // Close the write end of the pipe in the parent process
            perror("Error closing pipe write end in parent");
            return 255;
        }
        if (close(pipe2_3[0]) < 0) { // Close the read end of the pipe in the parent process
            perror("Error closing pipe read end in parent");
            return 255;
        }

        int p2_status = -1;
        // Wait for the executable process, kill if timeout
        while(waitpid(pid2, &p2_status, WNOHANG) == 0){
            sigpending(&mask); // Check if the SIGALRM signal is pending
            if (sigismember(&mask, SIGALRM)) {
                kill(pid2, SIGKILL); // Kill the child process if the timer expires
            }
        }
        if (WEXITSTATUS(p2_status) == 255) { // If the child process exited with 255, return 255
            kill(SIGKILL, pid3); 
            return 255;
        }
        if (WIFSIGNALED(p2_status)) {
            p2_status = WTERMSIG(p2_status); // Get the signal that terminated the process
            userGrade.termination = 0; 
            if (p2_status == SIGSEGV || p2_status == SIGABRT || p2_status == SIGBUS) {
                // Deduct points for memory errors
                userGrade.Memory = -15;
            }
            if (p2_status == SIGKILL) {
                // If the process was killed by the timer, set termination to -100
                userGrade.termination = -100;
            }
        }
        // Wait for the diff process to finish and get the output score
        int percentage = -2;
        waitpid(pid3,&percentage, 0);
        if (WEXITSTATUS(percentage) == 255) {
            return 255; // If the diff process exited with 255, return 255
        }
        userGrade.output = WEXITSTATUS(percentage);
    }
    // Calculate the final score
    userGrade.score = userGrade.compilation + userGrade.termination
    + userGrade.output + userGrade.Memory;
    if (userGrade.score < 0) {
        userGrade.score = 0; // Ensure the score is not negative
    }
    // Print the grading results
    printf("\nCompilation: %d\n",userGrade.compilation);
    printf("\nTermination: %d\n",userGrade.termination);
    printf("\nOutput: %d\n", userGrade.output);
    printf("\nMemory access: %d\n",userGrade.Memory);
    printf("\nScore: %d\n",userGrade.score);   
    return 0;
}