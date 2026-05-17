#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void kill_switch() {
  int fd;

  printf("\nKill switch activated. All MFA's are disabled!\n");
  fd = open("/var/www/devops/data/status", O_WRONLY);
  write(fd, "-,-,0,-,0", 9);
  close(fd);
}

int status() {
  int fd;
  int ret;
  char buffer[512];

  fd = open("/var/www/devops/data/status", O_RDONLY);
  if (fd != -1) {
    ret = read(fd, buffer, sizeof(buffer));
    if (ret != -1) {
      buffer[ret] = 0;
      printf("%s", buffer);
    }
  }
  close(fd);
  return 0;
}

int toggle_mfa(char command[2142]) {
  int fd;
  int id;
  char val;
  int ret;
  char buffer[512];

  if (strlen(command) < 10)
    return status();
  for (int i = 0; i < 2142; ++i)
    if (command[i] == ',')
      return status();
  fd = open("/var/www/devops/data/status", O_RDWR);
  ret = read(fd, buffer, sizeof(buffer));
  buffer[ret] = 0;
  id = atoi(command + 7);
  if (id < 0 || id > 4)
    return status();
  val = command[9];
  val = val == '1' ? '1' : '0';
  lseek(fd, id * 2, SEEK_SET);
  write(fd, &val, 1);
  close(fd);
  printf("%.*s", id*2, buffer);
  printf(command + 9);
  printf("%s", buffer + id * 2 + 1);
  exit(1337);
}

int main(int argc, char **argv) {
  char command[2142] = {0};
  int ret;

  ret = read(0, command, sizeof(command) -1);
  if (ret && command[ret-1] == '\n')
    command[ret-1] = 0;
  if (!strcmp(command, "status"))
    return status();
  else if (!strncmp(command, "toggle", 6))
    toggle_mfa(command);
  return 0;
}

// gcc toggle-mfa.c -fno-stack-protector -no-pie -m32