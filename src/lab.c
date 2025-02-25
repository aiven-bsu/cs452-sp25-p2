#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <readline/history.h>

#include "lab.h"

// define flags
#define FLAG_VERSION (1 << 0)   // bit shift 1 to the left by 0
#define FLAG_DEBUG (1 << 1)     // bit shift 1 to the left by 1

// static variables for the argument parsing
static int flags = 0;
static const char *cvalue = NULL;

/**
 * Helper function
 * 
 * @brief Print the arguments passed to the shell
 */
void print_args_values() {
    printf("dflag = %d, vflag = %d, cvalue = %s optind = %d\n",
            (flags & FLAG_DEBUG) ? 1 : 0, 
            (flags & FLAG_VERSION) ? 1 : 0, 
            cvalue, optind);
}

/* Parse the command line arguments when the shell is launched */
void parse_args(int argc, char **argv) {
    int opt;

    // parse args/options
    while ((opt = getopt(argc, argv, "vc:dh")) != -1) {
        switch (opt) {
            case 'v':
                flags |= FLAG_VERSION; // enable the version flag

                if (flags & FLAG_VERSION) {
                    printf("%s version %.1f\n", argv[0], (double)lab_VERSION_MAJOR);
                }
    
                break;
            case 'c':
                cvalue = optarg;

                // set the value of the environment variable MY_PROMPT
                setenv("MY_PROMPT", cvalue, 1);
                break;
            case 'd':
                flags |= FLAG_DEBUG; // enable the debug flag
                break;
            case 'h':
                // prints the usage message and options to the standard output
                printf("Usage: %s [-option1] [-option2] [-option3] [...]\n", argv[0]);
                printf("Options:\n");
                printf("  -c \"MY_PROMPT\"\tSet the value for the enviornment variable MY_PROMPT\n");
                printf("  -d\t\t\tTurn on the debug flag\n");
                printf("  -h\t\t\tDisplay the help message\n");
                printf("  -v\t\t\tPrint the version number\n");
                return; // exit the function 
            case '?':
                if (optopt == 'c') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                }
                break;
            default:
                printf("aborting...\n");
                abort();
        }
    }

    // print argument values if the debug flag is set
    if (flags & FLAG_DEBUG) {
        print_args_values();
    }

    // print usage message if no arguments are provided
    if (argc < 2) {
        printf("Usage: %s [-option1] [-option2] [-option3] [...]\n", argv[0]);
        printf("For help: %s -h\n", argv[0]);
    }
}

/* set the shell prompt */
char *get_prompt(const char *env) {
    // get the value of the environment variable
    const char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = "shell> ";
    }
    
    // internally allocate memory for the prompt
    char *new_prompt = strdup(prompt);
    if (new_prompt == NULL) {
        perror("strdup failed");
        exit(EXIT_FAILURE);
    }

    return new_prompt;
}

/* Changes the current working directory of the shell. */
int change_dir(char **dir) {
    // first check if the directory is NULL
    if (dir[1] == NULL) {
        // get the home directory
        const char *home = getenv("HOME");

        // change to the home directory if no arguments are provided
        if (home == NULL) {
            // get the user ID
            uid_t uid = getuid();

            // get the password entry for the user ID
            struct passwd *pw = getpwuid(uid);

            // check if the password entry is NULL
            if (pw == NULL) {
                perror("getpwuid failed");
                return -1;
            }

            // get the user home directory
            home = pw->pw_dir;
        }

        // after setting the home directory, check the return value of chdir
        if (chdir(home) != 0) {
            // print the error message
            perror("chdir failed");
            return -1;
        }

    } else {
        // change to the directory provided as an argument
        if (chdir(dir[1]) != 0) {
            // print the error message
            perror("chdir failed");
            return -1;
        }
    }

    // Print the current working directory if the debug flag is set
    if (flags & FLAG_DEBUG) {
        char cwd[1024];
        // get the current working directory
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working directory: %s\n", cwd);
        } else {
            perror("getcwd() error");
        }
    }

    return 0;
}

/* Handle parsing and double quotes */
char **cmd_parse(char const *line) {
    size_t arg_max = sysconf(_SC_ARG_MAX);
    char **cmd = malloc(arg_max * sizeof(char *));
    if (cmd == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    const char *p = line;
    while (*p != '\0') {
        // Skip leading whitespace
        while (isspace(*p)) {
            p++;
        }

        // Break if end of the string
        if (*p == '\0') {
            break;
        }

        // Handle quoted strings
        if (*p == '"') {
            p++;
            const char *start = p;
            while (*p != '"' && *p != '\0') {
                p++;
            }

            // Check for unmatched quote
            if (*p == '\0') {
                perror("Unmatched quote");
                exit(EXIT_FAILURE);
            }

            size_t len = p - start;
            cmd[i] = strndup(start, len);
            if (cmd[i] == NULL) {
                perror("strndup failed");
                exit(EXIT_FAILURE);
            }
            p++; // Skip closing quote

        } else {
            // Handle unquoted strings
            const char *start = p;
            while (!isspace(*p) && *p != '\0') {
                p++;
            }

            size_t len = p - start;
            cmd[i] = strndup(start, len);
            if (cmd[i] == NULL) {
                perror("strndup failed");
                exit(EXIT_FAILURE);
            }
        }

        // Print the token if the debug flag is set
        if (flags & FLAG_DEBUG) {
            printf("cmd[%d]: %s\n", i, cmd[i]);
        }

        i++;
    }

    // Set the last element to NULL
    cmd[i] = NULL;

    // Print the final command array if the debug flag is set
    if (flags & FLAG_DEBUG) {
        printf("Parsed command: ");
        for (int j = 0; cmd[j] != NULL; j++) {
            printf("%s ", cmd[j]);
        }
        printf("\n");
    }

    return cmd;
}

/**
 * @brief Free the line that was constructed with parse_cmd
 *
 * @param line the line to free
 */
void cmd_free(char ** line) {
    // free the memory allocated for the command
    for (int i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }

    // free the memory allocated for the command
    free(line);
}

/* Trim whitespaces from user input */
char *trim_white(char *line) {
    // pointer to the start of the string
    char *start = line;

    // pointer to the end of the string
    char *end = line + strlen(line) - 1;

    // move the start pointer to the first non-whitespace character
    while (isspace(*start)) {
        start++;
    }

    // move the end pointer to the last non-whitespace character
    while (end > start && isspace(*end)) {
        end--;
    }

    // null-terminate the string
    end[1] = '\0';

    return start;
}

/**
 * Helper function
 * 
 * @brief Print the history of the shell
 */
void print_history(int print_to_stdout) {
    // print the history
    HIST_ENTRY **the_history_list;
    the_history_list = history_list();

    // check if the history list is not NULL
    if (the_history_list) {
        // get the length of the history list
        if (print_to_stdout) {
            printf("History length: %d\n", history_length);
        }

        // print the history list
        for (int i = 0; the_history_list[i]; i++) {
            if (print_to_stdout) {
                printf("%d: %s\n", i, the_history_list[i]->line);
            }
        }
    }
}

/* Handle defined builtin commands */
bool do_builtin(struct shell *sh, char **argv) {
    // return value
    bool status = false;

    // check if any arguments were passed
    if (argv[0] == NULL) {
        return status;
    }

    // handle the "exit" command
    if (strcmp(argv[0], "exit") == 0) {
        // destroy the shell
        sh_destroy(sh);

        // update the status
        status = true;

        // exit the program
        exit(EXIT_SUCCESS);
    }

    // handle the "cd" command
    if (strcmp(argv[0], "cd") == 0) {
        // change to the home directory if no arguments are provided
        if (argv[1] == NULL) {
            // change to the home directory
            if (change_dir(argv) != 0) {
                // print the error message
                perror("cd failed");
            }

            // update the status
            status = true;

            return status;
        }

        // change to the directory provided as an argument
        if (change_dir(argv) != 0) {
            // print the error message
            perror("cd failed");
        }

        // update the status
        status = true;

        return status;
    }

    // handle built-in "printhistory" command
    if (strcmp(argv[0], "printhistory") == 0) {
        // print the history
        print_history(1);

        // update the status
        status = true;

        return status;
    }

    // update the status
    status = false;

    return status; // not a built-in command
}

/* Initialize the shell */
void sh_init(struct shell *sh) {
     // check if the shell is NULL
     if (sh == NULL) {
        perror("NULL shell pointer");
    }

    /* See if we are running interactively.  */
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty (sh->shell_terminal);

    if (sh->shell_is_interactive) {
        /* Loop until we are in the foreground.  */
        while (tcgetpgrp (sh->shell_terminal) != (sh->shell_pgid = getpgrp ()))
            kill (sh->shell_pgid, SIGTTIN);

        /* Ignore interactive and job-control signals.  */
        signal (SIGINT, SIG_IGN);
        signal (SIGQUIT, SIG_IGN);
        signal (SIGTSTP, SIG_IGN);
        signal (SIGTTIN, SIG_IGN);
        signal (SIGTTOU, SIG_IGN);

        /* Put ourselves in our own process group.  */
        sh->shell_pgid = getpid ();
        if (setpgid (sh->shell_pgid, sh->shell_pgid) < 0) {
            // perror ("Couldn't put the shell in its own process group");
            // exit (1);
            perror("Couldn't put the shell in its own process group");
        }

        /* Grab control of the terminal.  */
        tcsetpgrp (sh->shell_terminal, sh->shell_pgid);

        /* Save default terminal attributes for shell.  */
        tcgetattr (sh->shell_terminal, &sh->shell_tmodes);
    }

    // Set the prompt from the environment variable "MY_PROMPT"
    sh->prompt = get_prompt("MY_PROMPT");
}

/* Free shell members and reset the terminal settings */
void sh_destroy(struct shell *sh) {
    // Reset the terminal
    if (sh->shell_is_interactive) {
        tcsetattr(sh->shell_terminal, TCSADRAIN, &sh->shell_tmodes);
    }

    // free the prompt
    if (sh->prompt != NULL) {
        free(sh->prompt);
    }

    // Do not free the shell structure itself
}