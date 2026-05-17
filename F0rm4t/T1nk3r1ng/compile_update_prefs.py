import paramiko, base64

host = "ecorp.cyber.epitest.eu"
port = 28944
user = "junior-dev"
password = "1337_PAzZW0rd"

src = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* minimal base64 decode */
static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *b64_decode(const char *in) {
    int len = strlen(in);
    char *out = malloc(len);
    int i=0,j=0,v;
    unsigned char buf[4];
    while (i < len) {
        int k;
        for (k=0;k<4&&i<len;k++,i++) {
            char *p = strchr(b64chars, in[i]);
            if (in[i]=='=') buf[k]=0;
            else buf[k] = p ? (p - b64chars) : 0;
        }
        out[j++] = (buf[0]<<2)|(buf[1]>>4);
        if (k>2) out[j++] = (buf[1]<<4)|(buf[2]>>2);
        if (k>3) out[j++] = (buf[2]<<6)|buf[3];
    }
    out[j] = 0;
    return out;
}

char locked_set[64] = "employees";

void print_default(char *per_page) {
    char buf[4815];
    strncpy(buf, per_page, sizeof(buf));
    printf("1|");
    printf(buf);
    printf("|%s", locked_set);
}

int main(int argc, char **argv) {
    char *page, *per_page, *dataset;
    page    = b64_decode(argv[1]);
    per_page = b64_decode(argv[2]);
    dataset = b64_decode(argv[3]);
    if (strcmp(dataset, "employees")) {
        print_default(per_page);
        return 0;
    }
    printf("%s|%s|%s", page, per_page, locked_set);
    return 0;
}
'''

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(host, port=port, username=user, password=password, timeout=15)

sftp = ssh.open_sftp()
with sftp.open('/tmp/update_prefs.c', 'w') as f:
    f.write(src)
sftp.close()

_, out, err = ssh.exec_command(
    'gcc /tmp/update_prefs.c -fno-stack-protector -no-pie -m32 -o /tmp/update_prefs 2>&1'
)
result = out.read().decode() + err.read().decode()
print("Compile:", result.strip() or "OK")

_, out, _ = ssh.exec_command('nm /tmp/update_prefs | grep -E "locked_set|print_default|printf|strcmp|strncpy|b64"')
print("\nnm output:")
print(out.read().decode())

_, out, _ = ssh.exec_command('readelf -r /tmp/update_prefs')
print("\nRELOCATIONS:")
print(out.read().decode())

ssh.close()