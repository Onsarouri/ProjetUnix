#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <time.h>

#define MAX_BUFFER_SIZE 1024

GtkWidget *filenameLabel;
GtkWidget *filenameEntry;
int choice;
GtkWidget *window;
GtkWidget *menuComboBox;
GtkWidget *resultTextView;
int client_socket;
time_t start_time = 0;

void updateTextView(const char *text);
void getServerResponse(int server_socket);
void *socketThread(void *arg);
void sendRequest(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "TCP Client");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *label = gtk_label_new("Choose an option:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    menuComboBox = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menuComboBox), "Get Date and Time");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menuComboBox), "List Files");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menuComboBox), "Get File Content");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menuComboBox), "Get Connection Duration");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menuComboBox), "Quit");
    gtk_combo_box_set_active(GTK_COMBO_BOX(menuComboBox), 0);
    gtk_box_pack_start(GTK_BOX(vbox), menuComboBox, FALSE, FALSE, 0);

    filenameLabel = gtk_label_new("Please enter the file name:");
    gtk_widget_hide(filenameLabel);
    gtk_box_pack_start(GTK_BOX(vbox), filenameLabel, FALSE, FALSE, 0);

    filenameEntry = gtk_entry_new();
    gtk_widget_hide(filenameEntry);
    gtk_box_pack_start(GTK_BOX(vbox), filenameEntry, FALSE, FALSE, 0);

    GtkWidget *button = gtk_button_new_with_label("Submit");
    g_signal_connect(button, "clicked", G_CALLBACK(sendRequest), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    resultTextView = gtk_text_view_new();
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), resultTextView);
    gtk_box_pack_start(GTK_BOX(vbox), scrolledWindow, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    // Set up the initial connection time
    time(&start_time);
    gtk_main();

    return 0;
}

void updateTextView(const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(resultTextView));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, text, -1);
}

void getServerResponse(int server_socket) {
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

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(resultTextView));
    gtk_text_buffer_set_text(buffer, "", 0);

    if (response_size == sizeof(double)) {
        double duration;
        memcpy(&duration, response_data, sizeof(double));
        updateTextView(g_strdup_printf("Server response: Connection duration: %.2f seconds\n", duration));
    } else if (response_size > 0) {
        updateTextView(response_data);

        if (strcmp(response_data, "Enter the filename:") == 0) {
            gtk_widget_show(filenameLabel);
            gtk_widget_show(filenameEntry);
        }
    } else {
        updateTextView("Server response: Empty response\n");
    }

    free(response_data);
}

void *socketThread(void *arg) {
    getServerResponse(client_socket);
    return NULL;
}


void sendRequest(GtkWidget *widget, gpointer data) {
    choice = gtk_combo_box_get_active(GTK_COMBO_BOX(menuComboBox)) + 1;

    // Get the filename from the entry field
    const char *filename = gtk_entry_get_text(GTK_ENTRY(filenameEntry));

    // Hide the filename label and entry initially
    gtk_widget_hide(filenameLabel);
    gtk_widget_hide(filenameEntry);

    // Close the existing socket if it's open
    if (client_socket > 0) {
        close(client_socket);
    }

    // Replace the following lines with your logic to send the request to the server
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    if (choice == 4) {  // Get Connection Duration
        // Send a placeholder value to the server
        double placeholder = 0.0;
        bytes_sent = write(client_socket, &placeholder, sizeof(double));
        if (bytes_sent < 0) {
            perror("Error sending placeholder to server");
            close(client_socket);
            exit(EXIT_FAILURE);
        } else if (bytes_sent < sizeof(double)) {
            fprintf(stderr, "Error: Sent fewer bytes than expected\n");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    } else if (choice == 3) {  // Get File Content
        // Dynamically show/hide filename label and entry based on the selected option
        gtk_widget_show(filenameLabel);
        gtk_widget_show(filenameEntry);

        // Send the filename to the server
        write(client_socket, filename, strlen(filename) + 1);
    }

    if (choice == 5) {
        // Quit
        close(client_socket);
        gtk_main_quit();
        return;
    }

    // Handle server response based on user choice in a separate thread
    pthread_t thread;
    if (pthread_create(&thread, NULL, socketThread, NULL)) {
        perror("Error creating thread");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Detach the thread to allow it to run independently
    pthread_detach(thread);
}