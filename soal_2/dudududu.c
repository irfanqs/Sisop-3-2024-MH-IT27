#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <time.h>

int stringToNumber(char *str) {
    if (strcmp(str, "satu") == 0) return 1;
    else if (strcmp(str, "dua") == 0) return 2;
    else if (strcmp(str, "tiga") == 0) return 3;
    else if (strcmp(str, "empat") == 0) return 4;
    else if (strcmp(str, "lima") == 0) return 5;
    else if (strcmp(str, "enam") == 0) return 6;
    else if (strcmp(str, "tujuh") == 0) return 7;
    else if (strcmp(str, "delapan") == 0) return 8;
    else if (strcmp(str, "sembilan") == 0) return 9;
    else return -1; 
}

void angkaKeKata(int num, char *words) {
    char *units[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    char *tens[] = {"", "sepuluh", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    if (num < 10) {
        strcpy(words, units[num]);
    } else if (num < 20) {
        strcpy(words, teens[num - 10]);
    } else if (num < 100) {
        int digit1 = num / 10;
        int digit2 = num % 10;
        if (digit2 == 0) {
            strcpy(words, tens[digit1]);
        } else {
            sprintf(words, "%s %s", tens[digit1], units[digit2]);
        }
    }
}

void outputLog(int logNumber, const char *type, const char *message, const char *timestamp, const char *result) {
    FILE *logFile;
    logFile = fopen("histori.log", "a");
    if (logFile == NULL) {
        printf("Error: Tidak dapat membuka file log.\n");
        return;
    }

    fprintf(logFile, "[%s] [%s] %s %s\n", timestamp, type, message, result);
    fclose(logFile);
}

int main(int argc, char *argv[]) {
    char *opsi = argv[1];
    char *input1 = argv[2];
    char *input2 = argv[3];
    int pipefd[2];
    int logNumber = 1;
    int num1, num2;
    int hasil;
    char words[100];

    if (pipe(pipefd) == -1) {
        perror("Pipe gagal");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork gagal");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipefd[0]);

        num1 = stringToNumber(input1);
        num2 = stringToNumber(input2);

        if (num1 == -1 || num2 == -1) {
            printf("Input tidak valid\n");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        if (strcmp(opsi, "-kali") == 0) {
            hasil = num1 * num2;
            if (hasil < 0) {
                strcpy(words, "ERROR");
            } else {
                hasil = floor(hasil);
                angkaKeKata(hasil, words);
            }
            char timestamp[20];
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);

            outputLog(logNumber, "KALI", strcat(strcat(strcat(input1, " kali "), input2), " sama dengan "), timestamp, words);
        } else if (strcmp(opsi, "-tambah") == 0) {
            hasil = num1 + num2;
            if (hasil < 0) {
                strcpy(words, "ERROR");
            } else {
                hasil = floor(hasil);
                angkaKeKata(hasil, words);
            }
            char timestamp[20];
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);

            outputLog(logNumber, "TAMBAH", strcat(strcat(strcat(input1, " tambah "), input2), " sama dengan "), timestamp, words);
        } else if (strcmp(opsi, "-kurang") == 0) {
            hasil = num1 - num2;
            if (hasil < 0) {
                strcpy(words, "ERROR");
            } else {
                hasil = floor(hasil);
                angkaKeKata(hasil, words);
            }
            
            FILE *logFile;
            logFile = fopen("histori.log", "a");
            if (logFile == NULL) {
                printf("Error: Tidak dapat membuka file log.\n");
                exit(EXIT_FAILURE);
            }
            char timestamp[20];
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);

            fprintf(logFile, "[%s] [KURANG] %s pada pengurangan\n", timestamp, words);
            fclose(logFile);

        } else if (strcmp(opsi, "-bagi") == 0) {
            if (num2 == 0) {
                printf("Pembagian dengan nol tidak diperbolehkan.\n");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
            hasil = num1 / num2;
            if (hasil < 0) {
                strcpy(words, "ERROR");
            } else {
                hasil = floor(hasil);
                angkaKeKata(hasil, words);
            }
            char timestamp[20];
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);

            outputLog(logNumber, "BAGI", strcat(strcat(strcat(input1, " bagi "), input2), " sama dengan"), timestamp, words);
        } else {
            printf("Operasi tidak valid\n");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        write(pipefd[1], words, strlen(words) + 1);
        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[1]);

        char hasilKalimat[100];
        read(pipefd[0], hasilKalimat, sizeof(hasilKalimat));

        if (strcmp(opsi, "-kali") == 0) {
            printf("Hasil perkalian %s dan %s adalah %s.\n", input1, input2, hasilKalimat);
        } else if (strcmp(opsi, "-tambah") == 0) {
            printf("Hasil penjumlahan %s dan %s adalah %s.\n", input1, input2, hasilKalimat);
        } else if (strcmp(opsi, "-kurang") == 0) {
            printf("Hasil pengurangan %s dan %s adalah %s.\n", input1, input2, hasilKalimat);
        } else if (strcmp(opsi, "-bagi") == 0) {
            printf("Hasil pembagian %s dan %s adalah %s.\n", input1, input2, hasilKalimat);
        } else {
            printf("Operasi tidak valid\n");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
