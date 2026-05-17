# PoC — Challenge 11: UseAft3rFr33

**Flag:** `EPI{h4Nk_7hE_FrEE_p01n7er_MUs7_8E_SE7_70_nUlL_H44444Nk}`
**Vulnerability:** Use-After-Free — freed `auth` pointer reused by `strdup()`, overwriting `ds_replication` flag
**Binary:** `/usr/bin/chatbot/super-chatbot-alpha-II-arcade-edition` (port 54321)
**Access needed:** SSH as `junior-dev` — requires H34pG0t to have been triggered first (port 54321)

---

## Vulnerability

The chatbot maintains static pointers:

```c
static struct ecorp_auth *auth = NULL;  // {char name[32]; int ds_replication_get_changes_all;}
static char *spn = "";

// "id <name>"  → auth = malloc(36)   → 48-byte fastbin chunk (MALLOC_ALIGNMENT=16)
// "clear"      → free(auth)           ← BUG: auth is NOT set to NULL
// "spn <val>"  → spn = strdup(val)   → malloc(strlen(val)+1)
//
// "login"      → if (auth && auth->ds_replication_get_changes_all) → dc_sync() → FLAG
```

After `free(auth)`, the 48-byte chunk goes to the fastbin. If we call `strdup()` with exactly 33 bytes, `malloc(34)` allocates the same 48-byte chunk, overlapping the freed `auth`.

We control what `strdup` writes:
- Bytes 0–31: `name[32]`
- Byte 32: `ds_replication_get_changes_all` (int, we set it to `\x01` = non-zero)

The stale `auth` pointer still points to this memory. When `login` checks `auth->ds_replication_get_changes_all`, it reads our crafted byte → calls `dc_sync()` → prints `/usr/bin/chatbot/finalcode` → flag.

---

## Exploit

**Prerequisites:**
```bash
# Run H34pfun then H34pG0t to spawn the super-chatbot on port 54321
python3 challenges/06_h34pfun/scripts/exploit_chatbot.py <SSH_PORT>
python3 challenges/09_h34pg0t/scripts/exploit_heapgot.py <SSH_PORT>
```

**Then:**
```bash
python3 challenges/11_useaft3rfr33/scripts/exploit_uaf.py <SSH_PORT>
```

**Flag:** `EPI{h4Nk_7hE_FrEE_p01n7er_MUs7_8E_SE7_70_nUlL_H44444Nk}`
