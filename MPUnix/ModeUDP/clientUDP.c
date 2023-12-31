#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define NMAX 10

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <serveur_address> <serveur_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serveur_address = argv[1];
    int serveur_port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serveur_addr;
    memset(&serveur_addr, 0, sizeof(serveur_addr));
    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_port = htons(serveur_port);
    inet_pton(AF_INET, serveur_address, &serveur_addr.sin_addr);

    int nombres[NMAX];
    for (int i = 0; i < NMAX; ++i) {
        nombres[i] = rand() % NMAX + 1;
    }

    printf("Nombres aléatoires générés par le client et envoyés au serveur:\n");
    for (int i = 0; i < NMAX; ++i) {
        printf("%d ", nombres[i]);
    }
    printf("\n");

    if (sendto(sock, nombres, sizeof(int) * NMAX, 0, (struct sockaddr *)&serveur_addr, sizeof(serveur_addr)) == -1) {
        perror("Erreur lors de l'envoi des données au serveur");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Données envoyées avec succès au serveur.\n");

    int nombres_tries[NMAX];

    if (recvfrom(sock, nombres_tries, sizeof(int) * NMAX, 0, NULL, NULL) == -1) {
        perror("Erreur lors de la réception des données triées du serveur");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Nombres triés reçus du serveur:\n");
    for (int i = 0; i < NMAX; ++i) {
        printf("%d ", nombres_tries[i]);
    }
    printf("\n");

    close(sock);

    return 0;
}
