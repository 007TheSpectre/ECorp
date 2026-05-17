# PoC - Exploitation d'un Buffer Overflow (ECorp Tracking Reader)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de contourner les restrictions d'accès de l'application tracking-reader d'ECorp. En exploitant un débordement de tampon sur la pile combiné à une désactivation locale de l'ASLR, il est possible d'exécuter une attaque de type Return-to-libc (ret2libc) pour instancier un shell privilégié et lire les fichiers de logs sécurisés (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de corruption mémoire par débordement de tampon dans son mécanisme de lecture de logs :


char buffer[371];
gets(buffer);   // Lit jusqu'au caractère d'échappement ou de fin de fichier sans vérification de taille

Buffer Overflow : Le programme utilise la fonction non sécurisée gets() pour remplir la variable locale buffer. Comme aucune restriction de taille n'est imposée, la saisie peut dépasser les 371 octets alloués.

Écrasement d'une variable adjacente : En prenant en compte l'alignement des structures de pile géré par GCC (13 octets de rembourrage) et l'espace nécessaire pour stocker l'ancien pointeur de base de la pile (saved RBP de 8 octets), la distance exacte pour atteindre l'adresse de retour est de 392 octets. Tout octet supplémentaire vient écraser cette adresse de retour.

Contournement de la logique (Bypass) : Le binaire est exécuté via le wrapper /usr/bin/cat_ qui utilise la commande setarch -R pour désactiver la distribution aléatoire de l'espace d'adressage (ASLR). De ce fait, l'adresse de base de la bibliothèque standard libc devient fixe (0x7ffff7dc7000) à chaque lancement. Il devient possible de construire une chaîne ROP statique en pointant directement vers le gadget pop rdi; ret pour charger la chaîne "/bin/sh" comme argument de la fonction system().

## Le Script de PoC (exploit_st4ck0v3rfl0w.py)
Le script automatise la connexion SSH à l'aide de privilèges préalablement récupérés, instancie un pseudo-terminal (PTY) configuré en mode brut (raw mode) afin d'éviter la corruption ou la consommation de l'octet critique \x11 présent dans l'adresse du gadget ROP, puis injecte le payload structuré pour exécuter les commandes d'extraction dans le shell obtenu.

## Résultat Attendu
Après le contournement de l'adresse de retour, le shell hérite des privilèges du groupe adm associés à l'appel de l'application, ce qui permet l'accès au fichier de consignation contenant les données cibles :


[*] Running St4ck0v3rfl0w exploit on port 16500...
uid=1002(senior-dev) gid=1002(senior-dev) groups=1002(senior-dev),4(adm)
EPI{K0N5P1r4Cy_UNV31l3D_l375_F1n15h_17}

Flag récupéré : EPI{K0N5P1r4Cy_UNV31l3D_l375_F1n15h_17}

## Remédiation
Pour sécuriser l'application, on peut appliquer les corrections suivantess :

- Suppression de l'usage de fonctions obsolètes : Remplacez impérativement la fonction gets() par une fonction de lecture sécurisée telle que fgets(buffer, sizeof(buffer), stdin); pour éviter toute écriture hors des limites mémoires définies.

- Maintien des protections systèmes : Veillez à ne pas désactiver l'ASLR (setarch -R) en production et activez les mécanismes de protection du compilateur comme les indicateurs de pile (Stack Canaries / -fstack-protector-all) afin de rendre l'écrasement des adresses de retour inopérant.