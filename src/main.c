/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Thursday,  8 January 2015

  @brief        LSH (Libstephen SHell)

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_pwd(char **args);
int lsh_echo(char **args);
int lsh_history(char **args);
int lsh_env(char **args);


/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "pwd",
  "echo",
  "history",
  "env"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_pwd,
  &lsh_echo,
  &lsh_history,
  &lsh_env
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Builtin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(char **args)
{
    // 1. Too many arguments
    if (args[1] != NULL && args[2] != NULL) {
        fprintf(stderr, "lsh: cd: too many arguments to \"cd\"\n");
        return 1;
    }

    // 2. No argument → change to HOME directory (or handle as you wish)
    if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "lsh: cd: HOME not set\n");
            return 1;
        }
        if (chdir(home) != 0) {
            perror("lsh: cd");
        }
        return 1;
    }

    // 3. Argument starts with '-' → invalid option
    if (args[1][0] == '-') {
        fprintf(stderr, "lsh: cd: %s: invalid option\n", args[1]);
        return 1;
    }

    // 4. Normal directory change
    if (chdir(args[1]) != 0) {
        perror("lsh: cd");
    }

    return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
  int i;
  if (args[1][0] == '-') {
    fprintf(stderr, "lsh: help: %s: invalid option\n", args[1]);
    return 1;
  }
  if (strcmp(args[1], "cd") == 0) {
    printf("cd: change the shell working directory.\n");
    printf("Usage: cd <directory>\n");
    printf("Options: has no implemented options\n");
  } else if (strcmp(args[1], "help") == 0) {
    printf("help: display information about builtin commands.\n");
    printf("Usage: help [command]\n");
    printf("Options: has no implemented options\n");
  } else if (strcmp(args[1], "exit") == 0) {
    printf("exit: exit the shell.\n");
    printf("Usage: exit\n");
  } else if (strcmp(args[1], "pwd") == 0) {
    printf("pwd: print the current working directory.\n");
    printf("Usage: pwd\n");
    printf("Options: has no implemented options\n");
  } else if (strcmp(args[1], "echo") == 0) {
    printf("echo: print arguments to the terminal.\n");
    printf("Usage: echo [arguments...]\n");
    printf("Options: has no implemented options\n");
  } else if (strcmp(args[1], "history") == 0) {
    printf("history: print the command history.\n");
    printf("Usage: history\n");
    printf("Args: history [n] - print the last n commands. If n is not provided, print the full history.\n");
    printf("Options: has no implemented options\n");
  } else if (strcmp(args[1], "env") == 0) {
    printf("env: print environment variables.\n");
    printf("Usage: env\n");
    printf("Options: has no implemented options\n");
  } else {
    fprintf(stderr, "lsh: no help topics match '%s'.\n", args[1]);
    return 1;
  }
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
  return 0;
}

/**
   @brief Builtin command: print working directory.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_pwd(char **args)
{  
  if (args[1][0] == '-') {
    fprintf(stderr, "lsh: pwd: %s: invalid option\n", args[1]);
    return 1;
  }
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  } else {
    perror("lsh: pwd");
  }
  return 1;
}

/**
   @brief Builtin command: echo arguments.
   @param args List of args.  args[0] is "echo".  args[1..n] are printed.
   @return Always returns 1, to continue executing.
 */
int lsh_echo(char **args)
{  int i = 1;
  // if (args[1] == NULL) {
  //   fprintf(stderr, "lsh: expected argument to \"echo\"\n");
  //   return 1;
  // }
  while (args[i] != NULL) {
    printf("%s ", args[i]);
    i++;
  }
  printf("\n");
  return 1;
}

/**
   @brief Builtin command: print command history.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
#define MAX_HISTORY 100
char *history[MAX_HISTORY];
int history_count = 0;

int lsh_history(char **args) {
    // Case 1: too many arguments
    if (args[1] != NULL && args[2] != NULL) {
        fprintf(stderr, "lsh: too many arguments to \"history\"\n");
        return 1;
    }

    // Case 2: no argument → show full history
    if (args[1] == NULL) {
        for (int i = 0; i < history_count; i++)
            printf("%d %s\n", i + 1, history[i]);
        return 1;
    }

    // Case 3: one argument – must not start with '-'
    if (args[1][0] == '-') {
        fprintf(stderr, "lsh: history: %s: invalid option\n", args[1]);
        return 1;
    }

    // Case 4: one argument – must be a valid number
    char *endptr;
    long n = strtol(args[1], &endptr, 10);

    if (*endptr != '\0') {
        fprintf(stderr, "lsh: history: argument is not a valid number. "
                "Numeric argument required.\n");
        return 1;
    }
    if (n <= 0) {
        fprintf(stderr, "lsh: history: number must be positive.\n");
        return 1;
    }
    if (n > history_count) {
        fprintf(stderr, "lsh: history: argument exceeds history count.\n");
        return 1;
    }

    printf("Showing last %ld commands:\n", n);
    for (int i = history_count - n; i < history_count; i++)
        printf("%d %s\n", i + 1, history[i]);

    return 1;
}


/**
   @brief Builtin command: print environment variables.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
extern char **environ;   // declared by the system

int lsh_env(char **args) {
    // Print each environment variable in "KEY=VALUE" format
    if (args[1][0] == '-') {
    fprintf(stderr, "lsh: env: %s: invalid option\n", args[1]);
    return 1;
    }
    if (args[1] != NULL) {
        fprintf(stderr, "lsh: env: %s: no such file or directory\n", args[1]);
        return 1;
    }
    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
    return 1;   // continue the shell loop
}


/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Add a command to the history.
   @param cmd The command to add.
 */

void add_to_history(const char *cmd) {
    if (cmd == NULL || *cmd == '\0') return;   // ignore empty lines

    // If the list is full, drop the oldest entry (shift the array left).
    if (history_count == MAX_HISTORY) {
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++)
            history[i] = history[i + 1];
        history_count--;
    }
    history[history_count++] = strdup(cmd);
}


/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    add_to_history(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

