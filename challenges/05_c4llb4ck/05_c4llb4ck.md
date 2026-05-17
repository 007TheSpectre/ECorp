# PoC — Challenge 05: C4llb4ck

**Flag:** `EPI{1_H0P3_Y0uLL_g1v3_m3_4_C4Ll84ck}`
**Vulnerability:** Function pointer overwrite via `gets()` — redirect execution to `back2back()`
**Binary:** `/mnt/dev/tool-handler` (SUID junior-dev, 64-bit non-PIE)
**Access needed:** SSH as `guest`

---

## Vulnerability

`main()` stores a local struct with a function pointer adjacent to the input buffer:

```c
struct {
    char cool_prog[234];
    void (*cb)();
} locals = {
    .cool_prog = {0},
    .cb = &not_found
};
// ...
gets(locals.cool_prog);  // no bounds check
// ...
locals.cb();             // calls whatever address is in cb
```

Overflowing `cool_prog[234]` overwrites `cb` with an arbitrary address. The hidden function `back2back()` (at a fixed address since the binary is non-PIE) dumps `srcs.zip` in base64.

**Target function:** `back2back @ 0x4011a6`  
*(contains: `system("cat /mnt/dev/srcs.zip | base64")`)*

---

## Exploit

→ `scripts/exploit_c4llb4ck.py <SSH_PORT>`

Saves `/tmp/srcs.zip`. Then extract:

```bash
unzip -P 'Th1s_is_one_hell_of_a_P4ssw0rd' /tmp/srcs.zip -d /tmp/srcs_extract
cat /tmp/srcs_extract/secret-manual.txt
```

`secret-manual.txt` contains the flag and junior-dev credentials.

**Flag:** `EPI{1_H0P3_Y0uLL_g1v3_m3_4_C4Ll84ck}`
