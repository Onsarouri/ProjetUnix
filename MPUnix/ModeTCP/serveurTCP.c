#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER_SIZE 1024

void getDateTime(char *result);
void listFiles(char *result);
void getFileContent(char *filename, char *result);
void handleClient(int client_socket);
void handle_error(const char *msg);

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        handle_error("Error creating socket");
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        handle_error("setsockopt(SO_REUSEADDR) failed");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1235);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        handle_error("Error binding socket");
    }

    if (listen(server_socket, 5) < 0) {
        handle_error("Error listening for connections");
    }

    printf("Server is listening for connections...\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t pid = fork();

        if (pid < 0) {
            handle_error("Error forking process");
        } else if (pid == 0) {
            close(server_socket);
            handleClient(client_socket);
            close(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            close(client_socket);
            waitpid(pid, NULL, 0);
        }
    }

    close(server_socket);

    return 0;
}



void getDateTime(char *result) {
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    strftime(result, MAX_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", tm_info);
}

void listFiles(char *result) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if (dir == NULL) {
        perror("Error opening directory");
        exit(1);
    }

    result[0] = '\0'; // Clear the result string

    while ((entry = readdir(dir)) != NULL) {
        strcat(result, entry->d_name);
        strcat(result, "\n");
    }

    closedir(dir);
}

void getFileContent(char *filename, char *result) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        strcpy(result, "File not found");
    } else {
        size_t bytesRead = fread(result, 1, MAX_BUFFER_SIZE, file);
        result[bytesRead] = '\0';
        fclose(file);
    }
}

void handleClient(int client_socket) {
    time_t start_time;
    time(&start_time);  // Record the start time for each client connection

    while (1) {
        int choice;
        ssize_t bytes_received = read(client_socket, &choice, sizeof(choice));

        if (bytes_received < 0) {
            perror("Error receiving client choice");
            close(client_socket);
            return;
        } else if (bytes_received == 0) {
            printf("Client closed the connection\n");
            break;
        }

        if (choice < 1 || choice > 5) {
            // Invalid request
            printf("Received request: Invalid Request\n");
            char error_message[] = "Invalid option. Please enter a valid option.\n";
            size_t error_message_len = strlen(error_message) + 1;
            write(client_socket, &error_message_len, sizeof(error_message_len));
            write(client_socket, error_message, error_message_len);
            continue;
        }

        switch (choice) {
            case 1:
                // Get Date and Time
                printf("Received request: Get Date and Time\n");
                char datetime[MAX_BUFFER_SIZE];
                getDateTime(datetime);
                size_t datetime_len = strlen(datetime) + 1;
                write(client_socket, &datetime_len, sizeof(datetime_len));
                write(client_socket, datetime, datetime_len);
                break;

            case 2:
                // List Files
                printf("Received request: List Files\n");
                char file_list[MAX_BUFFER_SIZE];
                listFiles(file_list);
                size_t file_list_len = strlen(file_list) + 1;
                write(client_socket, &file_list_len, sizeof(file_list_len));
                write(client_socket, file_list, file_list_len);
                break;

            case 3: {
                // Get File Content
                printf("Received request: Get File Content\n");
                char filename[MAX_BUFFER_SIZE];
                ssize_t filename_size = read(client_socket, &filename, sizeof(filename));

                if (filename_size <= 0) {
                    perror("Error receiving filename");
                    close(client_socket);
                    return;
                }

                // Null-terminate the received filename
                filename[filename_size] = '\0';

                char file_content[MAX_BUFFER_SIZE];
                getFileContent(filename, file_content);
                size_t file_content_len = strlen(file_content) + 1;

                // Send the size and content of the file to the client
                write(client_socket, &file_content_len, sizeof(file_content_len));
                write(client_socket, file_content, file_content_len);

                break;
            }

            case 4:
                // Get Connection Duration
                printf("Received request: Get Connection Duration\n");

                time_t end_time;
                time(&end_time);  // Record the end time

                double duration = difftime(end_time, start_time);

                // Send the duration to the client
                size_t duration_size = sizeof(double);
                write(client_socket, &duration_size, sizeof(duration_size));
                write(client_socket, &duration, duration_size);

                break;


            case 5:
                // Quit
                printf("Received request: Quit\n");
                close(client_socket);
                return;
        }
    }
}


void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
