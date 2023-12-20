#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#define NMAX 10

volatile sig_atomic_t timeout_flag = 0;

void timeout_handler(int signo) {
    timeout_flag = 1;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_address> <server_port>\n", argv[0]);
        exit(1);
    }

    const char *server_address = argv[1];
    const int server_port = atoi(argv[2]);

    // Création de la socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Erreur lors de la création de la socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Utilisation de getaddrinfo pour résoudre l'adresse du serveur
    struct addrinfo hints, *result, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_address, NULL, &hints, &result) != 0) {
        fprintf(stderr, "Erreur lors de la résolution de l'adresse du serveur\n");
        close(sockfd);
        exit(1);
    }

    for (p = result; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            memcpy(&server_addr.sin_addr, &((struct sockaddr_in *)p->ai_addr)->sin_addr, sizeof(struct in_addr));
            break;
        }
    }
    freeaddrinfo(result);

    // Génération d'un nombre aléatoire
    int n = rand() % NMAX + 1;

    // Envoi du nombre au serveur
    sendto(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Set up a signal handler for timeout
    struct sigaction sa;
    sa.sa_handler = timeout_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    // Set a timeout (e.g., 2 seconds)
    alarm(2);

    // Réception de la réponse du serveur
    int sorted_number;
    recvfrom(sockfd, &sorted_number, sizeof(sorted_number), 0, NULL, NULL);

    // Reset the alarm
    alarm(0);

    if (timeout_flag) {
        printf("Timeout: No response from the server\n");
    } else {
        // Affichage de la réponse
        printf("Nombre trié reçu du serveur: %d\n", sorted_number);
    }

    // Fermeture de la socket
    close(sockfd);

    return 0;
}
