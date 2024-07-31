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
#define PORT 7777
#define SERVER_IP "127.0.0.1"
#define MAX_CHAR 50
void authentification(int fd);
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

    size_t bytesread;
    fprintf(stdout, "thread exec2 lire en cours ...%ld\n", pthread_self());
    fflush(stdout);
    // memset(buffer, 0, sizeof(buffer));
    while ((bytesread = recv(fd, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytesread] = '\0';
        printf("%s", buffer);
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
    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        clearPreviousLine();
        printf("[Vous] : %s", buffer);
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
}
void chat(int fd)
{

    if (pthread_create(&MT->thread_ecrire, NULL, writeth, (void *)&fd) != 0 || pthread_create(&MT->thread_lire, NULL, readth, (void *)&fd) != 0)
    {
        perror("THREAD :");
        exit(0);
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

void get_string_send_toserver(char pseudo[], int fd)
{

    if (fgets(pseudo, MAX_CHAR, stdin) == NULL)
    {
        printf("Erreur de lecture du pseudo\n");
    }
    char *newline_pos = strchr(pseudo, '\n');
    if (newline_pos != NULL)
    {
        // Remplacer le caractère de retour à la ligne par un caractère nul
        *newline_pos = '\0';
    }
    if (send(fd, pseudo, strlen(pseudo), 0) <= 0)
    {
        printf("Erreur d'envoi du pseudo\n");
    }
}
void chack_validite_pseudo(char pseudo[], int fd)
{
    char flag[10];

    size_t bytesread;
    get_string_send_toserver(pseudo, fd);
    while (((bytesread = recv(fd, flag, sizeof(flag), 0)) > 0))
    {
        flag[bytesread] = '\0';
        printf("Flag reçue %s\n",flag);
        if (strcmp(flag, "0") == 0)
        {
            printf("Veuillez choisir un autre cela existe deja :)\n");
            get_string_send_toserver(pseudo, fd);
        }
        else if (strcmp(flag, "1") == 0)
        {
            //Case success
            fprintf(stdout, "check_validite : %s \n", pseudo);
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
void get_password(client_info *Personne)
{
    char pswrd[256];
    char output[33];
    if (fgets(pswrd, MAX_CHAR, stdin) == NULL)
    {
        printf("Erreur lecture\n");
    }
    char *newline_pos = strchr(pswrd, '\n');
    if (newline_pos != NULL)
    {
        // Remplacer le caractère de retour à la ligne par un caractère nul
        *newline_pos = '\0';
    }
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
void get_pseudo_and_send_to_server(char pseudo[], int fd)
{
    char flag[10];

    size_t bytesread;
    get_string_send_toserver(pseudo, fd);
    /*En attente de deux vérification
     * 1) Si le pseudo correspond à un login déjà connecté
     * 2° Si le login est dans DATA
     */
    while (((bytesread = recv(fd, flag, sizeof(flag), 0)) > 0))
    {
        flag[bytesread] = '\0';
        if (strcmp(flag, "||") == 0)
        {
            printf("Vous êtes déjà connecté à ce serveur\n");
            authentification(fd); // On recommence le process
        }
        if (strcmp(flag, "0") == 0)
        {
            printf("Pseudo inconnue, vous devriez peut-être vous inscrire\n");
            authentification(fd);
        }
        else if (strcmp(flag, "1") == 0)
        {

            fprintf(stdout, "Entrer votre mot de passe ");
            break;
        }
        else
        {
            // fprintf(stdout, "checkout here 243\n");
            // fprintf(stdout, "%s\n", flag);
        }
    }
}
void get_password_and_hach(char Password[], size_t size)
{
    char pswrd[256];
    char output[33];
    if (fgets(pswrd, MAX_CHAR, stdin) == NULL)
    {
        printf("Erreur lecture\n");
    }
    char *newline_pos = strchr(pswrd, '\n');
    if (newline_pos != NULL)
    {
        // Remplacer le caractère de retour à la ligne par un caractère nul
        *newline_pos = '\0';
    }
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
    // fprintf(stdout, "%s\n", Password);
}

uint32_t sign_in(int fd)
{

    char pseudo[MAX_CHAR];
    char flag[10];
    printf("Commencer par entrer vos infomations personnelles\n");
    printf("Pseudo: ");
    get_pseudo_and_send_to_server(pseudo, fd);
    char Password[64];
    get_password_and_hach(Password, sizeof(Password));
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
        printf("\n ***************   Bienvenue dans le chat ***************   \n");
        chat(fd);
    }
    else if (strcmp(flag, "0") == 0)
    {
        fprintf(stdout, "Mot de passe incorrect, veuiller reésayer \n");
        authentification(fd);
    }
    else
    {
        fprintf(stdout, "erreur 322\n");
    }
    // quand c'est valide on le capture

    return 1;
}
uint32_t new_account(int fd)
{

    client_info *Personne = new_infos_client();
    char pseudo[MAX_CHAR];

    printf("Commencer par entrer vos infomations personnelles\n");
    printf("Pseudo: ");

    chack_validite_pseudo(pseudo, fd);
    strcpy(Personne->pseudo, pseudo);
    fprintf(stdout, "chosen pseudo is : %s %s\n", pseudo, Personne->pseudo);
    // quand c'est valide on le capture

    // mot de passe pas besoin de controle
    printf("Mot de passe: ");
    get_password(Personne);
    printf("Enregistrement des informations dans la BDD users... \n");

    // On envoie une instance de struct client_info aux serveur
    if (send(fd, Personne, sizeof(*Personne), 0) == SO_ERROR)
    {
        printf("Erreur d'envoie au serveur\n");
    }
    free(Personne);
    printf("Enregistrement effectué. Bienvenue\n");
    chat(fd);
    return 1;
}

void authentification(int fd)
{
    printf("----- Starting ------ \n");
    printf("Bonjour bienvenue dans notre client de conversation.\n");
    printf("1 - Connexion\n");
    printf("2 - Nouveau venue ? Créer un compte\n");
    printf("3 - Quitter\n");
    char answer[2];
    scanf("%s", answer);
    char c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (strcmp(answer, "1") == 0 || (strcmp(answer, "2") == 0))
    {
        send(fd, answer, sizeof(answer), 0);
        if (strcmp(answer, "1") == 0)
        {
            sign_in(fd);
        }
        if (strcmp(answer, "2") == 0)
        {
            new_account(fd);
        }
    }
    else if (strcmp(answer, "3") == 0)
    {
        nettoie();
        send(fd, answer, sizeof(answer), 0);
        printf("A bientôt ! \n");
        sleep(1);
        exit(22);
    }
    else // Si l'user n'écrit pas correctement
    {
        authentification(fd);
    }
}
int main()
{
    printf("Connection en cours\n");
    if (signal(SIGINT, sighandler) == SIG_ERR)
    {
        perror("Erreur lors de la mise en place du gestionnaire de signal");
        return EXIT_FAILURE;
    }
    MT = (myThreads *)malloc(sizeof(myThreads));
    MT->thread_ecrire = 0;
    MT->thread_lire = 0;
    int fd;
    struct sockaddr_in server_address;

    // Création du socket
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket échoué");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    // Convertir l'adresse IP de format texte en format binaire
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
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
        client_fd = fd;
        printf("Connection établie\n");
        authentification(fd);
        free(MT);
        return 1;
    }
}
