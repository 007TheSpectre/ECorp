
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *gets(char *s);

int check_access(char *username) {
  FILE *fd;
  char *line = malloc(256);
  size_t n = 255;
  int ret = 1;

  fd = fopen("/etc/passwd", "r");

  printf("Scanning /etc/passwd...\n");
  while (ret > 0) {
    ret = getline(&line, &n, fd);
    if (ret)
      line[ret - 1] = 0;
    for (int i = 1; i < 10; ++i) // usernames shouldn't be longer anyway
      if (!strncmp(line, username, i)) {
        printf("Valid username '%s' found\n", username);
        return 1;
      }
  }
  printf("! No valid username found !\n");
}

void data_exfiltration() {
  char *url = "http://internal.ecorp.epi";
  char *api_key = "EPI{F4k3_Fl4g_D0_1t_L1v3_N0w}";

  printf("Failsafe disabled, sending data to our internal website...\n");
  printf("=> calling %s?api_key=%s\n", url, api_key);
  system("curl http://internal.ecorp.epi/?api_key=EPI%7BF4k3_Fl4g_D0_1t_L1v3_N0w%7D");
}

int main(int argc, char **argv) {
  char failsafe[64] = "Locked"; // locked until further notice to prevent CNIL controls
  char username[64];

  printf("ECorp access checker, please enter your username: ");
  fflush(stdout);
  gets(username);

  if (check_access(username) == 1 && strcmp(failsafe, "Locked"))
    data_exfiltration();
  else
    puts("Thank you for using ECorp access checker!\n");

  return 0;
}

// gcc access-checker.c