#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../server/actions.h"
#include <string.h>

#define PORT 8080
#define IP "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    float distance;
    int fuel_percent, tire_usage;
    char current_tire[20];
    char fuel_string[10];
    char input[20];
    char output[100];

    while (1) {
        // Buka socket untuk server
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        // Coba melakukan koneksi ke server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // Meminta input dari pengguna
        printf("Command: ");
        scanf("%s", input);

        if (strcmp(input, "Gap") == 0 || strcmp(input, "gap") == 0) {
            printf("Info: ");
            scanf("%f", &distance);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            // Mengonversi float ke string
            char distance_str[20];
            sprintf(distance_str, "%f", distance);

            // Mengirimkan string distance ke server
            if (write(sock, distance_str, sizeof(distance_str)) < 0) {
                perror("Write failed");
                exit(EXIT_FAILURE);
            }

        }
        else if (strcmp(input, "Fuel") == 0 || strcmp(input, "fuel") == 0) {
            printf("Info: ");
            scanf("%s", fuel_string);
            sscanf(fuel_string, "%d%%", &fuel_percent);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, &fuel_percent, sizeof(fuel_percent)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "Tire") == 0 || strcmp(input, "tire") == 0) {
            printf("Info: ");
            scanf("%d", &tire_usage);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, &tire_usage, sizeof(tire_usage)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "TireChange") == 0) {
            printf("Info: ");
            scanf("%s", current_tire);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, current_tire, sizeof(current_tire)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else {
            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        // Menerima hasil dari server
        if (read(sock, output, sizeof(output)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        printf("[Paddock]: %s\n", output);

        close(sock);
    }

    return 0;
}
