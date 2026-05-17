# PoC - Exploitation d'un Buffer Overflow (ECorp Chatbot)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de détourner le flux d'exécution de l'application EcorpGPT (Chatbot) d'ECorp hébergée sur le port local 44444. En exploitant un débordement de tampon sur le tas (Heap Overflow), il est possible d'écraser un pointeur de fonction adjacent afin de rediriger l'exécution vers une fonction restreinte (protected_area) et de forcer l'ouverture du service de préproduction (obtention du flag).

## Analyse de la Vulnerabilité
L'application présente une vulnérabilité de corruption de mémoire dynamique due à l'allocation consécutive de deux structures sur le tas au sein de la fonction verify_access() :


struct credentials { char password[48]; };
struct callback { void (*fun)(int); };

struct credentials *d = malloc(sizeof(*d));  // Allocation de 48 octets
struct callback    *f = malloc(sizeof(*f));  // Allocation du pointeur de fonction
f->fun = unallowed_access;
Buffer Overflow : Le programme utilise la fonction non sécurisée strcpy() pour copier une sous-chaîne de l'entrée utilisateur directement dans le tampon d->password. Aucun contrôle de limites n'est opéré sur la longueur de la chaîne copiée.

Écrasement d'une variable adjacente : L'application s'exécute sur une architecture 32-bit avec un alignement de mémoire imposé par MALLOC_ALIGNMENT=16. Par conséquent, l'allocation de 48 octets pour la structure credentials réserve un bloc mémoire effectif de 64 octets. La structure callback est allouée immédiatement après ce bloc. Le pointeur de fonction f->fun se situe donc à une distance exacte de 64 octets du point de départ d->password[0].

Contournement de la logique (Bypass) : Bien que le binaire soit de type non-PIE, il subit une recompilation automatique à chaque relancement du serveur, ce qui modifie l'emplacement de ses fonctions. L'adresse de la fonction cible protected_area() reste cependant fixe durant toute la session d'exécution. En envoyant la phrase clé "access protected area " suivie d'un rembourrage totalisant 64 octets et de l'adresse dynamique de protected_area(), l'appel légitime f->fun(fd); exécute le code privilégié.

## Le Script de PoC (exploit_chatbot.py)
Le script gère la connexion SSH, interagit avec le système distant pour compiler le code source témoin et en extraire l'adresse mémoire actuelle de la fonction protected_area(). Il s'authentifie ensuite auprès du bot avec un jeton valide, puis injecte la charge utile sur le port réseau local 44444 via une socket Python déportée.


## Résultat Attendu
Une fois le pointeur de fonction détourné vers la fonction protected_area(), le serveur distant valide l'accès, affiche le flag de l'exercice et initialise le nouveau service réseau de préproduction :


[+] SSH connected
[+] Found protected_area address: 0x80496d0
[+] Exploit payload sent to local port 44444

[OUTPUT] (145 bytes):
EcorpGPT v0.1 - type 'exit' to quit
[YOU] > 
[YOU] > 
[GPT] > Access granted. Flag: EPI{H34p_1S_4_M0Ns73r_0n_17S_0wN}
Spawning preprod chatbot...


Flag récupéré : EPI{H34p_1S_4_M0Ns73r_0n_17S_0wN}

## Remédiation
Pour sécuriser l'application, appliquez les corrections suivantes :

- Bannir strcpy() : Remplacez l'affectation non contrôlée par l'usage exclusif de fonctions limitant explicitement le volume de transfert mémoire, par exemple : strncpy(d->password, lower + 22, sizeof(d->password) - 1); en veillant à clore manuellement la chaîne avec un octet nul.

- Durcissement de l'agencement mémoire : Évitez d'imbriquer de manière contiguë des tampons d'écriture utilisateur et des structures contenant des pointeurs d'exécution réutilisables au sein du tas.