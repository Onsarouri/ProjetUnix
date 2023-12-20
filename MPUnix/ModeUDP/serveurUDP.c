#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define NMAX 10

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_port>\n", argv[0]);
        exit(1);
    }

    const int server_port = atoi(argv[1]);

    // Création de la socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Erreur lors de la création de la socket");
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    // Liaison de la socket à l'adresse du serveur
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Erreur lors de la liaison de la socket");
    }

    while (1) {
        printf("Serveur en attente...\n");

        int n;
        // Réception du nombre du client
        ssize_t bytes_received = recvfrom(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&client_addr, &(socklen_t){sizeof(client_addr)});

        if (bytes_received < 0) {
            // Handle the error (printing an error message, etc.)
            perror("Erreur lors de la réception des données du client");
            close(sockfd);
            exit(1);
        } else if (bytes_received == 0) {
            // Client closed the connection, gracefully exit
            printf("Client a fermé la connexion\n");
            close(sockfd);
            exit(0);
        }

        // Génération de NMAX nombres aléatoires
        int random_numbers[NMAX];
        for (int i = 0; i < NMAX; i++) {
            random_numbers[i] = rand() % 100 + 1; // Modify the range as needed
        }

        // Tri des nombres générés
        qsort(random_numbers, NMAX, sizeof(int), compare_ints);

        // Envoi des nombres triés au client
        sendto(sockfd, random_numbers, sizeof(random_numbers), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }

    // The code should not reach here, as the server is intended to run indefinitely
    return 0;
}
