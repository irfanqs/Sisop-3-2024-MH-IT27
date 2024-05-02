#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_filenames_LENGTH 512
#define MAX_FILES 10

typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;

typedef struct {
    char name[100];
    float rating;
} PlaceData;

int main() {
    key_t key = 1234;
    if (key == -1) {
        fprintf(stderr, "Failed to generate key\n");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        fprintf(stderr, "Failed to create shared memory segment\n");
        return 1;
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        fprintf(stderr, "Failed to attach shared memory segment\n");
        return 1;
    }

    // start code
    for (int i = 0; i < shared_data->count; i += 2) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "../new-data/%s", shared_data->filenames[i+1]);

        FILE *file = fopen(fullpath, "r");
        if (!file) {
            fprintf(stderr, "Could not open file %s\n", fullpath);
        }

        char line[100];
        if (fgets(line, sizeof(line), file) == NULL) {
            fprintf(stderr, "Empty file or error reading header\n");
            fclose(file);
            return 1;
        }

        float max_rating = -1;
        PlaceData max_rating_data;

        while (fgets(line, sizeof(line), file) != NULL) {
            char *name = strtok(line, ",");
            char *rating_str = strtok(NULL, ",");

            if (rating_str == NULL) {
                fprintf(stderr, "Invalid file format\n");
                fclose(file);
                return 1;
            }

            float rating = atof(rating_str);

            if (rating > max_rating) {
                max_rating = rating;
                strcpy(max_rating_data.name, name);
                max_rating_data.rating = rating;
            }
        }

        fclose(file);

        printf("Type: %s\n", shared_data->filenames[i]);
        printf("filenames: %s\n", shared_data->filenames[i+1]);
        printf("----------------------\n");
        printf("Name: %s\n", max_rating_data.name);
        printf("Rating: %.1f\n\n", max_rating_data.rating);
        
    }

    if (shmdt(shared_data) == -1) {
        fprintf(stderr, "Failed to detach shared memory segment\n");
        return 1;
    }

    return 0;
}
