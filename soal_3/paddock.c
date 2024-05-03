#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include "actions.h"

#define PORT 8000

void signal_handler(int signum) {
    if (signum == SIGINT) {
        // Lakukan apa pun yang diperlukan saat menerima sinyal SIGINT (misalnya, tutup socket dan keluar dari program)
        exit(EXIT_SUCCESS);
    }
}

void write_to_log_float(const char *client_info, const char *server_info, const char *input, float quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%.1f]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log(const char *client_info, const char *server_info, const char *input, int quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%d]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_string(const char *client_info, const char *server_info, const char *input, const char *quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%s]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_percent(const char *client_info, const char *server_info, const char *input, int quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%d%%]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_error(const char *client_info, const char *server_info, const char *input, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s]\n", client_info, time_string, input);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

int main() {
    pid_t pid, sid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Atur signal handler untuk menangani SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    // Tutup file descriptor standar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Buka kembali stdin, stdout, dan stderr ke /dev/null
    int stdin_fd = open("/dev/null", O_RDWR);
    if (stdin_fd == -1) {
        perror("Failed to reopen stdin");
        exit(EXIT_FAILURE);
    }
    int stdout_fd = dup(stdin_fd);
    int stderr_fd = dup(stdin_fd);

    // Buka socket untuk server
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // START CODE
    float distance;
    int fuel_percent, tire_usage;
    char current_tire[20];
    char input[20];
    char output[100];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("172.23.143.99");
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Menerima data dari client (input)
        if (read(new_socket, input, sizeof(input)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        char output[100];
        if (strcmp(input, "Gap") == 0 || strcmp(input, "gap") == 0) {
            // Membaca string distance dari client
            char distance_str[20];
            if (read(new_socket, distance_str, sizeof(distance_str)) < 0) {
                perror("Read failed");
                exit(EXIT_FAILURE);
            }

            // Menambahkan karakter null pada akhir string
            distance_str[sizeof(distance_str) - 1] = '\0';

            // Mengonversi string distance ke float
            float distance;
            sscanf(distance_str, "%f", &distance);

            gap(distance, output);
            write_to_log_float("Driver", "Paddock", "Gap", distance, output);
            // Mengirimkan hasil ke client
            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }

        else if (strcmp(input, "Fuel") == 0 || strcmp(input, "fuel") == 0) {
            if (read(new_socket, &fuel_percent, sizeof(fuel_percent)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            fuel(fuel_percent, output);
            write_to_log_percent("Driver", "Paddock", "Fuel", fuel_percent, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        } 

        else if (strcmp(input, "Tire") == 0 || strcmp(input, "tire") == 0) {
            if (read(new_socket, &tire_usage, sizeof(tire_usage)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            tire(tire_usage, output);
            write_to_log("Driver", "Paddock", "Tire", tire_usage, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "TireChange") == 0) {
            if (read(new_socket, current_tire, sizeof(current_tire)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            tire_change(current_tire, output);
            write_to_log_string("Driver", "Paddock", "Tire Change", current_tire, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        } else {
            sprintf(output, "Invalid input!");
            write_to_log_error("Driver", "Paddock", input, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        close(new_socket);
    }

    close(server_fd);
    return 0;
}