#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#define PORT 44444
#define BUF_SIZE 1024

struct credentials { char password[48]; };
struct callback { void (*fun)(int); };

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
  send(fd, "EcorpGPT v0.1 - type 'exit' to quit", 35, 0);
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

void spawn_preprod() {
    pid_t pid = fork();

    if (pid == 0) {
        while (1) {
            pid_t child = fork();

            if (child == 0) {
                execl("/usr/bin/chatbot/chatbot-preprod", "chatbot-preprod", NULL);
                exit(1);
            }
            waitpid(child, NULL, 0);
            sleep(2);
        }
    }
}

void protected_area(int fd) {
  fake_typing(fd, "Welcome senior-dev, loading restricted functions...");
  print_file(fd, "/usr/bin/chatbot/specialcode");
  spawn_preprod();
  fake_typing(fd, "Preprod chatbot deployed successfully.");
}

void unallowed_access(int fd) {
  fake_typing(fd, "Ã¢ÂÂ Ã¯Â¸Â Sorry, you don't have the clearance to access the protected area Ã¢ÂÂ Ã¯Â¸Â");
}

void verify_access(int fd, char *lower) {
  struct credentials *d;
  struct callback    *f;

  d = malloc(sizeof(struct credentials));
  f = malloc(sizeof(struct callback));
  f->fun = unallowed_access;

  strcpy(d->password, lower + 22);
  /*
  // protected area disabled until further notice
  if (!strcmp(d->password, "Password123!"))
    f->fun = protected_area;
  */

  f->fun(fd);
}

int check_lock(int fd, char *input) {
  int fd2 = open("/usr/bin/chatbot/lockcode", O_RDONLY);
  char buf[0x4444];
  int ret;

  ret = read(fd2, buf, sizeof(buf));
  if (fd2 > 0 && ret > 0) {
    buf[ret] = 0;
    if (!strcmp(buf, input)) {
      fake_typing(fd, "Hello junior-dev, thank you for unlocking me, how can I help you?");
      close(fd2);
      return 0;
    }
    close(fd2);
  }

  fake_typing(fd, "I'm sorry, I really wish to help you but I'm locked right now. You need to give me the correct token.");
  return 1;
}

void chatbot(int fd, char *input) {
  char lower[BUF_SIZE];
  static int lock = 1;

  if (lock) {
    lock = check_lock(fd, input);
    return;
  }
  strncpy(lower, input, BUF_SIZE);
  to_lower(lower);

  if (contains(input, "access protected area")) {
    return verify_access(fd, input);
  }

  if (contains(lower, "source") || contains(lower, "src") || contains(lower, "code")) {
    fake_typing(fd, "For sure! Here is my source code:");
    return print_file(fd, "/usr/bin/chatbot/chatbot.c");
  }

  if (contains(lower, "buffer") || contains(lower, "overflow"))
    return fake_typing(fd, "A buffer overflow happens when data exceeds a buffer's allocated size, potentially overwriting memory.");

  if (contains(lower, "xss"))
    return fake_typing(fd, "Cross-site scripting allows injecting malicious scripts into web pages.");

  if (contains(lower, "sql injection") || contains(lower, "sqli"))
    return fake_typing(fd, "SQL injection is a vulnerability where an attacker can manipulate database queries.");

  if (contains(lower, "hello") || contains(lower, "hi") || contains(lower, "hey") || contains(lower, "bonjour")) {
    const char *responses[] = {
      "Hello Ã°ÂÂÂ How can I help you today?",
      "Hi there! What are you working on?",
      "Howdy!",
      "Bien le bonjour, or so they say in french!"
    };
    return fake_typing(fd, responses[rand_choice(4)]);
  }

  if (contains(lower, "how are you")) {
    const char *responses[] = {
      "I'm just a program, but I'm functioning as expected Ã°ÂÂÂ",
      "All systems operational.",
      "How are YOU? Not tired of trying to break into my code? Don't think I didn't see you..."
    };
    return fake_typing(fd, responses[rand_choice(3)]);
  }

  if (strlen(input) > 20) {
    char response[600];
    snprintf(response, sizeof(response),
      "It sounds like you're dealing with \"%s\", I'm afraid I can't help with that.", input);
    return fake_typing(fd, response);
  }

  const char *fallbacks[] = {
    "That depends on the context I guess",
    "There are a few ways to approach this, none of which I know!",
    "I didn't get that, can you reformulate?",
    "Wow that was a lot of words, I didn't understand a single one of those though...",
    "Can you clarify what you mean?"
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

  printf("EcorpGPT server running on port %d\n", PORT);

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
          fake_typing(fd, "Goodbye Ã°ÂÂÂ\n");
          break;
      }

      if ((rand() % 100) < 40) {
        const char *thinking[] = {
          "Hmm, let me think about that...",
          "Interesting question.",
          "Let me analyze this for a second...",
        };
        fake_typing(fd, thinking[rand_choice(3)]);
        sleep(1);
      }

      chatbot(fd, buffer);
    }
    close(fd);
  }

  return 0;
}

// gcc chatbot.c -fno-pie -no-pie -m32