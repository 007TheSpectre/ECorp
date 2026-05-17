# PoC - Exploitation d'un Buffer Overflow (ECorp Read Pub SSH)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de contourner les restrictions d'accès de l'application read-pub-ssh d'ECorp. En exploitant un débordement de tampon sur la pile, il est possible de rediriger le flux d'exécution (technique du Return-to-function) vers une fonction interne (read_ssh) afin de lire un fichier arbitraire ciblé via un lien symbolique et d'élever ses privilèges (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de corruption mémoire par débordement de tampon dans la fonction finish() qui traite la fin de l'exécution :


void finish(int fd) {
    char buf[144];
    read(0, buf, 1024);  // Lit jusqu'à 1024 octets dans un tampon de 144 octets
    // ...
}
Buffer Overflow : Le programme utilise la fonction read() pour recevoir des données utilisateur directement dans un tampon local buf de 144 octets sans vérifier si la quantité de données lues respecte la capacité du conteneur.

Écrasement d'une variable adjacente : Sur une architecture de type 64-bit, l'adresse de retour de la fonction courante se situe juste après la variable buf et l'espace réservé pour l'ancien pointeur de base de la pile (saved RBP, de 8 octets). En envoyant exactement 152 octets de remplissage (144 + 8), les données suivantes écrasent l'adresse de retour (return address).

Contournement de la logique (Bypass) : Le binaire s'exécutant avec des privilèges étendus via la configuration sudo NOPASSWD, la redirection du pointeur d'instruction vers la fonction read_ssh() située à l'adresse fixe 0x401176 force la réouverture du fichier configuré. L'application lisant le fichier pointé par le lien symbolique ~/.ssh/id_rsa.pub, remplacer ce lien par un fichier sensible cible (comme la clé privée SSH de l'utilisateur senior-dev) permet d'en divulguer le contenu.

## Le Script de PoC (exploit_r3t2func2.py)
Le script automatise la création des liens symboliques, initialise un pseudo-terminal (PTY) en mode brut (raw mode) pour éviter que le protocole de contrôle de ligne n'altère les octets de l'adresse injectée (\x11 et \x16), envoie la charge utile et extrait la clé ou le flag.

## Résultat Attendu
Lorsque le script s'exécute, il détourne l'exécution vers read_ssh() après avoir lié id_rsa.pub au fichier de destination. La clé de l'utilisateur senior-dev ou les fichiers cibles sont lus avec ses privilèges :


[+] SSH connected
[*] TEST: ret to read_ssh() without symlink (should print real pubkey)
...
[*] Trying symlink → /home/senior-dev/flag
stderr: Payload sent
output: EPI{5ENI0R_d0E5N7_mEAN_Le55_pr0ne_70_HACK}
[+] id_rsa.pub restored

Flag récupéré : EPI{5ENI0R_d0E5N7_mEAN_Le55_pr0ne_70_HACK}

## Remédiation
Pour sécuriser l'application, on peut appliquer les corrections suivantes :

- Contrôle strict de la taille des écritures : Modifiez l'instruction de lecture pour restreindre le nombre d'octets aux limites du conteneur d'accueil en utilisant la taille réelle définie : read(0, buf, sizeof(buf));.

- Alignement et sécurisation des privilèges : Assurez-vous que l'application valide l'intégrité et la nature des fichiers manipulés avant ouverture afin de rejeter les liens symboliques pointant vers des structures sensibles hors du périmètre de l'utilisateur standard.