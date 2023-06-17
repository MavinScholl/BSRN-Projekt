#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

int shouldExit = 0;
int semid;       // Semaphore-ID
int shmid;       // Shared Memory-ID

struct statistics {
    int sum;
    double average;
    int min;
    int max;
};

struct shared_memory {
    struct statistics stats;
    int voltage[ARRAY_SIZE];
};

void semaphoreWait(int semid, int sem_num) {
    struct sembuf operation;
    operation.sem_num = sem_num;
    operation.sem_op = -1;
    operation.sem_flg = 0;
    semop(semid, &operation, 1);
}

void semaphoreSignal(int semid, int sem_num) {
    struct sembuf operation;
    operation.sem_num = sem_num;
    operation.sem_op = 1;
    operation.sem_flg = 0;
    semop(semid, &operation, 1);
}

void sigintHandler(int signum) {
    printf("\nProgramm wurde abgebrochen.\n");
    fflush(stdout);
    shouldExit = 1;
}

int main() {
    signal(SIGINT, sigintHandler);

    // Semaphore erstellen
    key_t semkey = ftok(".", 's');
    semid = semget(semkey, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("Fehler beim Erstellen des Semaphors");
        return 1;
    }

    // Semaphore initialisieren
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("Fehler beim Initialisieren des Semaphors");
        return 1;
    }

    // Shared Memory erstellen
    key_t shmkey = ftok(".", 'm');
    shmid = shmget(shmkey, sizeof(struct shared_memory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Fehler beim Erstellen des Shared Memory");
        return 1;
    }

    // Shared Memory an das Programm anhängen
    struct shared_memory *sharedStats = (struct shared_memory *)shmat(shmid, NULL, 0);
    if (sharedStats == (void *)-1) {
        perror("Fehler beim Anhängen des Shared Memory");
        return 1;
    }

    while (!shouldExit) {
        signal(SIGINT, sigintHandler);

    // Semaphore erstellen
    key_t semkey = ftok(".", 's');
    semid = semget(semkey, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("Fehler beim Erstellen des Semaphors");
        return 1;
    }

    // Semaphore initialisieren
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("Fehler beim Initialisieren des Semaphors");
        return 1;
    }

    // Shared Memory erstellen
    key_t shmkey = ftok(".", 'm');
    shmid = shmget(shmkey, sizeof(struct shared_memory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Fehler beim Erstellen des Shared Memory");
        return 1;
    }

    // Shared Memory an das Programm anhängen
    struct shared_memory *sharedStats = (struct shared_memory *)shmat(shmid, NULL, 0);
    if (sharedStats == (void *)-1) {
        perror("Fehler beim Anhängen des Shared Memory");
        return 1;
    }

    while (!shouldExit) {
        int voltage[ARRAY_SIZE];
        int character, i, validCount = 0;

        srand(time(NULL));

        for (i = 0; i < ARRAY_SIZE; i++) {
            voltage[i] = rand() % MAX_VOLTAGE;
        }

        printf("Generierte Messwerte:\n");
        for (i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", voltage[i]);
            if (voltage[i] >= 1000) {
                printf("(Überschreitet Grenze)\n");
            } else {
                printf("\n");
                validCount++;
            }
        }

        // Schreibe gültige Messwerte in das Shared Memory
        semaphoreWait(semid, 0);

        sharedStats->stats.sum = 0;
        sharedStats->stats.min = MAX_VOLTAGE;
        sharedStats->stats.max = 0;

        for (i = 0; i < ARRAY_SIZE; i++) {
            if (voltage[i] < 1000) {
                sharedStats->voltage[i] = voltage[i];
                sharedStats->stats.sum += voltage[i];
                if (voltage[i] < sharedStats->stats.min) {
                    sharedStats->stats.min = voltage[i];
                }
                if (voltage[i] > sharedStats->stats.max) {
                    sharedStats->stats.max = voltage[i];
                }
            }
        }

        sharedStats->stats.average = (double)sharedStats->stats.sum / validCount;

        semaphoreSignal(semid, 0);

        // Datei erstellen und gültige Messwerte aus dem Shared Memory lesen und schreiben
        FILE *file = fopen("Messwerte.txt", "w");
        if (file == NULL) {
            perror("Fehler beim Erstellen der Datei");
            return 2;
        }

        printf("\nSchreibe gültige Messwerte in die Datei...\n");

        semaphoreWait(semid, 0);

        for (i = 0; i < validCount; i++) {
            if (sharedStats->voltage[i] < 1000) {
                fprintf(file, "%d\n", sharedStats->voltage[i]);
            }
        }

        semaphoreSignal(semid, 0);

        printf("%d Messwerte wurden erfolgreich in die Datei geschrieben.\n", i);

        // Datei schließen
        fclose(file);

        printf("\nInhalt der Datei 'Messwerte.txt':\n");
        file = fopen("Messwerte.txt", "r");
        if (file == NULL) {
            printf("Die Datei konnte nicht geöffnet werden.\n");
            return 1;
        }
        while ((character = fgetc(file)) != EOF) {
            putchar(character);
        }
        fclose(file);

        // Statistiken aus dem Shared Memory lesen und auf der Konsole ausgeben
        semaphoreWait(semid, 0);

        printf("\nStatistische Auswertung der gültigen Messwerte:\n");
        printf("Summe: %d\n", sharedStats->stats.sum);
        printf("Durchschnitt: %.2f\n", sharedStats->stats.average);
        printf("Minimum: %d\n", sharedStats->stats.min);
        printf("Maximum: %d\n", sharedStats->stats.max);

        semaphoreSignal(semid, 0);

        sleep(30);
    }

    }

    // Shared Memory und Semaphore freigeben
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Fehler beim Löschen des Semaphors");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Fehler beim Löschen des Shared Memory");
    }

    return 0;
}
