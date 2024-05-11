# Sisop-3-2024-MH-IT27
Anggota Kelompok :
|  NRP|Nama Anggota  |
|--|--|
|5027231079|Harwinda|
|5027221058|Irfan Qobus Salim|
|5027231038|Dani Wahyu Anak Ary|

List Soal:

 - [Soal 1](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/tree/main#soal_1)
 - [Soal 2](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/tree/main#soal_2)
 - [Soal 3](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/tree/main#soal_3)
 - [Soal 4](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/tree/main#soal_4)

## Soal 1
<details><summary>Klik untuk melihat soal</summary>
Pada zaman dahulu pada galaksi yang jauh-jauh sekali, hiduplah seorang Stelle. Stelle adalah seseorang yang sangat tertarik dengan Tempat Sampah dan Parkiran Luar Angkasa. Stelle memulai untuk mencari Tempat Sampah dan Parkiran yang terbaik di angkasa. Dia memerlukan program untuk bisa secara otomatis mengetahui Tempat Sampah dan Parkiran dengan rating terbaik di angkasa. Programnya berbentuk microservice sebagai berikut:
- Dalam auth.c pastikan file yang masuk ke folder new-entry adalah file csv dan berakhiran trashcan dan parkinglot. Jika bukan, program akan secara langsung akan delete file tersebut. 
- Contoh dari nama file yang akan diautentikasi:
  - belobog_trashcan.csv
  - osaka_parkinglot.csv
- File csv yang lolos tahap autentikasi akan dikirim ke shared memory. 
- Dalam rate.c, proses akan mengambil data csv dari shared memory dan akan memberikan output Tempat Sampah dan Parkiran dengan Rating Terbaik dari data tersebut.
- Pada db.c, proses bisa memindahkan file dari new-data ke folder microservices/database, WAJIB MENGGUNAKAN SHARED MEMORY.
- Log semua file yang masuk ke folder microservices/database ke dalam file db.log dengan contoh format sebagai berikut:
**[DD/MM/YY hh:mm:ss] [type] [filename]**
ex : **[07/04/2024 08:34:50] [Trash Can] [belobog_trashcan.csv]**
</details>
 
### Penjelasan
Berikut merupakan kode dari **auth.c**, berfungsi untuk memfiltrasi file csv yang valid dari folder `new-data`.

```c
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
```
- Pertama, kita membuat struct untuk menyimpan nama file dan kata dari trashcan maupun parkinglot. Kemudian data struct ini akan kita simpan ke dalam shared memory. Penyimpanan shared memory dilakukan oleh fungsi `copy_to_shared_memory()`.

```c
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
```
- Kemudian, kita membuat fungsi `removing_non_trash_parking_file()`. Fungsi ini berfungsi untuk menghapus file yang tidak memiliki format csv dan/atau tidak memiliki nama file trashcan atau parkinglot.

```c
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
```

- Masuk ke dalam fungsi main, kita akan menghapus file yang tidak sesuai kriteria, kemudian kita akan memindahkan nama dari file yang valid ke dalam shared memory dengan cara membuka folder `new-data` lalu memanggil fungsi `copy_to_shared_memory()`. Lalu kita akan menampilkan ke terminal, hal apa saja yang tersimpan di shared memory.

```c
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
```

Berikut merupakan kode dari **rate.c**, berfungsi untuk menampilkan rating tertinggi dari tiap file csv yang telah terfiltrasi.

```c
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
```
- Pertama, kita menginisiasi struct untuk menyimpan data dari shared memory yang telah diunggah sebelumnya di program **auth.c** dan juga menyimpan nama dan rating tertinggi dari tiap file.

```c
typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;

typedef struct {
    char name[100];
    float rating;
} PlaceData;
```

- Kedua, kita membuat fungsi main untuk mengambil data dari shared memory, kemudian kita menginisiasi loop untuk mengecek tiap file di dalam folder `new-data`. Lalu di dalam for loop terdapat while loop yang akan mengecek baris tiap file dan menemukan skor tertingginya. Jika baris tersebut merupakan skor tertinggi, maka baris tersebut akan disimpan ke dalam struct. Jika looping telah selesai, program menampilkan output rating tertinggi ke terminal beserta nama dan tipe data file tersebut.

```c
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
```

Terakhir, kita membuat program bernama **db.c** untuk memindahkan file yang berada di folder `new-data` ke folder `database` dan menuliskannya ke file log.

```c
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
```

- Pertama, kita mendefinisikan struct untuk menyimpan data dari shared memory.
```c
typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;
```

- Kemudian dalam fungsi `write_db_log()`, program akan menuliskan setiap progress yang dilakukan ke dalam file `db.log`.

```c
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
```

- Lalu, dalam fungsi `move_valid_file()`, kita akan memindah sebuah file dari folder `new-data` ke `database` dengan `mv`.

```c
void move_valid_file(const char *filename) {
    char command[512];
    snprintf(command, sizeof(command), "mv /home/irfanqs/modul3/soal_1/new-data/%s /home/irfanqs/modul3/soal_1/microservices/database/", filename);
    if (system(command) != 0) {
        fprintf(stderr, "Error moving file: %s\n", filename);
    } else {
        printf("File moved successfully: %s\n", filename);
    }
}
```
- Selanjutnya dalam fungsi main, program mengambil data dari shared memory dan data tersebut digunakan untuk mengecek apakah file tersebut benar file yang valid atau tidak. Jika valid, program akan memanggil `move_valid_file()` dan `write_db_log()` untuk memindahkan file dan menuliskannya ke log.

```c
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
```

### Berikut merupakan cara kerja dari setiap program <br>
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/c9abe76e-4776-4aa7-8ba9-860ab545019d)
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/7db4c119-3f5c-432d-9fc6-0687a25c98bd)
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/26d670dd-aed4-4780-8a67-604a8fc9eb00)
<br> Berikut merupakan isi dari folder database dan juga isi dari file db.log <br>
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/1dfefce7-0b0a-4be5-a808-1f6a60d1dd06)

### Kendala
Tidak ada kendala pada soal ini

### Revisi
Tidak ada revisi pada soal ini

## Soal 2
<details><summary>Klik untuk melihat soal</summary>

</details>

### Penjelasan

### Kendala

### Revisi

## Soal 3
<details><summary>Klik untuk melihat soal</summary>

</details>

### Penjelasan
Pertama, kita akan membuat file **actions.c** yang berfungsi untuk menyimpan segala fungsi yang dibutuhkan.

```c
#include <stdio.h>
#include <string.h>
#include "actions.h"

void gap(float distance, char *output) {
    if (distance < 3.5) sprintf(output, "Gogogo");
    else if (distance >= 3.5 && distance < 10) sprintf(output, "Push");
    else if (distance > 10) sprintf(output, "Stay out of trouble");
    else sprintf(output, "Invalid distance!");
}

void fuel(int fuel_percent, char *output) {
    if (fuel_percent > 80) sprintf(output, "Push Push Push");
    else if (fuel_percent >= 50 && fuel_percent <= 80) sprintf(output, "You can go");
    else if (fuel_percent < 50) sprintf(output, "Conserve Fuel");
}

void tire(int tire_usage, char *output) {
    if (tire_usage > 80) sprintf(output, "Go Push Go Push");
    else if (tire_usage >= 50 && tire_usage <= 80) sprintf(output, "Good Tire Wear");
    else if (tire_usage >= 30 && tire_usage < 50) sprintf(output, "Conserve Your Tire");
    else sprintf(output, "Box Box Box");
}

void tire_change(char* current_tire, char *output) {
    if (strcmp(current_tire, "Soft") == 0) sprintf(output, "Mediums Ready");
    else if (strcmp(current_tire, "Medium") == 0) sprintf(output, "Box for Softs");
    else sprintf(output, "Unknown tire type");
}
```

- Pertama, kita membuat fungsi `gap` yang menerima input berupa nilai distance dengan tipe data float. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void gap(float distance, char *output) {
    if (distance < 3.5) sprintf(output, "Gogogo");
    else if (distance >= 3.5 && distance < 10) sprintf(output, "Push");
    else if (distance > 10) sprintf(output, "Stay out of trouble");
    else sprintf(output, "Invalid distance!");
}
```
- Selanjutnya, kita membuat fungsi `fuel` yang menerima input berupa nilai fuel_percent dengan tipe data int. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void fuel(int fuel_percent, char *output) {
    if (fuel_percent > 80) sprintf(output, "Push Push Push");
    else if (fuel_percent >= 50 && fuel_percent <= 80) sprintf(output, "You can go");
    else if (fuel_percent < 50) sprintf(output, "Conserve Fuel");
}
```
- Selanjutnya, kita  membuat fungsi `tire` yang menerima input berupa nilai tire_usage dengan tipe data int. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void tire(int tire_usage, char *output) {
    if (tire_usage > 80) sprintf(output, "Go Push Go Push");
    else if (tire_usage >= 50 && tire_usage <= 80) sprintf(output, "Good Tire Wear");
    else if (tire_usage >= 30 && tire_usage < 50) sprintf(output, "Conserve Your Tire");
    else sprintf(output, "Box Box Box");
}
```
- Terakhir, kita membuat fungsi  `tire_change` yang menerima input berupa nilai current_tire dengan tipe data string. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void tire_change(char* current_tire, char *output) {
    if (strcmp(current_tire, "Soft") == 0) sprintf(output, "Mediums Ready");
    else if (strcmp(current_tire, "Medium") == 0) sprintf(output, "Box for Softs");
    else sprintf(output, "Unknown tire type");
}
```
- Agar kita dapat menjalankan fungsi ini di program lain, butuh yang namanya header file. Untuk itu kita perlu membuat file yang bernama **actions.h**.
```c
#ifndef ACTIONS_H
#define ACTIONS_H

void gap(float distance, char *output);
void fuel(int fuel_percent, char *output);
void tire(int tire_usage, char *output);
void tire_change(char* current_tire, char *output);

#endif

```
Selanjutnya, kita membuat file **padddock.c** yang berfungsi sebagai server.
```c
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

#define PORT 8080

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
```

Selanjutnya, kita akan membuat file **driver.c** yang berfungsi sebagai client.
```c
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
```

### Berikut merupakan demo dari file actions.c, driver.c, dan paddock.c

Client dan server terhubung dengan RPC non local dengan memanfaatkan ip dari vm linux
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/cbb0758e-8943-4922-a4e4-d32060775106)

<br> Berikut merupakan isi dari file race.log (start dari nomor 31) <br>
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/6e1b6773-c1ac-41b2-bf32-d7fc432ed9f2)

### Kendala
Tidak ada kendala pada soal ini

### Revisi
Tidak ada revisi pada soal ini

## Soal 4
<details><summary>Klik untuk melihat soal</summary>

</details>

### Penjelasan

### Kendala
Tidak ada kendala pada nomor ini

### Revisi
Tidak ada Revisi pada nomor ini
