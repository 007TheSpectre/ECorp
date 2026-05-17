# PoC - Exploitation d'un Buffer Overflow (ECorp Forgot Password)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de contourner les restrictions d'accès de l'application forgot-password d'ECorp. En exploitant un débordement de tampon sur la pile via une variable d'environnement, il est possible de modifier une variable locale interne pour passer au rôle autorisé et déclencher la fonction de déhachage (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de corruption mémoire due à l'agencement des variables dans une structure locale (struct locals) sur la pile :


struct {
  char hash[151];
  char role[64];
} locals;

Buffer Overflow : Le programme récupère la valeur de la variable d'environnement DEHASH via getenv() et la copie dans locals.hash en utilisant la fonction non sécurisée strcpy(). Cette fonction ne limite pas le nombre de caractères copiés et continue jusqu'à rencontrer un octet nul (\0).

Écrasement d'une variable adjacente : En mémoire, la variable locals.role (initialisée à "user") est située juste après le tampon hash. Si le contenu de la variable d'environnement dépasse 151 caractères, les caractères suivants déborderont directement dans role.

Contournement de la logique (Bypass) : La condition de validation vérifie si role est égal à "management" :

if (!strcmp(locals.role, "management"))
En injectant un hash MD5 valide de 32 caractères suivi de 119 caractères de remplissage (totalisant 151 octets), les octets suivants écrasent locals.role avec la chaîne "management", validant ainsi la condition.

## Le Script de PoC (exploit_forgot-password.py)
Le script automatise la configuration de l'environnement et l'exécution du binaire. Bien que le fichier Python initial doive être ajusté pour définir la bonne variable d'environnement (DEHASH au lieu de MA_VARIABLE_ENV) ainsi que la valeur du payload, le mécanisme d'exploitation s'appuie sur la définition préalable de la variable suivante :

## Résultat Attendu
Après l'envoi du payload, le programme valide l'accès grâce au rôle altéré et exécute la fonction dehash() pour afficher la solution correspondant au hash fourni :

ECorp password reminder app (BETA)

First service to dehash a hash!
Provide a MD5 hash in DEHASH env variable
Dehashing your hash...
=> EPI{7H1s_1SN7_s4f3R_1N_V4R14BL3z}

Flag récupéré : EPI{7H1s_1SN7_s4f3R_1N_V4R14BL3z}

## Remédiation
Pour sécuriser l'application, on peut appliquer les corrections suivantes :

- Bannir strcpy() : Remplacez l'appel par une fonction qui respecte et limite la taille du tampon de destination, par exemple : strncpy(locals.hash, ptr, sizeof(locals.hash) - 1); en s'assurant de forcer manuellement le dernier octet à zéro pour garantir la fin de chaîne.

- Validation stricte des entrées : Avant toute copie, vérifiez explicitement la longueur de la chaîne pointée par ptr à l'aide de strlen(ptr) et rejetez l'entrée si elle dépasse la capacité maximale autorisée du tampon (150 caractères utiles).