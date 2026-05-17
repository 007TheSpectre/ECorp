# PoC - Exploitation d'un Buffer Overflow (ECorp Tool Handler)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de détourner le flux d'exécution de l'application SUID tool-handler d'ECorp. En exploitant un débordement de tampon sur la pile, il est possible d'écraser un pointeur de fonction local pour rediriger l'exécution vers une fonction cachée (back2back), permettant ainsi de récupérer une archive contenant des données privilégiées (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de corruption mémoire due à la présence d'un pointeur de fonction adjacent à un tampon dans une structure locale (struct locals) sur la pile :

struct {
    char cool_prog[234];
    void (*cb)();
} locals;
Buffer Overflow : Le programme utilise la fonction non sécurisée gets() pour remplir le tampon locals.cool_prog. Cette fonction ne limitant pas le nombre d'octets lus, un attaquant peut écrire au-delà des 234 octets alloués.

Écrasement d'une variable adjacente : En raison des contraintes d'alignement de la mémoire sur une architecture 64-bit, un rembourrage de 6 octets sépare le tampon cool_prog du pointeur de fonction cb. En envoyant un total de 240 octets de remplissage (234 + 6), l'attaquant peut directement écraser les 8 octets du pointeur cb avec une adresse mémoire arbitraire.

Contournement de la logique (Bypass) : Le programme appelle par la suite la fonction pointée de manière dynamique :  
locals.cb();  
Le binaire n'étant pas compilé avec la protection PIE (Position Independent Executable), la fonction cachée back2back() se trouve à une adresse statique fixe (0x4011a6). En remplaçant la valeur de cb par cette adresse, le programme exécute l'instruction malveillante intégrée (system("cat /mnt/dev/srcs.zip | base64")).

## Le Script de PoC (exploit_c4llb4ck.py)
Le script automatise la connexion SSH à la machine cible, prépare le payload structuré, interagit avec le programme vulnérable et extrait automatiquement le fichier chiffré renvoyé.

## Résultat Attendu
Après exécution du script, l'archive srcs.zip exfiltrée est enregistrée localement dans /tmp. Il suffit de la décompresser avec le mot de passe extrait pour lire le fichier confidentiel :


unzip -P 'Th1s_is_one_hell_of_a_P4ssw0rd' /tmp/srcs.zip -d /tmp/srcs_extract
cat /tmp/srcs_extract/secret-manual.txt
Flag récupéré : EPI{1_H0P3_Y0uLL_g1v3_m3_4_C4Ll84ck}

## Remédiation
Pour sécuriser l'application, on peut appliquer les corrections suivantes :

- Remplacement de gets() : Supprimez impérativement l'utilisation de gets() au profit d'une alternative robuste comme fgets(locals.cool_prog, sizeof(locals.cool_prog), stdin); afin de garantir qu'aucun octet supplémentaire ne vienne déborder sur la pile.

- Isolation des pointeurs sensibles : Évitez de placer des pointeurs de fonction ou des données de contrôle critiques au sein d'une même structure ou à proximité immédiate de tampons de données modifiables par l'utilisateur.