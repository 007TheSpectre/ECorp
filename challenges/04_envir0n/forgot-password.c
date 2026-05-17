
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void dehash(const char *hash) {
  const char* rainbow[5][2] = {
    {"327a6c4304ad5938eaf0efb6cc3e53dc", "EPI{F4k3_Fl4g_D0_1t_L1v3_N0w}"},
    {"f02368945726d5fc2a14eb576f7276c0", "bonjour"},
    {"60d3445af1ae382994cec66b619bd2d8", "<REDACTED>"},
    {"d78b6f30225cdc811adfe8d4e7c9fd34", "hack"},
    {"7007296521223107d3445ea0db5a04f9", "ecorp"}
  };

  printf("Dehashing your hash...\n");
  for (int i = 0; i < 5; ++i) {
    if (!strncmp(hash, rainbow[i][0], 32)) {
      printf("=> %s\n", rainbow[i][1]);
      return;
    }
  }
  printf("Hash was not found in the database!\n");
}

int main(int argc, char **argv) {
  struct {
    char hash[151];
    char role[64];
  } locals = {
    .hash = {0},
    .role = "user"
  };

  printf("ECorp password reminder app (BETA)\n\n");

  printf("First service to dehash a hash!\n");
  printf("Provide a MD5 hash in DEHASH env variable\n");

  char *ptr = getenv("DEHASH");

  if (ptr == NULL) {
    printf("No DEHASH provided!\n");
    return 1;
  }
  
  strcpy(locals.hash, ptr);

  if (!strcmp(locals.role, "management")) // disabled by management once they found out about this app
    dehash(locals.hash);
  else
    printf("This program has been disabled by order of the management.\n");

  return 0;
}

// gcc forgot-password.c