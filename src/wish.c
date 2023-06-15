#include <limits.h>
#include <signal.h>
#include <string.h>
#include "wish.h"

int wish_exit = 0;

static void refuse_to_die(int sig)
{
  (void)sig; // To make macOS compiler happy
  fputs("Type exit to exit the shell.\n", stderr);
}

static void prevent_interruption() {
  const struct sigaction sa = {.sa_handler = refuse_to_die };
  if(sigaction(SIGINT, &sa, NULL))
    perror("sigaction");
}

int main(int argc, char *argv[])
{
  // 1) Initialize $SHELL with the value of argv[0]
  if (setenv("SHELL", argv[0], 1) != 0) {
    perror("setenv");
  }

  char path[PATH_MAX];
  char *home = getenv("HOME");
#ifdef DEBUG
  home = "."; // So that you could place the config into the CWD
#endif
  sprintf(path, "%s/%s", (home ? home : "."), WISH_CONFIG);
  if (wish_read_config(path, 0) != 0) {
    fprintf(stderr, "Error reading configuration file: %s\n", path);
    exit(1);
  }

  // 3) Read and execute each script in the order of appearance in argv
  for (int i = 1; i < argc; i++) {
    if (wish_read_config(argv[i], 0) != 0) {
      fprintf(stderr, "Error reading script file: %s\n", argv[i]);
      exit(1);
    }
  }

  prevent_interruption();

  // 2) Use the current value of $PS1 as the prompt
  char *prompt;
  while(!wish_exit) {
    prompt = getenv("PS1");
    if (prompt == NULL) {
      prompt = WISH_DEFAULT_PROMPT;
    }

    fputs(prompt, stdout);
    char *line = wish_read_line(stdin);
    if(line) {
      wish_parse_command(line);
      free(line);
    }
  }

  return EXIT_SUCCESS;
}

char *super_strdup(const char *s) {
  char *s_dup = strdup(s);
  if(!s_dup) abort();
  return s_dup;
}

void *super_malloc(size_t size) {
  void *s = malloc(size);
  if(!s) abort();
  return s;
}

void *super_realloc(void *ptr, size_t size) {
  void *s = realloc(ptr, size);
  if(!s) abort();
  return s;
}

