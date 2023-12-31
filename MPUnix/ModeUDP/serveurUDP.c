#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define NMAX 10

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <serveur_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serveur_port = atoi(argv[1]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serveur_addr, client_addr;
    memset(&serveur_addr, 0, sizeof(serveur_addr));
    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveur_addr.sin_port = htons(serveur_port);

    if (bind(sock, (struct sockaddr *)&serveur_addr, sizeof(serveur_addr)) == -1) {
        perror("Erreur lors du bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de données...\n");

    int nombres[NMAX];

    socklen_t client_addr_len = sizeof(client_addr);
    if (recvfrom(sock, nombres, sizeof(int) * NMAX, 0, (struct sockaddr *)&client_addr, &client_addr_len) == -1) {
        perror("Erreur lors de la réception des données du client");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Nombres reçus du client:\n");
    for (int i = 0; i < NMAX; ++i) {
        printf("%d ", nombres[i]);
    }
    printf("\n");

    qsort(nombres, NMAX, sizeof(int), compare);

    printf("Envoi de %d nombres triés au client...\n", NMAX);

    if (sendto(sock, nombres, sizeof(int) * NMAX, 0, (struct sockaddr *)&client_addr, client_addr_len) == -1) {
        perror("Erreur lors de l'envoi des nombres triés au client");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Données triées envoyées avec succès au client.\n");

    close(sock);

    return 0;
}
