# PoC - Exploitation d'un Use-After-Free (ECorp Super Chatbot Alpha II)
## Description
Ce dépôt contient le script d'exploitation (PoC) permettant de contourner le mécanisme d'authentification de l'application super-chatbot d'ECorp hébergée sur le port local 54321. En exploitant une vulnérabilité de type Use-After-Free (UAF), il est possible de réallouer et de réécrire une structure de session précédemment libérée afin d'activer de force des privilèges d'administration (obtention du flag).

## Analyse de la Vulnérabilité
L'application présente une vulnérabilité de pointeur suspendu (dangling pointer) lors de la manipulation de variables statiques persistantes sur le tas :


static struct ecorp_auth *auth = NULL;  // { char name[32]; int ds_replication_get_changes_all; }
static char *spn = "";
Buffer Overflow : L'erreur principale réside dans l'implémentation de la commande clear qui libère la mémoire de la structure de session sans réinitialiser le pointeur associé :


free(auth);   // Libération de la structure de 36 octets
              // BUG : auth n'est pas remis à NULL
Écrasement d'une variable adjacente : La structure ecorp_auth occupe 36 octets utiles, ce qui correspond à un bloc mémoire effectif de 48 octets géré par le mécanisme des fastbins de la glibc (dû à un alignement MALLOC_ALIGNMENT=16). Lorsque l'utilisateur invoque ensuite la commande spn <val>, le programme utilise strdup(val) qui effectue un malloc(strlen(val) + 1). En fournissant une chaîne de caractères de 33 octets, le gestionnaire de mémoire (malloc(34)) réalloue instantanément le même bloc de 48 octets tout juste libéré pour y stocker la valeur du SPN.

Contournement de la logique (Bypass) : Le pointeur statique d'authentification auth pointe toujours vers cet emplacement mémoire désormais réutilisé. Les 32 premiers octets de la commande spn remplissent le champ auth->name, tandis que le 33ème octet écrase directement la variable entière auth->ds_replication_get_changes_all. En injectant la valeur \x01 à cette position, le script valide la vérification de sécurité de la commande login :


if (auth && auth->ds_replication_get_changes_all) {
    dc_sync(fd); // Déclenche la fonction privilégiée d'exfiltration
}

## Le Script de PoC (exploit_uaf.py)
Le script valide la présence du service sur le port 54321 (généré suite à la réussite du PoC HeapG0t), ouvre une connexion réseau locale, puis orchestre la séquence d'instructions permettant de corrompre la structure de contrôle avant d'appeler la fonction d'authentification.

## Résultat Attendu
Lorsque la séquence de commandes est transmise, l'appel à login évalue positivement la structure falsifiée par le Use-After-Free et force l'affichage de la clé de validation finale :

[+] SSH connected
[+] super-chatbot-alpha-II-arcade-edition running on port 54321
[*] Running UseAft3rFr33 exploit...

[OUTPUT]:
EcorpGPT v0.001 - AD middleware - early alpha
[YOU] > 
[YOU] > 
[YOU] > 
[YOU] > 
[GPT] > Performing DCSYNC replication against domain controller...
[GPT] > Flag unlocked: EPI{h4Nk_7hE_FrEE_p01n7er_MUs7_8E_SE7_70_nUlL_H44444Nk}
Flag récupéré : EPI{h4Nk_7hE_FrEE_p01n7er_MUs7_8E_SE7_70_nUlL_H44444Nk}

## Remédiation
Pour sécuriser l'application, appliquez les corrections suivantes :

- Mise à zéro systématique des pointeurs libérés : Ne laissez jamais un pointeur faire référence à un bloc mémoire restitué au système. Réinitialisez immédiatement la variable à NULL après l'appel à free() :
free(auth);
auth = NULL;
  
- Validation d'état stricte : Assurez-vous que l'application valide l'état opérationnel et l'origine de l'allocation d'une structure de données avant de faire confiance aux indicateurs de privilèges ou d'autorisation qu'elle contient.