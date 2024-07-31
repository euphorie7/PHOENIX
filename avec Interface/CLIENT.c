#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <openssl/evp.h>

int client_fd;
int py_fd;
#define PORT_1 7777
#define PORT_py 6666
#define SERVER_IP "127.0.0.1"
#define SERVER_PY "127.0.0.1"
#define MAX_CHAR 50
void authentification(int fd, int py);
typedef struct client_info
{
    char pseudo[MAX_CHAR];
    char password[64];
} client_info;
typedef struct
{
    pthread_t thread_ecrire;
    pthread_t thread_lire;

} myThreads;
myThreads *MT;
void nettoie()
{
    close(client_fd);
    if (MT->thread_lire != 0 || MT->thread_lire != 0)
    {
        pthread_cancel(MT->thread_lire);
        pthread_cancel(MT->thread_ecrire);
    }

    free(MT);
    exit(77);
}
void sighandler(int sig)
{
    if (sig == SIGINT)
    {
        // nettoie
        printf("SHUT DOWN\n");
        nettoie();
    }
}

void clearPreviousLine()
{
    // Déplacer le curseur vers le haut
    printf("\033[F");
    // Effacer la ligne
    printf("\033[K");
}
void *readth(void *arg)
{
    int fd = *(int *)arg;
    char buffer[256];

    size_t bytesread, bytesent;
    fprintf(stdout, "thread exec2 lire en cours ...%ld\n", pthread_self());
    fflush(stdout);
    // memset(buffer, 0, sizeof(buffer));
    while ((bytesread = recv(fd, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytesread] = '\0';
        bytesent = send(py_fd, buffer, strlen(buffer), 0);
        if (bytesent <= 0)
        {
            fprintf(stdout, "erreur d'envoie 76\n");
        }
    }
    return NULL;
}
void *writeth(void *arg)
{
    int fd = *(int *)arg;
    char *message = "thread exec2 ecrire en cours ...\n";
    fprintf(stdout, "%s %ld \nm", message, pthread_self());
    fflush(stdout);
    char buffer[256];
    // memset(buffer, 0, sizeof(buffer));

    size_t byteswritten;

    size_t bytesrecv;

    // clearPreviousLine();
    printf("[Vous] : %s", buffer);
    fflush(stdout);
    // memset(dist, 0, sizeof(dist));
    // buffer[strlen(buffer)] = '\0';
    while ((bytesrecv = recv(py_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        printf("-> bytes %ld\n", bytesrecv);
        // clearPreviousLine();
        buffer[bytesrecv] = '\0';
        printf("[Vous] : %s\n", buffer);
        fflush(stdout);
        // memset(dist, 0, sizeof(dist));
        // buffer[strlen(buffer)] = '\0';
        byteswritten = send(fd, buffer, strlen(buffer), 0);
        if (byteswritten <= 0)
        {
            nettoie();
            printf("byteswritten error\n");
            return NULL;
        }
    }

    nettoie();
    printf("probleme de gets\n");
    return NULL;
    //--------------------------------
}
void chat(int fd, int py)
{

    if (pthread_create(&MT->thread_ecrire, NULL, writeth, (void *)&fd) != 0 || pthread_create(&MT->thread_lire, NULL, readth, (void *)&fd) != 0)
    {
        perror("THREAD :");
        // Traiter l'erreur ici
    }
    else
    {
        printf("hello %d\n", getpid());
    }
    // pthread_create(&lire, NULL, readth, (void *)argv[1]) != 0 ||
    //  pthread_join(lire, NULL);
    pthread_join(MT->thread_lire, NULL);
    pthread_join(MT->thread_ecrire, NULL);
}
void connexion()
{
    printf("Login");
}
void get_string_send_toserver(char pseudo[], int fd, int py)
{
    char buffer[256];
    size_t bytes;
    bytes = recv(py, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
    {
        printf("erreur ligne 135\n");
    }
    buffer[bytes] = '\0';
    strcpy(pseudo, buffer);
    fprintf(stdout, "-->%s\n", buffer);
    /*if (fgets(pseudo, MAX_CHAR, stdin) == NULL)
    {
        printf("Erreur de lecture du pseudo\n");
    }

    char *newline_pos = strchr(pseudo, '\n');
    if (newline_pos != NULL)
    {
        // Remplacer le caractère de retour à la ligne par un caractère nul
        *newline_pos = '\0';
    }
    */
    if (send(fd, pseudo, strlen(pseudo), 0) <= 0)
    {
        printf("Erreur d'envoi du pseudo\n");
    }
}
void chack_validite_pseudo(char pseudo[], int fd, int py)
{
    char flag[10];

    size_t bytesread, bytesent;
    get_string_send_toserver(pseudo, fd, py);
    while ((bytesread = recv(fd, flag, sizeof(flag), 0)) > 0)
    {
        flag[bytesread] = '\0';
        bytesent = send(py, flag, strlen(flag), 0);
        if (bytesent <= 0)
        {
            printf("erreor send 187\n");
        }
        else
        {
            fprintf(stdout, "->>%s\n", flag);
        }
        if (strcmp(flag, "0") == 0)
        {
            printf("Veuillez choisir un autre cela existe deja :)\n");

            get_string_send_toserver(pseudo, fd, py);
        }
        else if (strcmp(flag, "1") == 0)
        {

            fprintf(stdout, "chack_validite : %s \n", pseudo);

            break;
        }
        else
        {
            fprintf(stdout, "checkout here 164\n");
        }
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
void get_password(client_info *Personne, int py)
{
    char pswrd[256];
    char output[33];
    size_t bytesrecv;
    bytesrecv = recv(py, pswrd, sizeof(pswrd), 0);
    if (bytesrecv <= 0)
    {
        printf("erreor send 187\n");
    }
    /* if (fgets(pswrd, MAX_CHAR, stdin) == NULL)
      {
          printf("Erreur lecture\n");
      }

    char *newline_pos = strchr(pswrd, '\n');
    if (newline_pos != NULL)
    {
        // Remplacer le caractère de retour à la ligne par un caractère nul
        *newline_pos = '\0';
    }
    */
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    int i;

    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname("md5");
    if (!md)
    {
        printf("Error: unable to initialize MD5 digest\n");
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, pswrd, strlen(pswrd));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    for (i = 0; i < hash_len; i++)
    {
        sprintf(output + i * 2, "%02x", hash[i]);
    }
    output[32] = '\0';
    //  MD5(pswrd, strlen((char*)pswrd), output);
    strcpy(Personne->password, output);
    Personne->password[sizeof(Personne->password) - 1] = '\0';
}
void get_pseudo_and_send_to_server(char pseudo[], int fd, int py)
{
    char flag[10];

    size_t bytesread, bytesent;
    get_string_send_toserver(pseudo, fd, py);
    while (((bytesread = recv(fd, flag, sizeof(flag), 0)) > 0))
    {
        flag[bytesread] = '\0';
        bytesent = send(py, flag, strlen(flag), 0);
        if (bytesent <= 0)
        {
            fprintf(stdout, "erreur 248\n");
        }
        else
        {
            fprintf(stdout, "->%s\n", flag);
        }
        if (strcmp(flag, "||") == 0)
        {
            printf("vous etes deja en ligne\n");

            authentification(fd, py);
        }
        if (strcmp(flag, "0") == 0)
        {
            printf("ce user n'ame 'nexiste pas\n");
            authentification(fd, py);
        }
        else if (strcmp(flag, "1") == 0)
        {

            fprintf(stdout, "Enter ypur password");

            break;
        }
        else
        {
            fprintf(stdout, "checkout here 243\n");
            fprintf(stdout, "%s\n", flag);
        }
    }
}
void get_password_and_hach(char Password[], size_t size, int py)
{
    char pswrd[256];
    char output[33];
    size_t bytes;
    bytes = recv(py, pswrd, sizeof(pswrd), 0);
    if (bytes <= 0)
    {
        printf("erreur ligne 135\n");
    }
    pswrd[bytes] = '\0';
    fprintf(stdout, "-->%s\n", pswrd);
    /*if (fgets(pswrd, MAX_CHAR, stdin) == NULL)
    {
        printf("Erreur lecture\n");
    }
    char *newline_pos = strchr(pswrd, '\n');
    if (newline_pos != NULL)
    {
        // Remplacer le caractère de retour à la ligne par un caractère nul
        *newline_pos = '\0';
    }
    */
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    int i;

    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname("md5");
    if (!md)
    {
        printf("Error: unable to initialize MD5 digest\n");
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, pswrd, strlen(pswrd));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    for (i = 0; i < hash_len; i++)
    {
        sprintf(output + i * 2, "%02x", hash[i]);
    }
    output[32] = '\0';
    //  MD5(pswrd, strlen((char*)pswrd), output);
    strcpy(Password, output);
    Password[size - 1] = '\0';
    fprintf(stdout, "%s\n", Password);
}
uint32_t sign_in(int fd, int py)
{

    char pseudo[MAX_CHAR];
    char flag[10];
    printf("Commencer par entrer vos infomations personnelles\n");
    printf("Pseudo: ");

    // On teste si le pseudo est dejà dans la BDD en l'envoyant au serv via send à l'aide d'un flag
    // Statue à 0 >> Pseudo déja choisi || Statue à 1 >> Pseudo ok
    get_pseudo_and_send_to_server(pseudo, fd, py);
    char Password[64];
    get_password_and_hach(Password, sizeof(Password), py);
    if (send(fd, Password, strlen(Password), 0) <= 0)
    {
        printf("Error message 305");
    }
    size_t bytes_flag;
    if ((bytes_flag = recv(fd, flag, sizeof(flag), 0)) <= 0)
    {
        fprintf(stdout, "error messga 310\n");
    }
    flag[bytes_flag] = '\0';

    if (strcmp(flag, "1") == 0)
    {
        chat(fd, py);
    }
    else if (strcmp(flag, "0") == 0)
    {
        fprintf(stdout, "Password incorrect \n");
        authentification(fd, py);
    }
    else
    {
        fprintf(stdout, "erreur 322\n");
    }
    // quand c'est valide on le capture

    return 1;
}
uint32_t new_account(int fd, int py)
{

    client_info *Personne = new_infos_client();
    char pseudo[MAX_CHAR];

    printf("Commencer par entrer vos infomations personnelles\n");
    printf("Pseudo: ");

    // On teste si le pseudo est dejà dans la BDD en l'envoyant au serv via send à l'aide d'un flag
    // Statue à 0 >> Pseudo déja choisi || Statue à 1 >> Pseudo ok
    chack_validite_pseudo(pseudo, fd, py);
    strcpy(Personne->pseudo, pseudo);
    fprintf(stdout, "chosen pseudo is : %s %s\n", pseudo, Personne->pseudo);
    // quand c'est valide on le capture

    // mot de passe pas besoin de controle
    printf("Mot de passe: ");
    get_password(Personne, py);
    printf("C'est bon pour vous ?\n  Pseudo: %s | Mdp: %s\n", Personne->pseudo, Personne->password);
    printf("Enregistrement des informations dans la BDD users... \n");

    // On envoie une instance de struct client_info aux serveur
    if (send(fd, Personne, sizeof(*Personne), 0) == SO_ERROR)
    {
        printf("Erreur d'envoie au serveur\n");
    }
    free(Personne);
    printf("Enregistrement effectué. Bienvenue\n");
    chat(fd, py);
    return 1;
}

void authentification(int fd, int py)
{
    // system("clear");
    printf("----- Starting ------ \n");
    printf("Bonjour bienvenue dans notre client de conversation.\n");
    printf("1 - Connexion\n");
    printf("2 - Nouveau venue ? Créer un compte\n");
    printf("3 - Quitter\n");
    // char answer[2];
    //  memset(answer,0,sizeof(answer));
    /*scanf("%s", answer);
    printf("%s\n", answer);
    char c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; // Clear input buffer
    */
    char answer[10];
    size_t bytes;
    bytes = recv(py, answer, sizeof(answer), 0);
    if (bytes <= 0)
    {
        fprintf(stdout, "erreur de reception 397\n");
    }
    answer[bytes] = '\0';

    if (strcmp(answer, "1") == 0 || (strcmp(answer, "2") == 0))
    {
        send(fd, answer, sizeof(answer), 0);
        if (strcmp(answer, "1") == 0)
        {
            sign_in(fd, py);
        }
        if (strcmp(answer, "2") == 0)
        {
            new_account(fd, py);
        }

        // un thread de lire et un thread d'ecriture ( regarde le code de partie un dans exec2 ) le descripteur maintenant sera fd (pas de pipes) et utiliser send et recv
    }
    else if (strcmp(answer, "3") == 0)
    {
        nettoie();
        send(fd, answer, sizeof(answer), 0);
        printf("A bientôt ! \n");
        sleep(1);
        exit(22);
    }
    else
    {

        authentification(fd, py);
    }
}
int main()
{
    if (signal(SIGINT, sighandler) == SIG_ERR)
    {
        perror("Erreur lors de la mise en place du gestionnaire de signal");
        return EXIT_FAILURE;
    }
    MT = (myThreads *)malloc(sizeof(myThreads));
    MT->thread_ecrire = 0;
    MT->thread_lire = 0;
    int fd, py;
    struct sockaddr_in server_address, py_address;
    // authentification();

    // Création du socket
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket échoué");
        exit(EXIT_FAILURE);
    }
    // Création du socket
    py = socket(AF_INET, SOCK_STREAM, 0);
    if (py == -1)
    {
        printf("Could not create socket");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_1);
    // Convertir l'adresse IP de format texte en format binaire
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
    {
        perror("inet_pton échoué");
        exit(EXIT_FAILURE);
    }

    py_address.sin_family = AF_INET;
    py_address.sin_port = htons(PORT_py);
    // Convertir l'adresse IP de format texte en format binaire
    if (inet_pton(AF_INET, SERVER_PY, &py_address.sin_addr) <= 0)
    {
        perror("inet_pton échoué");
        exit(EXIT_FAILURE);
    }
    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connexion échouée");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (connect(py, (struct sockaddr *)&py_address, sizeof(py_address)) < 0)
        {
            perror("connexion py échouée");
            exit(EXIT_FAILURE);
        }
        else
        {
            py_fd = py;
            client_fd = fd;
            printf("connection\n");
            authentification(fd, py);
            free(MT);
            return 1;
        }
    }
    return 0;
}
