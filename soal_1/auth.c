#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_FILENAME_LENGTH 512
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 1234

typedef struct {
    char filenames[MAX_FILES][MAX_FILENAME_LENGTH];
    int count;
} SharedData;

void copy_to_shared_memory(const char *filename) {
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

    if (strstr(filename, "trashcan")) {
        strcpy(shared_data->filenames[shared_data->count], "trashcan");
        shared_data->count++;
    } else if  (strstr(filename, "parkinglot")) {
        strcpy(shared_data->filenames[shared_data->count], "parkinglot");
        shared_data->count++;
    }

    strcpy(shared_data->filenames[shared_data->count], filename);
    shared_data->count++;

    if (shmdt(shared_data) == -1) {
        perror("Error detaching shared memory");
        exit(EXIT_FAILURE);
    }
}

void removing_non_trash_parking_file() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("new-data")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == 8) {
                char *filename = ent->d_name;
                char *extension = strrchr(filename, '.');
                char *name = strtok(filename, "_");

                if (extension != NULL && strcmp(extension, ".csv") == 0 &&
                    (strstr(name, "trashcan") || strstr(name, "parkinglot"))) {
                    copy_to_shared_memory(name);
                } else {
                    char filepath[MAX_FILENAME_LENGTH];
                    snprintf(filepath, sizeof(filepath), "new-data/%s", ent->d_name);
                    remove(filepath);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Folder tidak dapat dibuka");
        exit(EXIT_FAILURE);
    }
}

int main() {
    removing_non_trash_parking_file();

    DIR *dp = opendir("new-data");
    struct dirent *ep;
    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_type == 8) {
            copy_to_shared_memory(ep->d_name);
        }
    }
    closedir(dp);

    // Menampilkan isi array filenames ke terminal
    printf("Files in shared memory:\n");
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Failed to get shared memory segment");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Failed to attach shared memory segment");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < shared_data->count; ++i) {
        printf("%s\n", shared_data->filenames[i]);
    }
    
    if (shmdt(shared_data) == -1) {
        perror("Failed to detach shared memory segment");
        exit(EXIT_FAILURE);
    }

    return 0;
}
