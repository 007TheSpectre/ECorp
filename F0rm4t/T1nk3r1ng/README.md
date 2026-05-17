# PoC - Exploitation d'une faille de chaîne de format (ECorp Update Preferences)
## Description
Ce dépôt contient la documentation et les scripts de test (PoC) mettant en évidence une vulnérabilité de chaîne de format (Format String) dans l'application CGI interne update-preferences d'ECorp. En exploitant un appel non sécurisé à la fonction printf(), il est possible de lire la mémoire de la pile pour obtenir des fuites d'adresses (leaks) et de réécrire des pointeurs de fonctions dans la table des liaisons globales (GOT) afin d'exécuter des commandes système arbitraires.

## Analyse de la Vulnérabilité
La faille se situe dans la fonction print_default() du binaire CGI, qui est déclenchée lorsque le paramètre de jeu de données (dataset) diffère de la valeur attendue "employees" :


void print_default(char *per_page) {
    char buf[4815];
    strncpy(buf, per_page, sizeof(buf));
    printf("1|");
    printf(buf);       // FAILLE : Le tampon 'buf' contrôlé par l'utilisateur est passé directement comme chaîne de format
    printf("|%s", locked_set);
}
Format String Vulnerability : L'utilisation de printf(buf) sans spécificateur de format statique permet à un utilisateur de transmettre des directives de formatage telles que %p (pour lire la pile), %s (pour lire à partir d'adresses pointées) ou %n / %hhn (pour écrire en mémoire).

Architecture et Protections : L'application est un binaire 32-bit non-PIE (adresses du binaire fixes) exécuté dans un environnement où la bibliothèque standard libc subit l'ASLR. Le tampon de pile de 4815 octets offre un espace conséquent pour injecter des structures complexes ou des adresses de cibles à écraser.

Stratégie d'Exploitation (Global Offset Table Overwrite)
Puisque le binaire n'est pas PIE, la table .got.plt se trouve à un emplacement mémoire statique immuable. La méthode d'attaque consiste à substituer l'adresse d'une fonction de terminaison ou d'affichage (comme exit@GOT ou printf@GOT) par l'adresse de la fonction standard system().

Calcul de l'offset sur la pile : En envoyant un motif reconnaissable (par exemple AAAA suivi de plusieurs spécificateurs %p), on détermine l'index exact de l'argument sur la pile où débute notre chaîne d'entrée.

Fuite de la Libc (Leak) : L'injection de jetons de formatage spécifiques permet de lire une adresse de retour pointant dans la libc afin de calculer dynamiquement son adresse de base et d'en déduire l'emplacement de system().

Écriture arbitraire via %hhn : L'écriture de la nouvelle adresse 32-bit dans l'entrée GOT sélectionnée s'effectue octet par octet (4 écritures successives) en utilisant le modificateur de taille %hhn couplé au nombre de caractères générés par le format d'affichage.

## Le Script de PoC (exploit_t1nk3r1ng.py)
Ce script gère l'extraction des données à distance, calcule dynamiquement l'adresse de la fonction system() à partir d'une fuite mémoire de la libc, génère le vecteur d'attaque et transmet la charge utile doublement encodée (Base64 + URL) au service d'infrastructure Web via la session SSH.

## Résultat Attendu
Une fois l'adresse de la fonction résolue et injectée dans la table GOT, le pointeur altéré déroute le cycle de fermeture de l'application CGI et force l'interprétation du tampon d'entrée en tant que commande shell :

[+] SSH connected to ecorp.cyber.epitest.eu
[*] Launching Format String exploitation chain...
[+] Libc leak: 0xf7da33a0
[+] Calculated system(): 0xf7dc3200
[*] Injecting 4-byte payload into exit@GOT (0x0804c014)...

EPI{that_wAs_D1fF1culT_8UT_YOU_MaNag3d_1t}

## Remédiation
Pour corriger définitivement cette vulnérabilité, appliquez les mesures suivantes :

- Spécifier systématiquement le format : Ne passez jamais une variable utilisateur directement comme premier argument d'une fonction d'impression. Utilisez un indicateur de chaîne de caractères explicite :
printf("%s", buf);

- Activer les options de compilation sécurisées : Compilez vos applications avec l'indicateur de détection d'erreurs de format -Wformat -Werror=format-security afin de bloquer automatiquement la compilation si un tel schéma à risque est détecté. Activer le RELRO complet (-z relro -z now) permet également de figer la table GOT en lecture seule dès le démarrage du programme, empêchant toute tentative d'écrasement dynamique.