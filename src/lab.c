#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <pwd.h>

#include "lab.h"

    /**
     * @brief Print the shell prompt to the terminal
     *
     * @param sh The shell
     */
    void print_prompt(struct shell *sh);
    
    /**
     * @brief Read a line from the terminal. This function will block until
     * a line is read from the terminal. This function will allocate memory
     * that must be freed by the caller.
     *
     * @param sh The shell
     * @return char* The line read from the terminal
     */
    char *read_line(struct shell *sh);
    
    /**
     * @brief Print the help message to the terminal
     *
     * @param sh The shell
     */
    void print_help(struct shell *sh);
    
    /**
     * @brief Print the version message to the terminal
     *
     * @param sh The shell
     */
    void print_version(struct shell *sh);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_cwd(struct shell *sh);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_jobs(struct shell *sh);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_fg(struct shell *sh, char **argv);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_bg(struct shell *sh, char **argv);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_kill(struct shell *sh, char **argv);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_exit(struct shell *sh);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_cd(struct shell *sh, char **argv);
    
    /**
     * @brief Print the current working directory to the terminal
     *
     * @param sh The shell
     */
    void print_pwd(struct shell *sh);
///////////////////////////////////////////////////////////////////

// define flags
#define FLAG_VERSION (1 << 0) // bit shift 1 to the left by 0
#define FLAG_DEBUG (1 << 1) // bit shift 1 to the left by 1

// static variables for the argument parsing
static int flags = 0;
static const char *cvalue = NULL;

/**
 * Helper function
 * 
 * @brief Print the arguments passed to the shell
 *
 * @param argc Number of arguments
 * @param argv The arguments
 */
void print_args_values() {
    printf("dflag = %d, vflag = %d, cvalue = %s optind = %d\n",
            (flags & FLAG_DEBUG) ? 1 : 0, 
            (flags & FLAG_VERSION) ? 1 : 0, 
            cvalue, optind);
}

/**
 * @brief Parse command line args from the user when the shell was launched
 *
 * @param argc Number of args
 * @param argv The arg array
 */
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

    // if the debug flag is set
    if (flags & FLAG_DEBUG) {
        print_args_values();
    }

    if (argc < 2) {
        printf("Usage: %s [-option1] [-option2] [-option3] [...]\n", argv[0]);
        printf("For help: %s -h\n", argv[0]);
    }
}

/**
 * @brief Set the shell prompt. This function will attempt to load a prompt
 * from the requested environment variable, if the environment variable is
 * not set a default prompt of "shell>" is returned.  This function calls
 * malloc internally and the caller must free the resulting string.
 *
 * @param env The environment variable
 * @return const char* The prompt
 */
char *get_prompt(const char *env) {
    // get the value of the environment variable
    const char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = "shell> ";
    }
    
    // allocate memory for the prompt
    char *new_prompt = strdup(prompt);
    if (new_prompt == NULL) {
        perror("strdup failed");
        exit(EXIT_FAILURE);
    }

    return new_prompt;
}



/**
 * Changes the current working directory of the shell. Uses the linux system
 * call chdir. With no arguments the users home directory is used as the
 * directory to change to.
 *
 * @param dir The directory to change to
 * @return  On success, zero is returned.  On error, -1 is returned, and
 * errno is set to indicate the error.
 */
int change_dir(char **dir) {
    // check if the directory is NULL
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

    // Print the current working directory (for debugging purposes)
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }

    return 0;
}

/**
 * @brief Convert line read from the user into to format that will work with
 * execvp. We limit the number of arguments to ARG_MAX loaded from sysconf.
 * This function allocates memory that must be reclaimed with the cmd_free
 * function.
 *
 * @param line The line to process
 *
 * @return The line read in a format suitable for exec
 */
char **cmd_parse(char const *line);

/**
 * @brief Free the line that was constructed with parse_cmd
 *
 * @param line the line to free
 */
void cmd_free(char ** line);

/**
 * @brief Trim the whitespace from the start and end of a string.
 * For example "   ls -a   " becomes "ls -a". This function modifies
 * the argument line so that all printable chars are moved to the
 * front of the string
 *
 * @param line The line to trim
 * @return The new line with no whitespace
 */

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
 * @brief Takes an argument list and checks if the first argument is a
 * built in command such as exit, cd, jobs, etc. If the command is a
 * built in command this function will handle the command and then return
 * true. If the first argument is NOT a built in command this function will
 * return false.
 *
 * @param sh The shell
 * @param argv The command to check
 * @return True if the command was a built in command
 */
bool do_builtin(struct shell *sh, char **argv) {
    // check if any arguments were passed
    if (argv[0] == NULL) {
        return false;
    }

    // handle the "exit" command
    if (strcmp(argv[0], "exit") == 0) {
        // free the memory allocated for the command
        cmd_free(argv);

        // destroy the shell
        sh_destroy(sh);

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

            return true;
        }

        // change to the directory provided as an argument
        if (change_dir(argv) != 0) {
            // print the error message
            perror("cd failed");
        }

        return true;
    }

    return false; // not a built-in command
}

/**
 * @brief Initialize the shell for use. Allocate all data structures
 * Grab control of the terminal and put the shell in its own
 * process group. NOTE: This function will block until the shell is
 * in its own program group. Attaching a debugger will always cause
 * this function to fail because the debugger maintains control of
 * the subprocess it is debugging.
 *
 * @param sh
 */
void sh_init(struct shell *sh);

/**
 * @brief Destroy shell. Free any allocated memory and resources and exit
 * normally.
 *
 * @param sh
 */
void sh_destroy(struct shell *sh);