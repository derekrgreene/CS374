/* Name: Derek Greene
*  OSU Email: greenede@oregonstate.edu
*  Course: CS 374 Operating Systems I
*  Assignment: SMALLSH
*  Due Date: 2/10/2025
*  Description: This program implements a simple shell that can run commands in the foreground and background, with input and output redirection.
*               It also handles built-in commands such as exit, cd, and status. Signals such as SIGINT, SIGCHLD, and SIGTSTP are handled as well.
*               Comments can be added to the input by starting the line with a '#'. Blank lines are ignored. The '&' character can be used to run
*               a command in the background. Program will terminate upon 'exit' command. 
*
*  References:
*  The Linux Programming Interface: a Linux and UNIX system programming handbook. Kerrisk, M. (2010)
*  Code adapted from CS 374 sample parser 'sample_parser.c'. 
*/


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


#define INPUT_LENGTH 2048
#define MAX_ARGS 512


int status = 0; // track exit status 
bool foregroundOnly = false; // track foreground only mode
pid_t fg_pid = 0; // track foreground process pid

struct command_line {
  char *argv[MAX_ARGS + 1];
  int argc;
  char *input_file;
  char *output_file;
  bool is_bg;     // background process flag
};


/*
* Function to parse user input and return a command_line struct
* Parameters: None
* Returns: struct
*/
struct command_line *parse_input() {
  char input[INPUT_LENGTH];
  struct command_line *curr_command = (struct command_line *)calloc(1, sizeof(struct command_line));

  // Get input
  printf(": ");
  fflush(stdout);

  if (fgets(input, INPUT_LENGTH, stdin) == NULL) {
    free(curr_command);
    return NULL;
  }
  // ignore comments
  if (input[0] == '#') {
    free(curr_command);
    return NULL;
  }

  // ignore blank lines
  if (input[0] == '\n') {
    free(curr_command);
    return NULL;
  }

  // Tokenize the input
  char *token = strtok(input, " \n");
  while (token) {
    if (!strcmp(token, "<")) {  // input redirection
      curr_command->input_file = strdup(strtok(NULL, " \n"));

    } else if (!strcmp(token, ">")) {  // output redirection
      curr_command->output_file = strdup(strtok(NULL, " \n"));

    } else if (!strcmp(token, "&")) {  // background process
      if (strtok(NULL, "\n") == NULL) {
        curr_command->is_bg = true;

      } else {
        curr_command->argv[curr_command->argc++] = strdup(token);
      }
    } else {
      curr_command->argv[curr_command->argc++] = strdup(token);
    }
    token = strtok(NULL, " \n");
  }
  return curr_command;
}


/*
* Function to handle exit, cd, and status commands, returns 0 if not a builtin command, 1 otherwise
* Parameters: struct command_line *curr_command
* Returns: int
*/
int builtin_commands(struct command_line *curr_command) {
  if (!strcmp(curr_command->argv[0], "exit")) {
    exit(0);
    return 1;

  } else if (!strcmp(curr_command->argv[0], "cd")) {
    if (curr_command->argc == 1) {
      chdir(getenv("HOME"));

    } else {
      chdir(curr_command->argv[1]);
    }
    return 1;

  } else if (!strcmp(curr_command->argv[0], "status")) {
    printf("exit value %d\n", status);
    return 1;
  }
  return 0;
}


/*
* Function to process input and output redirection
* Parameters: struct command_line *curr_command 
* Returns: void
*/
void inputOutput(struct command_line *curr_command){
  if (curr_command->input_file != NULL) {
    int input = open(curr_command->input_file, O_RDONLY);

    if (input == -1) {
      printf("cannot open %s for input\n", curr_command->input_file);
      status = 1;
      exit(1);
    }

    int result = dup2(input, STDIN_FILENO); // redirect input
    if (result == -1) {
      status = 1;
      close(input);
      exit(1);
    }
    close(input);
  }
  if (curr_command->output_file != NULL){
    int output = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    if (output == -1) {
      perror("open()");
      status = 1;
      return;
    }

    int result = dup2(output, STDOUT_FILENO); // redirect output
    if (result == -1) {
      perror("dup2()");
      status = 1;
      close(output);
      return;
    }
    close(output);
  }

  if (curr_command->is_bg){
    if (curr_command->input_file == NULL){
      int input = open("/dev/null", O_RDONLY);  // if background process, redirect input to /dev/null
      int result = dup2(input, STDIN_FILENO);
      
      if (result == -1) {
        perror("dup2()");
        status = 1;
        return;
      }
      close(input);
    }
    if (curr_command->output_file == NULL){
      int output = open("/dev/null", O_WRONLY);  // if background process, redirect output to /dev/null
      int result = dup2(output, STDOUT_FILENO);
      
      if (result == -1) {
        perror("dup2()");
        status = 1;
        return;
      }
      close(output);
    }
  }
}


/*
* Function to handle command execution
* Parameters: struct command_line *curr_command 
* Returns: void
*/
void handleCommand(struct command_line *curr_command) {
  if (foregroundOnly) {
    curr_command->is_bg = false;  // ignore & if foreground only mode
  }
    
  pid_t spawnPid = fork(); // fork child process
  switch(spawnPid) {
    case -1:
      perror("fork()\n");
      exit(1);
      break;
    case 0:
      // child process signal handling
      struct sigaction SIGTSTP_action = {0};
      SIGTSTP_action.sa_handler = SIG_IGN;
      sigaction(SIGTSTP, &SIGTSTP_action, NULL);

      if (curr_command->is_bg && !foregroundOnly) {
        signal(SIGINT, SIG_IGN);  // ignore SIGINT if background process

      } else {
        signal(SIGINT, SIG_DFL); // default SIGINT if foreground process
      }

      inputOutput(curr_command); // handle input and output redirection

      execvp(curr_command->argv[0], curr_command->argv); // execute command
      fprintf(stderr, "%s: no such file or directory\n", curr_command->argv[0]); // if execvp fails
      exit(1);
      break;

    default:
      if (curr_command->is_bg && !foregroundOnly) {
        printf("background pid is %d\n", spawnPid);
        curr_command->is_bg = false;
        return;

      } else {
        int childStatus;
        int fg_pid = spawnPid;
        pid_t donePid = waitpid(spawnPid, &childStatus, 0); // wait for child process to finish
        fg_pid = 0;

        if (WIFSIGNALED(childStatus)){ // if child process terminated by signal
          printf("terminated by signal %d\n", WTERMSIG(childStatus));

        } else {
          status = WEXITSTATUS(childStatus); // get exit status for child process
        }
    }
  }
}


/*
* Signal handler to check background process status 
* Parameters: int signum
* Returns: void
*/
void checkBgPids(int signum){
  pid_t pid;
  int exitStatus;
  char output[200];
 
  while ((pid = waitpid(-1, &exitStatus, WNOHANG)) > 0) {
    int outputLength = 0;
    
    if (WIFEXITED(exitStatus)) { //if child process exited normally
      outputLength = snprintf(output, sizeof(output), "\nbackground pid %d is done: exit value %d\n", pid, WEXITSTATUS(exitStatus));
    
    } else { // if child process terminated by signal
      outputLength = snprintf(output, sizeof(output), "\nbackground pid %d is done: terminated by signal %d\n", pid, WTERMSIG(exitStatus));
    }
    write(STDOUT_FILENO, output, outputLength);
    write(STDOUT_FILENO, ": ", 2);
    fflush(stdout);
  }
}


/*
* Signal handler for foreground only mode switching
* Parameters: int signum
* Returns: void
*/
void foregroundBackground(int signum) {
  if (fg_pid > 0) {
    bool switchMode = true;
    return;
  }

  if (!foregroundOnly) {
    foregroundOnly = true;
    char *output = "\nEntering foreground-only mode (& is now ignored)";
    write(STDOUT_FILENO, output, strlen(output));

  } else {
    foregroundOnly = false;
    char *output = "\nExiting foreground-only mode";
    write(STDOUT_FILENO, output, strlen(output));
  }
    char *output = "\n: ";
    write(STDOUT_FILENO, output, strlen(output));
}


/* 
* Main Function
* Parameters: None
* Returns: int
*/
int main() {
  bool switchMode = false;

  // signal handling for SIGINT, SIGCHLD, and SIGTSTP 
  struct sigaction SIGINT_action;
  SIGINT_action.sa_handler = SIG_IGN;
  SIGINT_action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &SIGINT_action, NULL);

  struct sigaction SIGCHLD_action;
  SIGCHLD_action.sa_handler = checkBgPids;
  SIGCHLD_action.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &SIGCHLD_action, NULL);

  struct sigaction SIGTSTP_action;
  SIGTSTP_action.sa_handler = foregroundBackground;
  SIGTSTP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  struct command_line *curr_command;
  while (true) {
    curr_command = parse_input();
    if (switchMode) {
      foregroundBackground(SIGTSTP); // enter foreground only mode
      switchMode = false; // reset switch mode flag
    }

    if (curr_command == NULL) { // if no input or comment
      continue;
    }

    if (builtin_commands(curr_command)) { // if builtin command
      free(curr_command);
      continue;

    } else {
      handleCommand(curr_command); // run command
      free(curr_command);
    }
  }
  return EXIT_SUCCESS;
}
