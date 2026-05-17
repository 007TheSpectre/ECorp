# PoC - Exploitation d'un Buffer Overflow (ECorp Database Reader)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de contourner l'authentification de l'application database-reader d'ECorp. En exploitant un débordement de tampon sur la pile, il est possible de modifier une variable locale interne pour passer en mode débogage et déchiffrer la base de données (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de corruption mémoire due à l'agencement des variables dans une structure locale (struct locals) sur la pile :


struct {
  char password[48];
  char debug_level[5];
} locals;

Buffer Overflow : Le programme utilise la fonction non sécurisée gets() pour lire le mot de passe et le stocker dans locals.password. Cette fonction ne limite pas le nombre de caractères saisis.

Écrasement d'une variable adjacente : En mémoire, la variable locals.debug_level (initialisée à "guest") est située juste après le tampon password. Si un utilisateur saisit exactement 48 caractères, les caractères suivants déborderont directement dans debug_level.

Contournement de la logique (Bypass) : La condition de validation vérifie si debug_level est égal à "debug" :


if (!strcmp(locals.password, "<REDACTED2>") || !strcmp(locals.debug_level, "debug"))
En injectant 48 octets génériques suivis de la chaîne "debug", on valide cette condition sans connaître le mot de passe administrateur.


## Le Script de PoC (exploit_database-reader.py)
Le script automatise l'envoi du payload. Il sature les 48 octets du mot de passe avec des caractères A puis écrit directement la valeur "debug" pour écraser la variable d'état.

## Résultat Attendu
Après l'envoi du payload, le programme valide l'accès grâce au niveau de privilège altéré et exécute la fonction decode() pour afficher le contenu de la base de données :


ECorp database reader v0.2.23

To access the database, please provide the admin password: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaadebug  
Deciphering the database...

** junior-dev vault **

- zip creds(hashed): 60d3445af1ae382994cec66b619bd2d8
- Secret: EPI{whY_lImIT_Our5elVe5_tO_a55i9NEd_vaLUEz}  

Flag récupéré : EPI{whY_lImIT_Our5elVe5_tO_a55i9NEd_vaLUEz}

## Remédiation
Pour sécuriser l'application, on peut appliquer les corrections suivantes :

- Bannir gets() : Remplacez l'appel par une fonction qui respecte la taille du tampon alloué, par exemple : fgets(locals.password, sizeof(locals.password), stdin);.

- Saisies sécurisées : Nettoyez les retours à la ligne potentiellement capturés par fgets pour éviter les comportements inattendus lors des comparaisons de chaînes (strcmp).