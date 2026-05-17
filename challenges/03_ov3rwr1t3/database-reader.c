
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *gets(char *s);

void decode(const char *cipher, const char *key) {
  char *out = calloc(1, 132);
  int out_len = strlen(cipher) / 2;
  int key_len = strlen(key);

  for (int i = 0; cipher[i]; ++i)
    sscanf(cipher + 2 * i, "%2hhx", out + i);

  for (int i = 0; i < out_len; i++) {
    out[i] ^= key[i % key_len];
  }

  printf("Deciphering the database...\n\n");
  printf("%s\n", out);
  free(out);
}

int main(int argc, char **argv) {
  const char *cipher =
      "63475005061d000d1e48300a304918053a19006445447e5e5f593304004f10010c06"
      "1f4d3c0e35010b00664f54725f0a4760464c280b410e16405150555c600c230a5852"
      "2d43457d0d0a46304a73644d230a10010c165645113f0f12190c162a180d0227200b"
      "3d0c3b58150325165c3d182a0b0e735c075d0130101b190f3801370334";
  const char *key = "<REDACTED>";
  struct {
    char password[48]; // password will be replaced in production to prevent unwanted access
    char debug_level[5];
  } locals = {
    .password = {0},
    .debug_level = "guest"
  };

  printf("ECorp database reader v0.2.23\n\n");

  printf("To access the database, please provide the admin password: ");
  fflush(stdout);
  gets(locals.password);

  if (!strcmp(locals.password, "<REDACTED2>") || !strcmp(locals.debug_level, "debug"))
    decode(cipher, key);
  else
    printf("Wrong password, access denied.\n");

  return 0;
}

// gcc database-reader.c