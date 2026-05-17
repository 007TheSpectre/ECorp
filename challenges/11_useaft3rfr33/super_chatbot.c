#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#define PORT 54321
#define BUF_SIZE 2048

struct ecorp_auth {
  char name[32];
  int ds_replication_get_changes_all;
};

int rand_choice(int max) {
  return rand() % max;
}

void fake_typing(int fd, const char *text) {
  send(fd, "\n[GPT] > ", 9, 0);
  for (int i = 0; text[i]; ++i) {
    usleep(20000);
    send(fd, text + i, 1, 0);
  }
}

void prompt(int fd) {
  send(fd, "\n\n[YOU] > ", 10, 0);
}

void banner(int fd) {
  send(fd, "EcorpGPT v0.001 - AD middleware - early alpha", 45, 0);
}

void to_lower(char *str) {
  for (int i = 0; str[i]; i++) {
    if (str[i] >= 'A' && str[i] <= 'Z')
      str[i] += 32;
  }
}

int contains(const char *str, const char *sub) {
  return strstr(str, sub) != NULL;
}

void print_file(int fd, const char *file) {
  int fd2 = open(file, O_RDONLY);
  char buf[0x4444];
  int ret;

  ret = read(fd2, buf, sizeof(buf));
  if (fd2 > 0 && ret > 0) {
    buf[ret] = 0;
    fake_typing(fd, buf);
    close(fd2);
    return;
  }
  fake_typing(fd, "I'm sorry, I couldn't read the file for some reasons...");
}

void dc_sync(int fd) {
  fake_typing(fd, "Authorization granted...");
  print_file(fd, "/usr/bin/chatbot/finalcode");
}

void chatbot(int fd, char *input) {
  char lower[BUF_SIZE];
  static struct ecorp_auth *auth = NULL;
  static char *spn = "";

  strncpy(lower, input, BUF_SIZE);
  to_lower(lower);

  if (contains(lower, "whoami")) {
    char response[600];
    snprintf(response, sizeof(response),
      "Username: <%s>, SPN: [%s]", !auth ? "" : auth->name, spn);
    return fake_typing(fd, response);
  }

  if (strncmp(lower, "id ", 3) == 0) {
    auth = malloc(sizeof(struct ecorp_auth));
    memset(auth, 0, sizeof(struct ecorp_auth));
    if (strlen(input + 3) < 31) {
      strcpy(auth->name, input + 3);
    }
    return fake_typing(fd, "username updated!");
  }

  if (contains(lower, "clear")) {
    free(auth);
    return fake_typing(fd, "username cleared!");
  }

  if (strncmp(lower, "spn ", 4) == 0) {
    spn = strdup(input + 4);
    return fake_typing(fd, "Service Principal Name (SPN) updated!");
  }

  if (contains(lower, "login")) {
    if (auth && auth->ds_replication_get_changes_all) {
      return dc_sync(fd);
    }
    return fake_typing(fd, "RemoteOperations failed: DCERPC Runtime Error: code: 0x5 - rpc_s_access_denied");
  }

  if (contains(lower, "source") || contains(lower, "src") || contains(lower, "code")) {
    fake_typing(fd, "// Do not distribute");
    print_file(fd, "/usr/bin/chatbot/super-chatbot-alpha-II-arcade-edition.c");
    fake_typing(fd, "// Do not distribute");
    return;
  }

  const char *fallbacks[] = {
    "This isn't a valid command",
    "Command not found",
    "After a long time processing, the command is still not found",
    "Nope, I don't know that command",
    "This bot is limited, please only input valid commands"
  };

  return fake_typing(fd, fallbacks[rand_choice(5)]);
}

int main() {
  int server_fd, fd;
  struct sockaddr_in addr;
  char buffer[BUF_SIZE];
  struct timeval tv = {30, 0};
  int opt = 1;

  srand(time(NULL));

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
  listen(server_fd, 5);

  printf("EcorpGPT (AD Middleware) server running on port %d\n", PORT);

  while (1) {
    fd = accept(server_fd, NULL, NULL);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    banner(fd);
    while (2) {
      prompt(fd);
      memset(buffer, 0, BUF_SIZE);
      int n = recv(fd, buffer, BUF_SIZE - 1, 0);
      if (n <= 0)
        break;
      buffer[strcspn(buffer, "\r\n")] = 0;

      if (!strcmp(buffer, "exit")) {
          fake_typing(fd, "[EXIT]\n");
          break;
      }

      chatbot(fd, buffer);
    }
    close(fd);
  }

  return 0;
}

// gcc super-chatbot-alpha-II-arcade-edition.c
[GPT] > // Do not distribute
