#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#define PORT 55555
#define BUF_SIZE 2048

struct data { char *secret; };
int global_fd = 0;

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
  send(fd, "EcorpGPT v0.01 - unstable features, be warned!", 46, 0);
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

void spawn_alpha() {
  print_file(global_fd, "/usr/bin/chatbot/arcadecode");
  pid_t pid = fork();

  if (pid == 0) {
    while (1) {
      pid_t child = fork();

      if (child == 0) {
        execl("/usr/bin/chatbot/super-chatbot-alpha-II-arcade-edition", "super-chatbot-alpha-II-arcade-edition", NULL);
        exit(1);
      }
      waitpid(child, NULL, 0);
      sleep(2);
    }
  }
}

void try_to_hack_me(int fd, char *input) {
  struct data *d1, *d2;
  int pos;

  global_fd = fd;
  d1 = malloc(sizeof(struct data));
  d1->secret = malloc(163);
  d2 = malloc(sizeof(struct data));
  d2->secret = malloc(46);

  for (pos = 16; input[pos] && input[pos] != '$'; ++pos);
  input[pos] = 0;
  strcpy(d1->secret, input + 16);
  strcpy(d2->secret, input + pos + 1);

  puts("LOG: User tried to hack the preprod bot!\n");
  return fake_typing(fd, "Well, at least you tried!");
}

void chatbot(int fd, char *input) {
  char lower[BUF_SIZE];

  strncpy(lower, input, BUF_SIZE);
  to_lower(lower);

  if (contains(input, "let me hack you")) {
    return try_to_hack_me(fd, input);
  }

  if (contains(lower, "source") || contains(lower, "src") || contains(lower, "code")) {
    fake_typing(fd, "Do I really have to? fine...");
    return print_file(fd, "/usr/bin/chatbot/chatbot-preprod.c");
  }

  const char *fallbacks[] = {
    "Don't want to.",
    "This is supposed to be internal only",
    "How are you even here?",
    "Now, if you don't mind, I have other things to do",
    "Can you clarify what you mean? Not that I care, but let's pretend"
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

  printf("EcorpGPT (PREPROD) server running on port %d\n", PORT);

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
        fake_typing(fd, "Goodbye ð Don't come back\n");
        break;
      }

      chatbot(fd, buffer);
    }
    close(fd);
  }

  return 0;
}

// gcc chatbot-preprod.c -fno-pie -no-pie -m32

