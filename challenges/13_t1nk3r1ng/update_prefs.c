#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal b64_decode stub for reference — real binary on remote has full implementation
// gcc update-preferences.c -fno-stack-protector -no-pie -m32

char locked_set[64] = "employees";

void print_default(char *per_page) {
    char buf[4815];
    strncpy(buf, per_page, sizeof(buf));
    printf("1|");
    printf(buf);    // FORMAT STRING VULNERABILITY
    printf("|%s", locked_set);
}

int main(int argc, char **argv) {
    // placeholder
    return 0;
}
