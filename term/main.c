#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#define LS "ls"
#define CAT "cat"
#define NICE "nice"
#define KILLALL "killall"

static volatile bool keep_running = true;

void handle_interrupt(int signal) {
  keep_running = false;
  printf("Interrupted...\n");
  exit(0);
}

int main(void) {
  printf("term init\n");

  signal(SIGINT, handle_interrupt);

  while (keep_running) {
    size_t line_cap = 1024;
    char *command = (char *) malloc(sizeof(char) * line_cap);
    printf("%d, Enter avaliable command [ls, cat, nice, killall]\n", keep_running);
    getline(&command, &line_cap, stdin);
    printf("command: %s\n", command);
    system(command);
  }
}
