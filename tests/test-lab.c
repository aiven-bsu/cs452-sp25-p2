#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <readline/history.h>
#include "harness/unity.h"
#include "../src/lab.h"

void setUp(void) {
  // set stuff up here

}

void tearDown(void) {
  // clean stuff up here

}

void test_cmd_parse2(void)
{
  //The string we want to parse from the user.
  //foo -v
  char *stng = (char*)malloc(sizeof(char)*7);
  strcpy(stng, "foo -v");
  char **actual = cmd_parse(stng);
  //construct our expected output
  size_t n = sizeof(char*) * 6;
  char **expected = (char**) malloc(sizeof(char*) *6);
  memset(expected,0,n);
  expected[0] = (char*)malloc(sizeof(char)*4);
  expected[1] = (char*)malloc(sizeof(char)*3);
  expected[2] = (char*)NULL;
  strcpy(expected[0], "foo");
  strcpy(expected[1], "-v");
  TEST_ASSERT_EQUAL_STRING(expected[0],actual[0]);
  TEST_ASSERT_EQUAL_STRING(expected[1],actual[1]);
  TEST_ASSERT_FALSE(actual[2]);
  free(expected[0]);
  free(expected[1]);
  free(expected);
}

void test_cmd_parse(void)
{
  char **rval = cmd_parse("ls -a -l");
  TEST_ASSERT_TRUE(rval);
  TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
  TEST_ASSERT_EQUAL_STRING("-a", rval[1]);
  TEST_ASSERT_EQUAL_STRING("-l", rval[2]);
  TEST_ASSERT_EQUAL_STRING(NULL, rval[3]);
  TEST_ASSERT_FALSE(rval[3]);
  cmd_free(rval);
}

void test_trim_white_no_whitespace(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "ls -a", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("ls -a", rval);
  free(line);
}

void test_trim_white_start_whitespace(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "  ls -a", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("ls -a", rval);
  free(line);
}

void test_trim_white_end_whitespace(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "ls -a  ", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("ls -a", rval);
  free(line);
}

void test_trim_white_both_whitespace_single(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, " ls -a ", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("ls -a", rval);
  free(line);
}

void test_trim_white_both_whitespace_double(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "  ls -a  ", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("ls -a", rval);
  free(line);
}

void test_trim_white_all_whitespace(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "  ", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("", rval);
  free(line);
}

void test_trim_white_mostly_whitespace(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "    a    ", 10);
  char *rval = trim_white(line);
  TEST_ASSERT_EQUAL_STRING("a", rval);
  free(line);
}

void test_get_prompt_default(void)
{
  char *prompt = get_prompt("MY_PROMPT");
  TEST_ASSERT_EQUAL_STRING(prompt, "shell> ");
  free(prompt);
}

void test_get_prompt_custom(void)
{
  const char* prmpt = "MY_PROMPT";
  if(setenv(prmpt,"foo>",true)) {
       TEST_FAIL();
  }

  char *prompt = get_prompt(prmpt);
  TEST_ASSERT_EQUAL_STRING(prompt, "foo>");
  free(prompt);
  unsetenv(prmpt);
}

void test_ch_dir_home(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "cd", 10);
  char **cmd = cmd_parse(line);
  char *expected = getenv("HOME");
  change_dir(cmd);
  char *actual = getcwd(NULL,0);
  TEST_ASSERT_EQUAL_STRING(expected, actual);
  free(line);
  free(actual);
  cmd_free(cmd);
}

void test_ch_dir_root(void)
{
  char *line = (char*) calloc(10, sizeof(char));
  strncpy(line, "cd /", 10);
  char **cmd = cmd_parse(line);
  change_dir(cmd);
  char *actual = getcwd(NULL,0);
  TEST_ASSERT_EQUAL_STRING("/", actual);
  free(line);
  free(actual);
  cmd_free(cmd);
}

/** My Additional Tests */

// Test shell initialization
void test_shell_init() {
  struct shell sh;
  sh_init(&sh);
  TEST_ASSERT_EQUAL(0, sh.shell_terminal);
  TEST_ASSERT_NOT_NULL(&sh.shell_tmodes);
  TEST_ASSERT_NOT_NULL(sh.prompt);
  sh_destroy(&sh);
}

// Test builtin "printhistory" command
void test_builtin_printhistory() {
  struct shell sh;
  sh_init(&sh);
  char *line = (char *)malloc(sizeof(char) * 13);
  strcpy(line, "printhistory");
  char **cmd = cmd_parse(line);
  // check if the command is a built-in command
  TEST_ASSERT_TRUE(do_builtin(&sh, cmd));
  free(line);
  cmd_free(cmd);
  sh_destroy(&sh);
}

// Test command history navigation
void test_command_history_navigation(void) {
  struct shell sh;
  sh_init(&sh);

  // Add commands to the history
  add_history("ls -l");
  add_history("cd /");
  add_history("echo Hello");

  // Simulate navigating the history
  // char *line = (char *)malloc(sizeof(char) * 13);
  // strcpy(line, "printhistory");
  // char **cmd = cmd_parse(line);
  // TEST_ASSERT_TRUE(do_builtin(&sh, cmd));

  // call printhistory function with print_to_stdout = 0

  // Verify the history
  HIST_ENTRY **history_entries = history_list();
  TEST_ASSERT_NOT_NULL(history_entries);
  TEST_ASSERT_EQUAL_STRING("ls -l", history_entries[0]->line);
  TEST_ASSERT_EQUAL_STRING("cd /", history_entries[1]->line);
  TEST_ASSERT_EQUAL_STRING("echo Hello", history_entries[2]->line);
  
  // free(line);
  // cmd_free(cmd);
  sh_destroy(&sh);
}

// Test builtin invalid (non-existent) command
void test_builtin_invalid_cmd() {
  struct shell sh;
  sh_init(&sh);
  char *line = (char *)malloc(sizeof(char) * 11);
  strcpy(line, "invalidcmd");
  char **cmd = cmd_parse(line);
  // check if the command is a built-in command
  TEST_ASSERT_FALSE(do_builtin(&sh, cmd));
  free(line);
  cmd_free(cmd);
  sh_destroy(&sh);
}

// Test signal handling
void test_signal_handling() {
  struct shell sh;
  sh_init(&sh);

  // Simulate sending SIGINT to the shell
  raise(SIGINT);
  // Verify that the shell is still running
  TEST_ASSERT_TRUE(sh.shell_is_interactive);
  // Simulate sending SIGTSTP to the shell
  raise(SIGTSTP);
  // Verify that the shell is still running
  TEST_ASSERT_TRUE(sh.shell_is_interactive);
  sh_destroy(&sh);
}

int main(void) {
  UNITY_BEGIN();

    RUN_TEST(test_cmd_parse);
    RUN_TEST(test_cmd_parse2);
    RUN_TEST(test_trim_white_no_whitespace);
    RUN_TEST(test_trim_white_start_whitespace);
    RUN_TEST(test_trim_white_end_whitespace);
    RUN_TEST(test_trim_white_both_whitespace_single);
    RUN_TEST(test_trim_white_both_whitespace_double);
    RUN_TEST(test_trim_white_all_whitespace);
    RUN_TEST(test_get_prompt_default);
    RUN_TEST(test_get_prompt_custom);
    RUN_TEST(test_ch_dir_home);
    RUN_TEST(test_ch_dir_root);

    // Additional Tests
    RUN_TEST(test_shell_init);
    RUN_TEST(test_builtin_printhistory);
    RUN_TEST(test_command_history_navigation);
    RUN_TEST(test_builtin_invalid_cmd);
    RUN_TEST(test_signal_handling);

  return UNITY_END();
}
