#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>

#define maks_clients 10
#define size_buffer 1024

void send_response(int client_socket, const char *message) {
    char response[size_buffer];
    snprintf(response, size_buffer, "%s\n", message);
    send(client_socket, response, strlen(response), 0);
}

//mencatat di file Change.log
void log_change(FILE *log_file, const char *type, const char *detail) {
    time_t now;
    struct tm *tm_info;
    char timestamp[20];
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y", tm_info);
    char log_entry[size_buffer];
    sprintf(log_entry, "[%s] [%s] %s\n", timestamp, type, detail);
    log_entry[strcspn(log_entry, "\n")] = ' ';
    fputs(log_entry, log_file);
    fflush(log_file);
}

void handle_client(int client_socket, FILE *anime_file, FILE *log_file) {
    char buffer[size_buffer];
    int read_bytes;

    while ((read_bytes = read(client_socket, buffer, size_buffer - 1)) > 0) {
        buffer[read_bytes] = '\0';  

    printf("Received input from client: %s\n", buffer);
        if (buffer[read_bytes - 1] == '\n')
            buffer[read_bytes - 1] = '\0';

        char *command = strtok(buffer, " ");
        char *argument = strtok(NULL, "\n");

        if (command == NULL) {
            send_response(client_socket, "Invalid Command\n");
            continue;
        }

        if (strcmp(command, "exit") == 0) {
            printf("Exit command received. Closing connection.\n");
            send_response(client_socket, "Connection closed.\n");
            break;
        }

        if (strcmp(command, "tampilkan") == 0) {
            char response[size_buffer] = ""; 
            char line[size_buffer];
            int count = 1; 
            rewind(anime_file);
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *token = strtok(line, ",");
                token = strtok(NULL, ","); 
                token = strtok(NULL, ","); 
                token[strcspn(token, "\n")] = '\0'; 
                char formatted_line[size_buffer];
                sprintf(formatted_line, "%2d. %s\n", count++, token); 
                strcat(response, formatted_line); 
            }
            send_response(client_socket, response);
        } else if (strcmp(command, "hari") == 0) {
            if (argument == NULL) {
                send_response(client_socket, "Invalid Command\n");
                continue;
            }
            char response[size_buffer] = ""; 
            char line[size_buffer];
            int count = 1;
            rewind(anime_file);
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *token = strtok(line, ",");
                if (strcmp(token, argument) == 0) {
                    token = strtok(NULL, ","); 
                    token = strtok(NULL, ","); 
                    token[strcspn(token, "\n")] = '\0'; 
                    char formatted_line[size_buffer];
                    sprintf(formatted_line, "%2d. %s\n", count++, token); 
                    strcat(response, formatted_line);
                }
            }
            if (strlen(response) == 0) {
                send_response(client_socket, "No anime found for the specified day\n");
            } else {
                send_response(client_socket, response);
            }
        } else if (strcmp(command, "genre") == 0) {
            if (argument == NULL) {
                send_response(client_socket, "Invalid Command\n");
                continue;
            }
            char response[size_buffer] = ""; 
            char line[size_buffer];
            int count = 1; 
            rewind(anime_file);
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *token = strtok(line, ",");
                token = strtok(NULL, ","); 
                if (strcmp(token, argument) == 0) {
                    token = strtok(NULL, ","); 
                    token[strcspn(token, "\n")] = '\0';
                    char formatted_line[size_buffer];
                    sprintf(formatted_line, "%2d. %s\n", count++, token); 
                    strcat(response, formatted_line); 
                }
            }
            if (strlen(response) == 0) {
                send_response(client_socket, "No anime found for the specified genre\n");
            } else {
                send_response(client_socket, response);
            }
        } else if (strcmp(command, "status") == 0) {
            if (argument == NULL) {
                send_response(client_socket, "Invalid Command\n");
                continue;
            }
            char response[size_buffer] = ""; 
            char line[size_buffer];
            rewind(anime_file);
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *day = strtok(line, ",");
                char *genre = strtok(NULL, ",");
                char *title = strtok(NULL, ",");
                char *status = strtok(NULL, ",");
                
                while (strtok(NULL, ",") != NULL) {
                    strcat(title, strtok(NULL, ","));
                }

                char *clean_title = strtok(title, "\n");
                for (int i = 0; clean_title[i]; i++) {
                    clean_title[i] = tolower(clean_title[i]);
                }
                for (int i = 0; argument[i]; i++) {
                    argument[i] = tolower(argument[i]);
                }
                if (strcasecmp(clean_title, argument) == 0) { 
                    sprintf(response, "%s\n", status);
                    break;
                }
            }
            if (strlen(response) == 0) {
                send_response(client_socket, "Anime not found\n");
            } else {
                send_response(client_socket, response);
            }
        } else if (strcmp(command, "add") == 0) {
            if (argument == NULL) {
                send_response(client_socket, "Invalid Command\n");
                continue;
            }
            char *day = strtok(argument, ",");
            char *genre = strtok(NULL, ",");
            char *title = strtok(NULL, ",");
            char *status = strtok(NULL, ",");

            fprintf(anime_file, "\n%s,%s,%s,%s", day, genre, title, status);
            fflush(anime_file);
            char log_entry[size_buffer];
            snprintf(log_entry, size_buffer, "ADD: %s, %s, %s, %s", day, genre, title, status);
            log_change(log_file, "ADD", log_entry);

            send_response(client_socket, "Anime successfully added.\n");
        } else if (strcmp(command, "edit") == 0) {
            if (argument == NULL) {
                send_response(client_socket, "Invalid Command\n");
                continue;
            }
            char *anime_to_edit = strtok(argument, ",");
            char *new_day = strtok(NULL, ",");
            char *new_genre = strtok(NULL, ",");
            char *new_title = strtok(NULL, ",");
            char *new_status = strtok(NULL, ",");

            FILE *temp_file = fopen("/home/winds/soal_4/temp.csv", "w");
            if (temp_file == NULL) {
                perror("Failed to create temporary file");
                send_response(client_socket, "Failed to edit anime.\n");
                continue;
            }
            rewind(anime_file);
            char line[size_buffer];
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *day = strtok(line, ",");
                char *genre = strtok(NULL, ",");
                char *title = strtok(NULL, ",");
                char *status = strtok(NULL, ",");

                while (strtok(NULL, ",") != NULL) {
                    strcat(title, strtok(NULL, ","));
                }
                char *clean_title = strtok(title, "\n");
                if (strcasecmp(clean_title, anime_to_edit) == 0) {
                    fprintf(temp_file, "%s,%s,%s,%s\n", new_day, new_genre, new_title, new_status);
                } else {
                    fprintf(temp_file, "%s,%s,%s,%s", day, genre, title, status);
                }
            }

            fclose(anime_file);
            fclose(temp_file);
            if (rename("/home/winds/soal_4/temp.csv", "/home/winds/soal_4/myanimelist.csv") != 0) {
                perror("Failed to rename file");
                send_response(client_socket, "Failed to edit anime.\n");
                continue;
            }

            char log_entry[size_buffer];
            snprintf(log_entry, size_buffer, "EDIT: %s, %s, %s, %s, %s", anime_to_edit, new_day, new_genre, new_title, new_status);
            log_change(log_file, "EDIT", log_entry);
            send_response(client_socket, "Anime successfully edited.\n");
            anime_file = fopen("/home/winds/soal_4/myanimelist.csv", "r+");
            if (anime_file == NULL) {
                perror("Failed to reopen anime list file");
                return;
            }
        } else if (strcmp(command, "delete") == 0) {
            if (argument == NULL) {
                send_response(client_socket, "Invalid Command\n");
                continue;
            }
            FILE *temp_file = fopen("/home/winds/soal_4/temp.csv", "w");
            if (temp_file == NULL) {
                perror("Gagal membuat file sementara");
                send_response(client_socket, "Gagal menghapus anime.\n");
                continue;
            }
            rewind(anime_file);
            char line[size_buffer];
            int deleted = 0; 
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *day = strtok(line, ",");
                char *genre = strtok(NULL, ",");
                char *title = strtok(NULL, ",");
                char *status = strtok(NULL, ",");
               
                while (strtok(NULL, ",") != NULL) {
                    strcat(title, strtok(NULL, ","));
                }
    
                char *clean_title = strtok(title, "\n");
                if (strcasecmp(clean_title, argument) == 0) {
                    deleted = 1; 
                    continue; 
                }

                fprintf(temp_file, "%s,%s,%s,%s", day, genre, title, status);
            }

            fclose(temp_file);
            fclose(anime_file);
            if (remove("/home/winds/soal_4/myanimelist.csv") != 0) {
                perror("Gagal menghapus file");
                send_response(client_socket, "Gagal menghapus anime.\n");
                continue;
            }
            if (rename("/home/winds/soal_4/temp.csv", "/home/winds/soal_4/myanimelist.csv") != 0) {
                perror("Gagal mengganti nama file");
                send_response(client_socket, "Gagal menghapus anime.\n");
                continue;
            }
            if (deleted) {
                char log_entry[size_buffer];
                snprintf(log_entry, size_buffer, "DELETE: %s", argument);
                log_change(log_file, "DELETE", log_entry);
                send_response(client_socket, "Anime berhasil dihapus.\n");
            } else {
                send_response(client_socket, "Anime tidak ditemukan.\n");
            }
            anime_file = fopen("/home/winds/soal_4/myanimelist.csv", "r+");
            if (anime_file == NULL) {
                perror("Gagal membuka kembali file daftar anime");
                return;
            }
        } else {
            send_response(client_socket, "Invalid Command\n");
        }

        memset(buffer, 0, size_buffer);
    }

    if (read_bytes == 0) {
        printf("Client closed the connection.\n");
    } else {
        perror("read failed");
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    int opt = 1;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len;

    system("wget 'https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50' -O /home/winds/soal_4/myanimelist.csv");

    FILE *anime_file = fopen("/home/winds/soal_4/myanimelist.csv", "r+");
    if (anime_file == NULL) {
        perror("Failed to open anime list file");
        return 1;
    }

    FILE *log_file = fopen("/home/winds/soal_4/change.log", "a+");
    if (log_file == NULL) {
        // akan membuat file change.log kalau tidak ada
        FILE *newlog_file = fopen("/home/winds/soal_4/change.log", "w");
        if (newlog_file == NULL) {
            perror("Failed to create log file");
            fclose(anime_file);
            return 1;
        }
        fclose(newlog_file);
        log_file = fopen("/home/winds/soal_4/change.log", "a+");
        if (log_file == NULL) {
            perror("Failed to open log file");
            fclose(anime_file);
            return 1;
        }
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        fclose(anime_file);
        fclose(log_file);
        return 1;
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        close(server_socket);
        fclose(anime_file);
        fclose(log_file);
        return 1;
    }

    if (listen(server_socket, maks_clients) < 0) {
        perror("Failed to listen on socket");
        close(server_socket);
        fclose(anime_file);
        fclose(log_file);
        return 1;
    }

    printf("Server started. Listening on port 8080...\n");

    while (1) {
        client_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0) {
            perror("Failed to accept client");
            continue;
        }

        handle_client(client_socket, anime_file, log_file);
        break;
    }

    fclose(anime_file);
    fclose(log_file);
    close(server_socket);

    return 0;
}
