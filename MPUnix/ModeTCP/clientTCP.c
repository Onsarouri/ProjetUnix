#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

// Function prototypes
void getServerResponse(int server_socket);
void displayMenu();

int main() {
    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address and port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1235);  // Replace with your desired port
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    // Authentication (replace with actual authentication logic)
    // authenticate(client_socket);


    while (1) {
        displayMenu();

        int choice;
        scanf("%d", &choice);

        // Send user choice to the server
        ssize_t bytes_sent = write(client_socket, &choice, sizeof(choice));

        if (bytes_sent < 0) {
            perror("Error sending choice to server");
            close(client_socket);
            exit(EXIT_FAILURE);
        } else if (bytes_sent < sizeof(choice)) {
            fprintf(stderr, "Error: Sent fewer bytes than expected\n");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        if (choice == 5) {
            // Quit
            break;
        }

        // Handle server response based on user choice
        if (choice == 3) {
            // Get File Content
            printf("\nEnter the filename: ");
            char filename[MAX_BUFFER_SIZE];
            scanf("%s", filename);

            // Send the filename to the server
            write(client_socket, filename, strlen(filename) + 1);
        }

        getServerResponse(client_socket);
    }

    // Close the socket
    close(client_socket);

    return 0;
}

void getServerResponse(int server_socket) {
    // Receive and display server response
    size_t response_size;
    ssize_t bytes_received = read(server_socket, &response_size, sizeof(response_size));

    if (bytes_received < 0) {
        perror("Error receiving server response size");
        return;
    } else if (bytes_received == 0) {
        printf("Server closed the connection\n");
        return;
    }

    char *response_data = (char *)malloc(response_size);
    bytes_received = read(server_socket, response_data, response_size);

    if (bytes_received < 0) {
        perror("Error receiving server response data");
        free(response_data);
        return;
    }

    // Handle different response types based on the received data
    if (response_size == sizeof(double)) {
        // If the response size is equal to the size of a double, assume it's the duration
        double duration;
        memcpy(&duration, response_data, sizeof(double));
        printf("Server response: Connection duration: %.2f seconds\n", duration);
    } else if (response_size > 0) {
        // If the response size is greater than 0, assume it's a string
        printf("Server response:\n%s\n", response_data);
    } else {
        // If the response size is 0, assume it's a special case (e.g., file content request)
        printf("Server response:\n");

        // Prompt the user for the filename
        char filename[MAX_BUFFER_SIZE];
        printf("Enter the filename: ");
        scanf("%s", filename);

        // Send the filename to the server
        write(server_socket, filename, strlen(filename) + 1);

        // Receive and display the file content
        getServerResponse(server_socket);
    }

    free(response_data);
}





void displayMenu() {
    printf("----------------------");
    printf("\nMenu:\n");
    printf("1. Get Date and Time\n");
    printf("2. List Files\n");
    printf("3. Get File Content\n");
    printf("4. Get Connection Duration\n");
    printf("5. Quit\n");
    printf("----------------------\n");
    printf("Enter your choice: ");
}
