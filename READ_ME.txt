
Pour la partie 1, il nécessaire d'avoir installer le paquet xterm pour continuer


##############

Pour la partie 2:

1. Ouvrir les ports d’entrée et de sortie du port choisis dans le serveur, içi 7777, on peut le faire via ufw
2. On change l’adresse IP SERVER_IP du fichier CLIENT.c avec l’adresse IP de la machine serveur dans le réseau
3. Coté serveur on compilera de cette façon : gcc -Wall -pthread SERVER.c -o ./serv
4. Coté client, il faut rajouter le drapeau -lcrypto après -o
5. On exécute le programme serveur
6. Puis, les programmes client 
7. Puis vous pouvez lancer l'interface python , attendre la connexion entre programme client et l'interface.


Pour pouvoir communiquer de la même manière qu’avec la version 1 du client, il suffit
de changer l’adresse IP SERVER_IP avec celle du localhost.
