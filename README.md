<div align="center">

# Unfold

**Binary exploitation Proof-of-Concepts — memory corruption, from the stack to the heap and format strings.**

*Epitech · G-SEC-401 · PGE2 2025*

[🇬🇧 English](#-english) · [🇫🇷 Français](#-français)

</div>

---

## 🇬🇧 English

### Overview

**Unfold** is a security project built around a fictional company, **ECorp**. The goal was to
audit a set of deliberately vulnerable C binaries and network services, identify the
memory-corruption flaws they contain, and exploit them to recover the associated flags.

The project gathers **11 Proof-of-Concepts (PoCs)** organised into three families:

| Family | Focus | Challenges |
| --- | --- | --- |
| **Stack** | Stack-based buffer overflows, return-address control, ROP | 6 |
| **Heap** | Heap overflows, function-pointer corruption, use-after-free | 3 |
| **Format** | Format-string leaks and arbitrary writes (`%n` / `%hhn`) | 2 |

Every PoC is self-contained in its own folder and comes with a dedicated `README.md` describing:

- the challenge and the target service/binary,
- the vulnerability analysis (memory layout, offsets, offsets on the stack/heap),
- a walkthrough of the exploit script,
- the expected output and the recovered flag,
- remediation advice to fix the flaw.

### Techniques covered

**Stack**

| Challenge | Target | Root cause | Technique |
| --- | --- | --- | --- |
| `Ov3rfl0w` | `access-checker` | `gets()` overflow + weak `strncmp` check | Overwrite the `failsafe` flag to disable the security |
| `Ov3rwr1t3` | `database-reader` | `gets()` overflow | Overwrite the adjacent `debug_level` variable with `debug` |
| `Envir0n` | `forgot-password` | `strcpy()` of the `DEHASH` env var | Overflow `hash[]` into the adjacent `role` → `management` |
| `C4llb4ck` | `tool-handler` (SUID) | `gets()` overflow | Overwrite a local function pointer → hidden `back2back()` |
| `R3t2func` | `read-pub-ssh` | `read()` overflow (144-byte buffer) | Return-to-function into `read_ssh()` via symlink abuse |
| `St4ck0v3rfl0w` | `tracking-reader` | `gets()` overflow | `ret2libc` ROP chain with ASLR disabled (`setarch -R`) |

**Heap**

| Challenge | Target | Root cause | Technique |
| --- | --- | --- | --- |
| `H34pfun` | Chatbot (port 44444) | `strcpy()` heap overflow | Overwrite the adjacent function pointer → `protected_area()` |
| `H34pG0t` | Chatbot-preprod (port 55555) | `strcpy()` heap overflow | Corrupt `d2->secret` pointer, then rewrite `puts@GOT` → `spawn_alpha()` |
| `UseAft3rFr33` | Super-chatbot (port 54321) | Use-after-free (dangling pointer) | Reallocate the freed struct with `strdup()` and set the privilege flag |

**Format string**

| Challenge | Target | Root cause | Technique |
| --- | --- | --- | --- |
| `Ov3rf0rm4t` | `cgi-login` | `sprintf()` with a user-controlled format | `%32c` overflow overwrites the local `role` → `intrauser` |
| `T1nk3r1ng` | `update-preferences` | `printf(buf)` with no fixed format | libc leak + byte-by-byte `%hhn` write into `exit@GOT` → `system()` |

### Repository structure

```
unfold/
├── St4ck/                  # Stack-based exploitation
│   ├── Ov3rfl0w/           #   exploit_access-checker.py
│   ├── Ov3rwr1t3/          #   exploit_database-reader.py
│   ├── Envir0n/            #   exploit_forgot-password.py
│   ├── C4llb4ck/           #   exploit_Callback.py
│   ├── R3t2func/           #   exploit_Ret2func.py
│   └── St4ck0v3rfl0w/      #   exploit_st4ck0v3rfl0w.py
├── H34p/                   # Heap-based exploitation
│   ├── H34pfun/            #   exploit_chatbot.py
│   ├── H34pG0t/            #   exploit_heapgot.py
│   └── UseAft3rFr33/       #   exploit_uaf.py
├── F0rm4t/                 # Format-string exploitation
│   ├── Ov3rf0rm4t/         #   exploit_ovr3f0rm4t.py
│   └── T1nk3r1ng/          #   compile_update_prefs.py
└── README.md
```

Each challenge folder holds the exploit script (`*.py`) along with its own `README.md`.

### Tooling

The exploits are written in **Python 3** and rely on:

- [`pwntools`](https://github.com/Gallopsled/pwntools) — process/socket interaction and payload building;
- [`paramiko`](https://www.paramiko.org/) — SSH transport to reach the target environment;
- the standard library — `socket`, `struct`, `pty`, `tty`, `termios`, `select`, `base64`.

### Usage

Every PoC is independent. Read the `README.md` inside the relevant folder for the exact
context, prerequisites and expected output, then run the script:

```bash
python3 St4ck/Ov3rfl0w/exploit_access-checker.py
```

> The scripts target the original challenge infrastructure (hosts, users and ports of the
> ECorp environment) and are kept as-is for documentation purposes. They are not meant to be
> run against live systems.

### Disclaimer

This project is published **for educational and documentation purposes only**. All the flaws
described here belong to intentionally vulnerable binaries and services from a pedagogical
security exercise. Do not use these techniques on systems you do not own or have explicit
permission to test. The authors disclaim any responsibility for misuse.

---

## 🇫🇷 Français

### Présentation

**Unfold** est un projet de sécurité construit autour d'une entreprise fictive, **ECorp**.
L'objectif était d'auditer un ensemble de binaires C et de services réseau volontairement
vulnérables, d'identifier les failles de corruption mémoire qu'ils contiennent, puis de les
exploiter afin de récupérer les flags associés.

Le projet regroupe **11 preuves de concept (PoC)** réparties en trois familles :

| Famille | Thème | Challenges |
| --- | --- | --- |
| **Stack** | Débordements de pile, contrôle de l'adresse de retour, ROP | 6 |
| **Heap** | Débordements sur le tas, corruption de pointeurs de fonction, use-after-free | 3 |
| **Format** | Fuites de chaîne de format et écritures arbitraires (`%n` / `%hhn`) | 2 |

Chaque PoC est autonome dans son propre dossier et est accompagné d'un `README.md` dédié qui
décrit :

- le challenge et le service/binaire ciblé,
- l'analyse de la vulnérabilité (agencement mémoire, offsets sur la pile/le tas),
- une description du script d'exploitation,
- le résultat attendu et le flag récupéré,
- une remédiation pour corriger la faille.

### Techniques abordées

**Stack**

| Challenge | Cible | Cause racine | Technique |
| --- | --- | --- | --- |
| `Ov3rfl0w` | `access-checker` | Débordement `gets()` + vérification `strncmp` trop faible | Écrasement du flag `failsafe` pour désactiver la sécurité |
| `Ov3rwr1t3` | `database-reader` | Débordement `gets()` | Écrasement de la variable adjacente `debug_level` par `debug` |
| `Envir0n` | `forgot-password` | `strcpy()` de la variable d'env. `DEHASH` | Débordement de `hash[]` dans `role` adjacent → `management` |
| `C4llb4ck` | `tool-handler` (SUID) | Débordement `gets()` | Écrasement d'un pointeur de fonction local → `back2back()` cachée |
| `R3t2func` | `read-pub-ssh` | Débordement `read()` (tampon de 144 octets) | Return-to-function vers `read_ssh()` via abus de lien symbolique |
| `St4ck0v3rfl0w` | `tracking-reader` | Débordement `gets()` | Chaîne ROP `ret2libc` avec ASLR désactivée (`setarch -R`) |

**Heap**

| Challenge | Cible | Cause racine | Technique |
| --- | --- | --- | --- |
| `H34pfun` | Chatbot (port 44444) | Débordement `strcpy()` sur le tas | Écrasement du pointeur de fonction adjacent → `protected_area()` |
| `H34pG0t` | Chatbot-preprod (port 55555) | Débordement `strcpy()` sur le tas | Corruption du pointeur `d2->secret`, puis réécriture de `puts@GOT` → `spawn_alpha()` |
| `UseAft3rFr33` | Super-chatbot (port 54321) | Use-after-free (pointeur suspendu) | Réallocation de la structure libérée via `strdup()` et activation du flag de privilège |

**Chaîne de format**

| Challenge | Cible | Cause racine | Technique |
| --- | --- | --- | --- |
| `Ov3rf0rm4t` | `cgi-login` | `sprintf()` avec une chaîne de format contrôlée | Débordement `%32c` écrasant le `role` local → `intrauser` |
| `T1nk3r1ng` | `update-preferences` | `printf(buf)` sans format fixe | Fuite de la libc + écriture octet par octet via `%hhn` dans `exit@GOT` → `system()` |

### Arborescence du dépôt

```
unfold/
├── St4ck/                  # Exploitation par débordement de pile
│   ├── Ov3rfl0w/           #   exploit_access-checker.py
│   ├── Ov3rwr1t3/          #   exploit_database-reader.py
│   ├── Envir0n/            #   exploit_forgot-password.py
│   ├── C4llb4ck/           #   exploit_Callback.py
│   ├── R3t2func/           #   exploit_Ret2func.py
│   └── St4ck0v3rfl0w/      #   exploit_st4ck0v3rfl0w.py
├── H34p/                   # Exploitation par débordement de tas
│   ├── H34pfun/            #   exploit_chatbot.py
│   ├── H34pG0t/            #   exploit_heapgot.py
│   └── UseAft3rFr33/       #   exploit_uaf.py
├── F0rm4t/                 # Exploitation par chaîne de format
│   ├── Ov3rf0rm4t/         #   exploit_ovr3f0rm4t.py
│   └── T1nk3r1ng/          #   compile_update_prefs.py
└── README.md
```

Chaque dossier de challenge contient le script d'exploitation (`*.py`) ainsi que son propre
`README.md`.

### Outils

Les exploits sont écrits en **Python 3** et s'appuient sur :

- [`pwntools`](https://github.com/Gallopsled/pwntools) — interaction avec les processus/sockets et construction des payloads ;
- [`paramiko`](https://www.paramiko.org/) — transport SSH pour atteindre l'environnement cible ;
- la bibliothèque standard — `socket`, `struct`, `pty`, `tty`, `termios`, `select`, `base64`.

### Utilisation

Chaque PoC est indépendant. Reportez-vous au `README.md` du dossier concerné pour le contexte
exact, les prérequis et le résultat attendu, puis lancez le script :

```bash
python3 St4ck/Ov3rfl0w/exploit_access-checker.py
```

> Les scripts ciblent l'infrastructure d'origine des challenges (hôtes, utilisateurs et ports
> de l'environnement ECorp) et sont conservés tels quels à des fins de documentation. Ils ne
> sont pas destinés à être exécutés contre des systèmes réels.

### Avertissement

Ce projet est publié **à des fins éducatives et de documentation uniquement**. Toutes les
failles décrites ici appartiennent à des binaires et services volontairement vulnérables
issus d'un exercice pédagogique de sécurité. N'utilisez pas ces techniques sur des systèmes
que vous ne possédez pas ou pour lesquels vous ne disposez pas d'une autorisation explicite.
Les auteurs déclinent toute responsabilité en cas de mauvaise utilisation.
