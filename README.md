# P2 Interactive Shell

## Overview
This project involves implementing a simple shell program that can start
background processes. To implement the shell various tasks related to 
system calls, process management, and user input handling must be completed. 

## Project Outcomes

- **Interactive Shell**: A functional interactive shell that allows users to execute commands, manage processes, and navigate the file system.
- **Built-in Commands**: Implementation of essential built-in commands (`cd`, `exit`, etc.) with proper handling and error checking.
- **Process Management**: Robust process management, including forking, signal handling, and process group management.
- **Command History**: Support for command history, allowing users to view and reuse previously executed commands.
- **Customizable Prompt**: Ability to customize the shell prompt through environment variables.
- **Comprehensive Testing**: A suite of tests to verify the correctness and reliability of the shell's functionality.


**Below contain the steps to configure, build, run, and test the project**

## Building

```bash
make
```

## Testing

```bash
make check
```

## Valgrind Testing
```bash
# Runs valgrind on the main program
make valgrind1
```
```bash
# Runs valgrind on the test program
make valgrind2
```

## Clean

```bash
make clean
```

## Install Dependencies

In order to use git send-mail you need to run the following command:

```bash
make install-deps
```
