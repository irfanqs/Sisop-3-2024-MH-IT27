#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <time.h>

#define SHARED_MEMORY_KEY 1234
#define MAX_filenames_LENGTH 512
#define MAX_FILES 10

typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;

void write_db_log(const char *type, const char *filename) {
    time_t now;
    struct tm *tm_info;
    char timestamp[20];
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", tm_info);

    FILE *logFile = fopen("/home/irfanqs/modul3/soal_1/microservices/database/db.log", "a");
    if (logFile != NULL) {
        fprintf(logFile, "[%s] [%s] [%s]\n", timestamp, type, filename);
        fclose(logFile);
    }
}

void move_valid_file(const char *filename) {
    char command[512];
    snprintf(command, sizeof(command), "mv /home/irfanqs/modul3/soal_1/new-data/%s /home/irfanqs/modul3/soal_1/microservices/database/", filename);
    if (system(command) != 0) {
        fprintf(stderr, "Error moving file: %s\n", filename);
    } else {
        printf("File moved successfully: %s\n", filename);
    }
}

int main() {
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    for(int i = 1; i <= shared_data->count; i+=2) {
        if (strstr(shared_data->filenames[i], "trashcan")) {
            move_valid_file(shared_data->filenames[i]);
            write_db_log("Trash Can", shared_data->filenames[i]);
        } else if  (strstr(shared_data->filenames[i], "parkinglot")) {
            move_valid_file(shared_data->filenames[i]);
            write_db_log("Parking Lot", shared_data->filenames[i]);
        }
    }

    if (shmdt(shared_data) == -1) {
        perror("Error detaching shared memory");
        exit(EXIT_FAILURE);
    }
}