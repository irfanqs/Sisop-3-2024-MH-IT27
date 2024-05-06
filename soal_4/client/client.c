#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define size_buffer 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[size_buffer];
    char response[2048];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(8080);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error connecting to server");
        return 1;
    }

    printf("Connected to server. Enter commands ('exit' to quit):\n");

    while (1) {

        printf("\nYou> ");
        fgets(buffer, size_buffer, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send(client_socket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        int bytes_received = recv(client_socket, response, sizeof(response) - 1, 0);
        if (bytes_received < 0) {
            perror("recv failed");
            break;
        } else if (bytes_received == 0) {
            printf("Server closed the connection.\n");
            break;
        }

        response[bytes_received] = '\0'; 

        printf("Server:\n");
        char *token = strtok(response, "\n");
        while (token != NULL) {
            printf("%s\n", token);
            token = strtok(NULL, "\n");
        }
        printf("\n");
        if (strstr(response, "Connection closed.") != NULL) {
            break;
        }
    }
    close(client_socket);
    return 0;
    
}
