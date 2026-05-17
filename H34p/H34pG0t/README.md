# PoC - Exploitation d'un Buffer Overflow (ECorp Chatbot Preprod)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de détourner le flux d'exécution de l'application EcorpGPT (PREPROD) d'ECorp hébergée sur le port local 55555. En exploitant un débordement de tampon en chaîne sur le tas (Heap Overflow), il est possible d'écraser le pointeur d'une structure adjacente, puis d'altérer la table des liaisons globales (GOT) afin de substituer la fonction standard puts() par une fonction privilégiée (spawn_alpha) et d'obtenir le flag.

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de corruption de mémoire dynamique au sein de la fonction try_to_hack_me() lors de l'allocation successive de deux structures de type struct data sur le tas :


struct data { char *secret; };

struct data *d1 = malloc(sizeof(*d1));    // Allocation de la structure d1
d1->secret = malloc(163);                 // Tampon de réception de 163 octets
struct data *d2 = malloc(sizeof(*d2));    // Allocation de la structure d2 (contiguë)
d2->secret = malloc(46);                  // Tampon de réception de d2
Buffer Overflow : Le programme utilise la fonction non sécurisée strcpy() pour copier les données utilisateur d'une part dans d1->secret, puis dans un second temps dans d2->secret. Aucun contrôle de limites n'est opéré sur la longueur des chaînes copiées.

Écrasement d'une variable adjacente : Sur cette architecture 32-bit sans PIE, le tampon d1->secret réside dans un bloc de 176 octets effectifs. La structure d2 se trouvant immédiatement après, son pointeur membre d2->secret est situé à une distance exacte de 176 octets depuis le début des données de d1->secret. Un premier débordement permet d'écraser la valeur de d2->secret avec l'adresse de la fonction puts() dans la section .got.plt.

Contournement de la logique (Bypass) : Le second appel à strcpy() utilise le pointeur falsifié d2->secret comme destination. Par conséquent, l'écriture écrit directement à l'adresse de l'entrée puts@GOT. En y injectant l'adresse de la fonction confidentielle spawn_alpha(), toute invocation ultérieure de la fonction d'affichage standard puts() au sein du programme déroute l'exécution vers le code malveillant afin de lire la clé d'administration (arcadecode).

## Le Script de PoC (exploit_heapgot.py)
Le script gère la validation de l'état du service sur le port local 55555 (qui nécessite l'activation préalable du PoC Chatbot standard), compile dynamiquement le code source témoin sur la cible afin de résoudre les adresses de puts@GOT et spawn_alpha(), puis injecte la charge structurée en deux étapes séparées par le délimiteur $ sur la socket réseau.

## Résultat Attendu
Lorsque l'exploitation aboutit, la table GOT est modifiée. Dès que le Chatbot tente d'écrire un log, il appelle spawn_alpha(), affiche le flag système et déploie le super-chatbot sur le port 54321 :


[+] SSH connected
[+] chatbot-preprod is up on port 55555
[*] Resolved puts@GOT: 0x804c018, spawn_alpha: 0x8049305
[*] Payload: prefix(16) + padding(176) + puts_got(4) + '$' + spawn_alpha(4)

[OUTPUT] (212 bytes):
EcorpGPT v0.01 - unstable features, be warned!
[YOU] > 
[GPT] > Access granted. Flag: EPI{reWr171ng_4_p01N7ER_70_G07_70_b4CKd00r_15_5eXy}
Spawning arcade chatbot...

[!!!] FLAG: EPI{reWr171ng_4_p01N7ER_70_G07_70_b4CKd00r_15_5eXy}
Flag récupéré : EPI{reWr171ng_4_p01N7ER_70_G07_70_b4CKd00r_15_5eXy}

## Remédiation
Pour sécuriser l'application, appliquez les corrections suivantes :

- Bannir strcpy() : Substituez les fonctions de copie non vérifiées par des alternatives bornées telles que snprintf ou strncpy en limitant strictement l'écriture à la taille allouée pour les tampons respectifs (163 octets pour d1->secret et 46 octets pour d2->secret).

- Activation des protections binaires : Compiles l'application avec l'option de protection complète de la table des procédures RELRO (-z relro -z now) afin de rendre la section GOT totalement accessible en lecture seule après le chargement initial, interdisant ainsi toute modification malveillante à l'exécution.