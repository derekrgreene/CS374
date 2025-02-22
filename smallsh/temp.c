/**
 * A sample program for parsing a command line. If you find it useful,
 * feel free to adapt this code for Assignment 4.
 * Do fix memory leaks and any additional issues you find.
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

int status = 0;
bool foregroundOnly = false;

struct command_line {
  char *argv[MAX_ARGS + 1];
  int argc;
  char *input_file;
  char *output_file;
  bool is_bg;
};

struct command_line *parse_input() {
  char input[INPUT_LENGTH];
  struct command_line *curr_command =
      (struct command_line *)calloc(1, sizeof(struct command_line));

  // Get input
  printf(": ");
  fflush(stdout);
  if (fgets(input, INPUT_LENGTH, stdin) == NULL) {
    free(curr_command);
    return NULL;
  }
  // Handle comments
  if (input[0] == '#') {
    free(curr_command);
    return NULL;
  }
  // handle blank lines
  if (input[0] == '\n') {
    free(curr_command);
    return NULL;
  }

  // Tokenize the input
  char *token = strtok(input, " \n");

  while (token) {
    if (!strcmp(token, "<")) {
      curr_command->input_file = strdup(strtok(NULL, " \n"));
    } else if (!strcmp(token, ">")) {
      curr_command->output_file = strdup(strtok(NULL, " \n"));
    } else if (!strcmp(token, "&")) {
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


void inputOutput(struct command_line *curr_command){
  if (curr_command->input_file != NULL) {
    int input = open(curr_command->input_file, O_RDONLY);
    if (input == -1) {
      printf("cannot open %s for input\n", curr_command->input_file);
      status = 1;
      return;
    }
    int result = dup2(input, STDIN_FILENO);
    if (result == -1) {
      status = 1;
      return;
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
    int result = dup2(output, STDOUT_FILENO);
    if (result == -1) {
      perror("dup2()");
      status = 1;
      return;
    }
    close(output);
  }
  if (curr_command->is_bg){
    if (curr_command->input_file == NULL){
      int input = open("/dev/null", O_RDONLY);
      int result = dup2(input, STDIN_FILENO);
      if (result == -1) {
        perror("dup2()");
        status = 1;
        return;
      }
      close(input);
    }
    if (curr_command->output_file == NULL){
      int output = open("/dev/null", O_WRONLY);
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

void handleCommand(struct command_line *curr_command) {
    if (foregroundOnly) {
      curr_command->is_bg = false;
    }

    pid_t spawnPid = fork();
    switch(spawnPid) {
    case -1:
        perror("fork()\n");
        exit(1);
        break;
    case 0:
        struct sigaction SIGTSTP_action = {0};
        SIGTSTP_action.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);

        if (curr_command->is_bg && !foregroundOnly) {
            signal(SIGINT, SIG_IGN);
        } else {
          signal(SIGINT, SIG_DFL);
        }
        inputOutput(curr_command);
        execvp(curr_command->argv[0], curr_command->argv);
        fprintf(stderr, "%s: no such file or directory\n", curr_command->argv[0]);
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
          pid_t donePid = waitpid(spawnPid, &childStatus, 0);
          fg_pid = 0;
          if (WIFSIGNALED(childStatus)){
            printf("terminated by signal %d\n", WTERMSIG(childStatus));
          } else {
            status = WEXITSTATUS(childStatus);
          }
        }
    }
}

pid_t fg_pid = 0;

void checkBgPids(int signum){
  pid_t pid;
  int exitStatus;

  while ((pid = waitpid(-1, &exitStatus, WNOHANG)) > 0) {
    if (WIFEXITED(exitStatus)) {
      printf("\nbackground pid %d is done: exit value %d\n", pid, WEXITSTATUS(exitStatus));
      printf(": ");
      fflush(stdout);
    } else {
      printf("\nbackground pid %d is done: terminated by signal %d\n", pid, WTERMSIG(exitStatus));
      printf(": ");
      fflush(stdout);
    }
  }
}

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

int main() {
  bool switchMode = false;
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
      foregroundBackground(SIGTSTP);
      switchMode = false;
    }
    if (curr_command == NULL) {
      continue;
    }
    if (builtin_commands(curr_command)) {
      free(curr_command);
      continue;
    } else {
      handleCommand(curr_command);
      free(curr_command);
    }
  }
  return EXIT_SUCCESS;
}
