#!/bin/bash

# Compilation des programmes
gcc SERVER.c -o serv -pthread
gcc CLIENT.c -o cli -pthread -lssl -lcrypto

# Lancement du serveur en arrière-plan
./serv &

# Sauvegarde du PID du serveur
SERVER_PID=$!

# Lancement de l'interface Python
python3 interface.py &

# Lancement du client
./cli &

# Attendre que l'utilisateur appuie sur une touche pour arrêter le serveur et le client
read -p "Press any key to stop..."

# Arrêt du serveur et du client
kill $SERVER_PID
pkill -P $$ cli

echo "Server and client have been stopped."

