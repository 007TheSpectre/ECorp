# PoC - Exploitation d'un Buffer Overflow (ECorp Access Checker)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de contourner les restrictions de sécurité de l'application interne access-checker d'ECorp. L'exploitation réussie permet de désactiver la sécurité (failsafe) et d'exfiltrer des données sensibles (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente deux vulnérabilités majeures qui, combinées, permettent l'exécution de la fonction data_exfiltration() :
  
- Buffer Overflow (Stack-based) : La fonction main utilise la fonction non sécurisée gets() pour remplir le tampon username (fixé à 64 octets). gets() ne vérifiant pas la taille des entrées, envoyer une chaîne plus longue permet d'écraser les variables adjacentes sur la pile (stack), notamment la variable failsafe (initialement positionnée à "Locked"). En modifiant cette valeur, la condition strcmp(failsafe, "Locked") devient fausse, désactivant ainsi la sécurité.
  
- Faiblesse de validation (Logique) : La fonction check_access valide l'utilisateur en comparant l'entrée avec le fichier /etc/passwd via un strncmp limité aux 9 premiers caractères. En commençant le payload par root, le programme valide l'existence de l'utilisateur malgré les caractères superflus qui suivent.


## Le Script de PoC (exploit_access-checker.py)
Le script automatise l'envoi du payload malveillant. Il envoie root (pour valider le contrôle d'accès) suivi de 100 caractères A pour saturer le tampon et écraser la variable failsafe.


### Résultat Attendu
Lorsque le script est exécuté, la pile est corrompue, le mode Failsafe est désactivé, et le programme effectue une requête vers l'intranet pour récupérer le flag :


Scanning /etc/passwd...
Valid username 'rootaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' found
Failsafe disabled, sending data to our internal website...
=> calling http://internal.ecorp.epi?api_key=EPI{W3lC0m3_70_0v3RFL0wZ}

Flag récupéré : EPI{W3lC0m3_70_0v3RFL0wZ}

## Remédiation
Pour corriger cette faille, il est impératif de :

- Remplacer gets(username) par une fonction sécurisée qui limite la taille de la saisie, comme fgets(username, sizeof(username), stdin).

- Corriger la logique de validation dans check_access pour s'assurer que la correspondance dans /etc/passwd est exacte et ne se base pas uniquement sur les premiers caractères.