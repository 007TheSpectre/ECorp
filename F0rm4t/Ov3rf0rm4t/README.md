# PoC - Exploitation d'un Stack Overflow via Sprintf (ECorp Internal Login)
## Description
Ce dépôt contient la documentation et les scripts de test (PoC) mettant en évidence un contournement de privilèges sur l'application Web interne d'ECorp. En exploitant une vulnérabilité de formatage de chaîne (Format String) mal contrôlée dans le binaire CGI cgi-login, il est possible de provoquer un débordement de tampon linéaire et d'écraser la variable de rôle locale afin de s'authentifier de force en tant qu'administrateur (intrauser).

## Analyse de la Vulnérabilité
L'application Web délègue l'authentification à un exécutable CGI écrit en C, qui structure ses variables locales au sein d'un bloc mémoire contigu :


struct {
    char username[32];
    char role[64];
} locals = { .role = "guest" };

Format String & Buffer Overflow : La faille survient lorsque l'application traite l'entrée de l'utilisateur fournie par PHP via le tampon d'entrée buffer en utilisant la fonction sprintf() sans argument de format explicite :


sprintf(locals.username, buffer); // Faille : buffer est traité comme une chaîne de format
Parce que l'utilisateur contrôle directement la chaîne de format, il peut injecter des directives de formatage spécifiques (telles que %c ou %s) pour forcer sprintf() à générer une sortie beaucoup plus longue que les 32 octets alloués à locals.username.

Contournement de la logique (Bypass) : L'agencement en mémoire fait que la variable role (initialisée à "guest") est située immédiatement après le tampon username. En envoyant la charge utile %32cintrauser, l'instruction %32c va générer un caractère unique complété par 31 espaces (remplissant ainsi exactement les 32 octets de username). La chaîne "intrauser" qui suit immédiatement vient alors déborder et écraser les premiers octets de locals.role.

Contrainte d'encodage Web : L'entrée transite par un script PHP qui décode une première fois les paramètres POST reçus. Pour que les caractères de formatage %32c parviennent intacts au binaire CGI sous-jacent, le script d'exploitation applique un double encodage URL en transmettant la valeur %2532cintrauser.

## Le Script de PoC (exploit_ovr3f0rm4t.py)
Le script automatise la connexion SSH au serveur pour atteindre le réseau local, génère une requête HTTP POST forgée contenant la charge utile doublement encodée vers le service Web, extrait le cookie de session généré (PHPSESSID) et l'utilise pour interroger le tableau de bord d'administration afin de récupérer le flag.

## Résultat Attendu
À la réception de la chaîne, le binaire CGI écrase le rôle d'accès. Le script PHP valide l'authentification comme intrauser et renvoie la page d'administration contenant le flag associé :

[+] SSH connected
[*] Sending double-encoded exploit format string payload to internal.ecorp.epi...
[*] Extracting administrative dashboard session content...

[OUTPUT] :
<!DOCTYPE html>
<html>
<head><title>Internal Corporate Dashboard</title></head>
<body>
   <h1>Welcome Back, Administrator</h1>
   <p>System status: All services operational.</p>
   <div class="flag">EPI{7H475_50M3_r34LLy_84D_C91}</div>
</body>
</html>


Flag récupéré : EPI{7H475_50M3_r34LLy_84D_C91}

## Remédiation
Pour sécuriser l'application, appliquez les corrections suivantes :

- Assurer l'intégrité des chaînes de format : Ne passez jamais une variable contrôlée par l'utilisateur final en tant que premier argument d'une fonction de la famille printf. Utilisez systématiquement un format statique strict pour l'assignation :
snprintf(locals.username, sizeof(locals.username), "%s", buffer);

- Utilisation de fonctions de copie bornées : Remplacez l'usage de sprintf() par snprintf() en spécifiant explicitement la taille maximale autorisée du tampon récepteur afin d'interdire structurellement tout débordement sur les champs adjacents.