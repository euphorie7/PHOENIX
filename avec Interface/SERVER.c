#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // Bibliothèque pour la manipulation des adresses IP

#include <sys/stat.h>
#include <signal.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <pthread.h>
#define PORT 7777
#define DATA "./DATA/data.txt"
#define MAX_CHAR 50
#define MAX 1000
FILE *data;
int Gserver_fd;
pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t lock;
void initialize_rwlock()
{
  if (pthread_rwlock_init(&lock, NULL) != 0)
  {
    perror("Failed to initialize rwlock");
    exit(1); // Exit if failed to initialize.
  }
}
/*pthread_rwlock_rdlock(&lock);
// Opérations de lecture
pthread_rwlock_unlock(&lock);
pthread_rwlock_wrlock(&lock);
// Opérations d'écriture
pthread_rwlock_unlock(&lock);
*/
typedef struct client_info
{
  char pseudo[MAX_CHAR];
  char password[64];
} client_info;

typedef struct Structclient
{
  int socket_fd;
  // struct sockaddr_in *address; // Adresse IP et port du client
  char ip[INET_ADDRSTRLEN];
  int port;
  pthread_t listen;
  pthread_t post_connect_th;
  int en_ligne;
  char username[50];
  struct Structclient *next;

} Structclient;

typedef struct FIFO
{
  Structclient *tete;
  pthread_t patrouille;
  int taille;
} FIFO;
FIFO *Clients;
FIFO *newFIFO()
{
  FIFO *temp = (FIFO *)malloc(sizeof(FIFO));
  if (temp == NULL)
  {
    printf("erreur creation liste\n");
  }
  temp->patrouille = 0;
  temp->taille = 0;
  return temp;
}
Structclient *newStructclient(int fd, char ip[], int port)
{
  Structclient *temp = (Structclient *)malloc(sizeof(Structclient));
  if (temp == NULL)
  {
    printf("erreur creation maillon\n");
  }
  temp->socket_fd = fd;
  // temp->address = adresse;
  strcpy(temp->ip, ip);
  temp->port = port;
  temp->listen = 0;
  temp->post_connect_th = 0;
  temp->en_ligne = 0;
  return temp;
}

void add(int fd, FIFO *f, char ip[], int port)
{
  Structclient *temp = newStructclient(fd, ip, port);
  if (f->taille != 0)
  {
    Structclient *teteFIFO = f->tete;
    temp->next = teteFIFO;
  }
  f->tete = temp;
  f->taille++;
}
void remove_client(Structclient *c)
{
  if (!Clients || !c)
  {
    printf("Liste vide ou client NULL.\n");
    return;
  }
  printf("-> taille : %d", Clients->taille);
  // Annuler le thread dédié au client
  // pthread_join(c->post_connect_th, NULL); // Attendez que le thread se termine proprement

  // Retrait du client de la liste
  // Verrouillage en écriture pour modification de la liste
  if (Clients->tete == c)
  {
    // Le client est la tête de la liste
    Clients->tete = c->next;
  }
  else
  {
    // Recherche et retrait du client dans la chaîne
    Structclient *prev = Clients->tete;
    while (prev != NULL && prev->next != c)
    {
      prev = prev->next;
    }
    if (prev != NULL)
    {
      prev->next = c->next;
    }
  }
  strcpy(c->username, "");
  free(c); // Libérer la mémoire du client
  c = NULL;

  Clients->taille--;
  // Déverrouillage après modification
  printf("Client supprimé taille : %d\n", Clients->taille);
}

Structclient *pop(FIFO *f)
{
  if (f->taille == 0)
  {
    return NULL;
  }
  Structclient *first = f->tete;
  f->tete = first->next;
  f->taille--;
  return first;
}
void freeFIFO(FIFO *f)
{
  while (f->tete != NULL)
  {
    free(pop(f));
  }
  free(f);
}

void nettoie()
{

  // close(Gserver_fd);
  pthread_rwlock_rdlock(&lock);
  Structclient *temp = Clients->tete;
  pthread_rwlock_unlock(&lock);
  while (temp != NULL)
  {

    sleep(1);
    if (temp->post_connect_th != 0)
    {
      pthread_cancel(temp->post_connect_th);
    }
    if (temp->listen != 0)
    {
      pthread_cancel(temp->listen);
    }
    close(temp->socket_fd);
    temp = temp->next;
  }
  freeFIFO(Clients);
}
void sighandler(int sig)
{
  if (sig == SIGINT)
  {
    // nettoie
    printf("\nSHUT DOWN\n");
    nettoie();
    exit(1);
  }
}

// Plutôt juste pour vérifier

// pthread_mutex_t conv_mutex = PTHREAD_MUTEX_INITIALIZER;
void distribuer(char message[], size_t sizemsg, Structclient *destributeur)
{
  size_t bytes;
  pthread_rwlock_rdlock(&lock);
  Structclient *temp = Clients->tete;
  printf("141 ici ca stope ? %s \n", message);
  printf("taille de file : %d en tete %p   distributeur %p\n", Clients->taille, temp, destributeur);
  while (temp != NULL)
  {
    printf("from :%p->to->%p\n", destributeur, temp);

    if (temp != destributeur)
    {
      pthread_mutex_lock(&fd_mutex);
      bytes = send(temp->socket_fd, message, sizemsg, 0);
      pthread_mutex_unlock(&fd_mutex);
      if (bytes <= 0)
      {

        printf("probleme d'ecriture ~ mutex\n");
        nettoie();
      }
    }

    temp = temp->next;
  }
  pthread_rwlock_unlock(&lock);
  printf("done distr\n");
  // nettoie();
  // exit(0);
  // conserver la vonc
  /* pthread_mutex_lock(&conv_mutex);
   if (write(Clients->fd, message, sizemsg) <= 0)
   {

       printf("probleme d'ecriture ~ mutex\n");
       t_join(Clients);
   }
   pthread_mutex_unlock(&conv_mutex);
*/
  // faut distribuer le buffer puis voir le probleme de la boucle infinie
}

void *post_connect(void *arg)
{
  Structclient *temp = (Structclient *)arg;
  temp->en_ligne = 1;
  char buffer[256];
  char buffer_send[326];
  size_t bytesread;
  printf("thread server lire en cours ...\n");
  char message_connect[256], message_disconnect[256];
  sprintf(message_connect, "[%s] is connected\n", temp->username);
  sprintf(message_disconnect, "[%s] is disconnected\n", temp->username);
  distribuer(message_connect, strlen(message_connect), temp);

  fflush(stdout);
  // memset(buffer, 0, sizeof(buffer));
  while (1)
  {
    bytesread = recv(temp->socket_fd, buffer, sizeof(buffer), 0);
    if (bytesread <= 0)
    { // Déconnexion ou erreur
      fprintf(stdout, "[%s] is disconnected\n", temp->username);
      pthread_rwlock_wrlock(&lock);
      remove_client(temp);
      pthread_rwlock_unlock(&lock);
      close(temp->socket_fd); // Fermez le socket
      pthread_exit(NULL);
    }
    buffer[bytesread] = '\0';
    sprintf(buffer_send, "[%s] : %s", temp->username, buffer);
    distribuer(buffer_send, sizeof(buffer_send), temp);

    fflush(stdout);
  }
  printf("thread distr va se terminer\n");
  distribuer(message_disconnect, sizeof(message_disconnect), temp);
  nettoie();
  pthread_exit(NULL);
}
void post_connexion(Structclient *temp)
{

  if (pthread_create(&temp->post_connect_th, NULL, post_connect, (void *)temp) == 0)
  {
    pthread_detach(temp->post_connect_th); // Détachez le thread pour qu'il se nettoie seul à sa terminaison.
  }
  else
  {
    perror("THREAD : Could not create thread");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
int comparerChaine(char *char1, const char *chaineComparaison)
{
  return strcmp(char1, chaineComparaison) == 0 ? 1 : 0;
}
int in_data_sign_up(char pseudo[])
{
  data = fopen(DATA, "a+");
  if (data == NULL)
  {
    printf("Impossible de créer ou d'ouvrir le fichier.\n");
    exit(EXIT_FAILURE);
  }
  char ligne[256];
  char user[50], pass[50];
  while (fgets(ligne, sizeof(ligne), data) != NULL)
  {
    if (sscanf(ligne, "%49[^;];%49[^\n]\n", user, pass) == 2)
    { // Sécurise la lecture des chaînes
      fprintf(stdout, "user : %s\n", user);
      if (comparerChaine(user, pseudo) == 1)
      {
        fprintf(stdout, "indata : ca existe\n");
        fclose(data);
        return 1; // Retourne 1 si le pseudo est trouvé
      }
    }
  }
  fclose(data);
  return 0;
}
void checkinClientsenligne(char pseudo[], int fd)
{
  char error[10] = "||";
  pthread_rwlock_rdlock(&lock);
  size_t bytes;
  Structclient *temp = Clients->tete;
  while (temp != NULL)
  {
    if (comparerChaine(temp->username, pseudo) == 1)
    {
      fprintf(stdout, "Vous etes deja en ligne %p %d\n", Clients->tete, Clients->taille);
      pthread_mutex_lock(&fd_mutex);
      bytes = send(fd, error, strlen(error), 0);
      pthread_mutex_unlock(&fd_mutex);
      if (bytes <= 0)
      {
        printf("|| ne s'envoie pas\n");
      }

      pthread_exit(NULL);
    }
    temp = temp->next;
  }
  pthread_rwlock_unlock(&lock);
}
int in_data_sign_in(char pseudo[], char password[])
{
  data = fopen(DATA, "a+");
  if (data == NULL)
  {
    printf("Impossible de créer ou d'ouvrir le fichier.\n");
    exit(EXIT_FAILURE);
  }
  char ligne[256];
  char user[50], pass[50];
  while (fgets(ligne, sizeof(ligne), data) != NULL)
  {
    if (sscanf(ligne, "%49[^;];%49[^\n]\n", user, pass) == 2)
    { // Sécurise la lecture des chaînes
      fprintf(stdout, "user : %s\n", user);
      if (comparerChaine(user, pseudo))
      {

        fclose(data);
        strcpy(password, pass); // Retourne 1 si le pseudo est trouvé
        return 1;
      }
    }
  }

  strcpy(password, "&&");
  fclose(data);
  return 0;
}
void ask_for_valid_user_name(char *pseudo, int fd)
{

  char flag_0[10] = "0";
  char flag_1[10] = "1";
  size_t bytesread, bytes;
  while (in_data_sign_up(pseudo) == 1)
  {
    fprintf(stdout, "adk_for_valid_name : on refait\n");

    pthread_mutex_lock(&fd_mutex);
    bytes = send(fd, flag_0, strlen(flag_0), 0);
    pthread_mutex_unlock(&fd_mutex);
    if (bytes <= 0)
    {
      printf("|| ne s'envoie pas\n");
    }
    bytesread = recv(fd, pseudo, sizeof(pseudo), 0);
    pseudo[bytesread] = '\0';
  }

  printf("%s flag = %s\n", pseudo, flag_1);
  pthread_mutex_lock(&fd_mutex);
  bytes = send(fd, flag_1, strlen(flag_1), 0);
  pthread_mutex_unlock(&fd_mutex);
  if (bytes <= 0)
  {
    printf("|| ne s'envoie pas\n");
  }
}
void check_for_user_name_validity(char pseudo[], int fd, char Password[])
{
  char password[64];
  char flag_0[10] = "0";
  char flag_1[10] = "1";
  size_t bytes;
  in_data_sign_in(pseudo, password);
  if (strcmp(password, "&&") == 0)
  {
    pthread_mutex_lock(&fd_mutex);
    bytes = send(fd, flag_0, sizeof(flag_0), 0);
    pthread_mutex_unlock(&fd_mutex);
    if (bytes <= 0)
    {
      printf("|| ne s'envoie pas\n");
    }
    strcpy(Password, "&&");
  }
  else
  {
    pthread_mutex_lock(&fd_mutex);
    bytes = send(fd, flag_1, sizeof(flag_1), 0);
    pthread_mutex_unlock(&fd_mutex);
    if (bytes <= 0)
    {
      printf("|| ne s'envoie pas\n");
    }
    strcpy(Password, password);
  }
}
client_info *new_infos_client()
{
  client_info *client = (client_info *)malloc(sizeof(client_info));
  if (client == NULL)
  {
    printf("Erreur malloc");
    return NULL;
  }
  return client;
}
void sign_in(Structclient *arg)
{
  Structclient *client = (Structclient *)arg;
  int fd = client->socket_fd;
  char flag_0[10] = "0";
  char flag_1[10] = "1";
  printf("j'attend un pseudo\n");
  printf("%p\n", Clients->tete);
  char pseudo[50];
  char password[64];
  char pass_recu[64];
  size_t bytes;
  bytes = recv(fd, pseudo, sizeof(pseudo), 0);
  pseudo[bytes] = '\0';
  checkinClientsenligne(pseudo, fd);
  check_for_user_name_validity(pseudo, fd, password);
  strcpy(client->username, pseudo);

  if (strcmp(password, "&&") == 0)
  {
    printf("This username doesn't exist\n");
    pthread_exit(NULL);
  }
  else
  {
    bytes = recv(fd, pass_recu, sizeof(pass_recu), 0);
    pass_recu[bytes] = '\0';
    if (comparerChaine(pass_recu, password) == 1)
    {
      pthread_mutex_lock(&fd_mutex);
      bytes = send(fd, flag_1, strlen(flag_1), 0);
      pthread_mutex_unlock(&fd_mutex);
      if (bytes <= 0)
      {
        printf("|| ne s'envoie pas\n");
      }
      post_connexion(client);
      pthread_exit(NULL);
    }
    else
    {
      pthread_mutex_lock(&fd_mutex);
      bytes = send(fd, flag_0, strlen(flag_0), 0);
      pthread_mutex_unlock(&fd_mutex);
      if (bytes <= 0)
      {
        printf("|| ne s'envoie pas\n");
      }
    }
  }

  pthread_exit(NULL);
}
////////////////////////////////////////////////////////////////////////////////////////////

void sign_up(Structclient *arg)
{
  Structclient *client = (Structclient *)arg;
  int fd = client->socket_fd;

  printf("j'attend un pseudo\n");
  char pseudo[50];
  size_t bytes;
  bytes = recv(fd, pseudo, sizeof(pseudo), 0);
  pseudo[bytes] = '\0';
  checkinClientsenligne(pseudo, fd);
  /* test */ printf("%s\n", pseudo);

  ask_for_valid_user_name(pseudo, fd);

  printf("c'est bon \n");

  // On recoit la structure client_info via le client
  client_info *Personne = new_infos_client();
  if (recv(fd, Personne, sizeof(client_info), 0) <= 0)
  {
    printf("Problème reception \n");
    pthread_rwlock_wrlock(&lock);
    remove_client(client);
    pthread_rwlock_unlock(&lock);
    pthread_exit(NULL);
  }
  strcpy(client->username, Personne->pseudo);

  char requette[256];

  // >> base de donne
  sprintf(requette, "%s;%s", Personne->pseudo, Personne->password);
  requette[strlen(Personne->password) + strlen(Personne->pseudo) + 1] = '\0';
  // printf("%s\n", requette);

  data = fopen(DATA, "a+");
  if (data == NULL)
  {
    printf("Impossible de créer ou d'ouvrir le fichier.\n");
    exit(EXIT_FAILURE);
  }
  if (fprintf(data, "%s\n", requette) <= 0)
  {
    printf("ereur\n");
    nettoie();
    pthread_exit(NULL);
  }
  fclose(data);
  free(Personne);
  post_connexion(client);
  pthread_exit(NULL);
}

void *listenth(void *arg)
{
  Structclient *temp = (Structclient *)arg;
  char buffer[256];
  size_t bytesread;

  // Vous pourriez envisager de mettre une condition de boucle ici qui vérifie un état qui peut être modifié
  while (temp->en_ligne == 0 && (bytesread = recv(temp->socket_fd, buffer, sizeof(buffer), 0)) > 0)
  {
    buffer[bytesread] = '\0';

    if (strcmp(buffer, "1") == 0)
    {
      printf("%s connexion\n", buffer);
      sign_in(Clients->tete);
      break; // Ajouter un break pour sortir de la boucle une fois que sign_in est appelé
    }
    else if (strcmp(buffer, "2") == 0)
    {
      printf("%s Nouvelle utilisateur connecté\n", buffer);
      sign_up(Clients->tete);
      break; // Ajouter un break pour sortir de la boucle une fois que sign_up est appelé
    }
    else if (strcmp(buffer, "3") == 0)
    {
      printf("il quitte\n");
      pthread_exit(NULL);
    }
  }

  return NULL;
}

void *accept_thread(void *arg)
{
  int server_fd = *(int *)arg;
  struct sockaddr_in client_address;
  socklen_t size = sizeof(client_address);
  while (1)
  {
    fprintf(stdout, "je suis a l'ecoute taille de liste : %d\n", Clients->taille);

    int new_client_fd = 0;
    if ((new_client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&size)) < 0)
    {
      perror("accept échoué");
      // Gérer l'erreur ici, par exemple, fermer les ressources, signaler l'erreur au gestionnaire principal, etc.
      pthread_exit(NULL);
      // ou autre action appropriée
    }
    else
    {
      char client_ip[INET_ADDRSTRLEN];
      //  memset(client_ip, 0, sizeof(client_ip));
      inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
      int client_port = ntohs(client_address.sin_port);
      // Accéder au port du client
      pthread_rwlock_wrlock(&lock);
      add(new_client_fd, Clients, client_ip, client_port);
      pthread_rwlock_unlock(&lock);
      // Accéder à l'adresse IP du client

      printf(" Client : %s  >> port : %d est connecté\n", Clients->tete->ip, Clients->tete->port);

      if (pthread_create(&Clients->tete->listen, NULL, listenth, (void *)Clients->tete) == 0)
      {
        pthread_detach(Clients->tete->listen); // Détachez le thread pour qu'il se nettoie seul à sa terminaison.
      }
      else
      {
        perror("THREAD : Could not create thread");
      }
    }
  }
  pthread_exit(NULL);
}

int main()
{
  initialize_rwlock();
  // data = fopen(DATA, "a+");

  // Vérifier si l'ouverture du fichier a réussi
  /* if (data == NULL)
   {
     printf("Impossible de créer ou d'ouvrir le fichier.\n");
     return 1;
   }
   */

  // Écrire dans le fichier si nécessaire

  int server_fd;                                  // Descripteurs de fichiers pour le socket serveur
  struct sockaddr_in server_adress;               // Structure pour l'adresse du serveur
  int server_adress_size = sizeof(server_adress); // Taille de la structure d'adresse
  if (signal(SIGINT, sighandler) == SIG_ERR)
  {
    perror("Erreur lors de la mise en place du gestionnaire de signal");
    return EXIT_FAILURE;
  }
  // Création du socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {                          // Création d'un socket IPv4 TCP
    perror("socket échoué"); // Affiche un message d'erreur en cas d'échec de création du socket
    exit(EXIT_FAILURE);      // Quitte le programme en cas d'échec
  }
  // Configuration de l'adresse du serveur
  server_adress.sin_family = AF_INET;         // Famille d'adresses (IPv4)
  server_adress.sin_addr.s_addr = INADDR_ANY; // Adresse IP du serveur (0.0.0.0 pour toutes les interfaces)
  // le serveur acceptera les connexions entrantes sur toutes les interfaces réseau disponibles de la machine.
  server_adress.sin_port = htons(PORT); // Port sur lequel le serveur écoutera (conversion en ordre de octets réseau)

  // Attachement du socket à l'adresse et au port spécifiés
  if (bind(server_fd, (struct sockaddr *)&server_adress, server_adress_size) < 0)
  {
    perror("bind échoué"); // Affiche un message d'erreur en cas d'échec de liaison du socket
    exit(EXIT_FAILURE);    // Quitte le programme en cas d'échec
  }
  // Mise en écoute du socket avec une file d'attente de taille 3
  if (listen(server_fd, 3) < 0)
  {
    perror("listen échoué"); // Affiche un message d'erreur en cas d'échec de mise en écoute du socket
    exit(EXIT_FAILURE);      // Quitte le programme en cas d'échec
  }
  Gserver_fd = server_fd;
  Clients = newFIFO();
  pthread_t accept;

  if ((pthread_create(&accept, NULL, accept_thread, (void *)&server_fd) != 0))
  {
    {
      perror("THREAD :");
      // Traiter l'erreur ici
    }
  }

  pthread_join(accept, NULL);

  return 1;
}