
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *gets(char *s);

struct str_to_prog {
  char *name;
  void (*callback)();
};

void back2back() {
  system("cat /mnt/dev/srcs.zip | base64");
}

void check_it_up() {
  system("/opt/intern/access-checker");
}

void data_based() {
  system("/opt/intern/database-reader");
}

void i_forgot() {
  system("/opt/intern/forgot-password");
}

void c_tchao() {
  exit(EXIT_SUCCESS);
}

void not_found() {
  printf("Program not found...\n");
}

int main(int argc, char **argv) {
  struct str_to_prog valid[] = {
    {.name = "access-checker",  .callback = check_it_up},
    {.name = "database-reader", .callback = data_based},
    {.name = "forgot-password", .callback = i_forgot},
    {.name = "exit",            .callback = c_tchao}/*,
    {.name = "backup-man",      .callback = back2back}*/
  };
  struct {
    char cool_prog[234];
    void (*cb)();
  } locals = {
    .cool_prog = {0},
    .cb = &not_found
  };

  setreuid(1002, 1002);
  printf("_.-~* ECorp super duper caller mkIII (copyright junior-dev) *~-._\n\n");

  while (0x1337) {
    locals.cb = &not_found;
    printf("$[Enter program] > ");
    fflush(stdout);
    gets(locals.cool_prog);
    for (int i = 0; i < 4; ++i)
      if (!strncmp(locals.cool_prog, valid[i].name, strlen(valid[i].name)))
        locals.cb = valid[i].callback;
    locals.cb();
  }
  return 0;
}

// gcc tool-handler.c -fno-pie -no-pie