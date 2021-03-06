/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "my_linux_types.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

int main(int argc, char **argv)
{

    int socket_descriptor,      /* descripteur de socket */
        longueur;               /* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; /* adresse de socket local */
    hostent *ptr_host;          /* info sur une machine hote */
    servent *ptr_service;       /* info sur service */
    char buffer[256];
    char *host;   /* nom de la machine distante */
    char mesg[256];   /* message envoyé */
    char *pseudo; /* identifiant de l'utilisateur */
    char wholeMsg[512]; /* le message complet avec le pseudo */
    bool stop = false;    /* deconnexion du client */

    if (argc != 3)
    {
        perror("usage : client <adresse-serveur> <pseudo>");
        exit(1);
    }

    host = argv[1];
    pseudo = argv[2];

    printf("adresse du serveur  : %s \n", host);
    printf("pseudo envoye       : %s \n", pseudo);


    if ((ptr_host = gethostbyname(host)) == NULL)
    {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }

    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char *)ptr_host->h_addr, (char *)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */

    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */

    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
        if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
        perror("erreur : impossible de recuperer le numero de port du service desire.");
        exit(1);
        }
        adresse_locale.sin_port = htons(ptr_service->s_port);
        */
    /*-----------------------------------------------------------*/

    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/

    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("erreur : impossible de creer la socket de connexion avec le serveur.");
        exit(1);
    }

    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr *)(&adresse_locale), sizeof(adresse_locale))) < 0)
    {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }

    printf("connexion etablie avec le serveur. \n");

    char mesgtemp;

    do
    {
        memset(wholeMsg, 0, 512);
        memset(mesg, 0, 256);

        printf("Nouveau message : \n");
        scanf("%s", mesg);

        printf("----- \n");
        printf("%s", mesg);
        printf("\n");
        printf("----- \n");

        if (strcmp("stop", mesg) == 0)
        {
            stop = true;
        }
        
        strcpy(wholeMsg, pseudo);
        strcat(wholeMsg, " : "); 
        strcat(wholeMsg, mesg);

        printf("envoi d'un message au serveur. \n");

        /* envoi du message vers le serveur */
        if ((write(socket_descriptor, wholeMsg, strlen(wholeMsg))) < 0)
        {
            perror("erreur : impossible d'ecrire le message destine au serveur.");
            exit(1);
        }

        /* mise en attente du prgramme pour simuler un delai de transmission */
        sleep(3);

        printf("message envoye au serveur. \n");

        /* lecture de la reponse en provenance du serveur */
        while ((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0)
        {
            printf("reponse du serveur : \n");
            write(1, buffer, longueur);
        }

        printf("\nfin de la reception.\n");
    } while (stop != true);

    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");

    exit(0);
}