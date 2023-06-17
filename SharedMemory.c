#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

// Struktur für die Nachricht in der Shared Memory
struct shared_data {
    int voltage[ARRAY_SIZE];
    int count;
};

int shouldExit = 0;
int shmemID;  // ID des Shared Memory
int semID;    // ID des Semaphors

// Signal Handler für Strg+C
void sigintHandler(int signum) {
    printf("\nProgramm wurde abgebrochen.\n");
    fflush(stdout);
    shouldExit = 1;
    shmctl(shmemID, IPC_RMID, NULL);  // Shared Memory aufräumen
    semctl(semID, 0, IPC_RMID);       // Semaphore aufräumen
}

// Semaphore P-Operation (wait)
void semWait(int semID) {
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = -1;
    semaphore.sem_flg = SEM_UNDO;
    semop(semID, &semaphore, 1);
}

// Semaphore V-Operation (signal)
void semSignal(int semID) {
    struct sembuf semaphore;
    semaphore.sem_num = 0;
    semaphore.sem_op = 1;
    semaphore.sem_flg = SEM_UNDO;
    semop(semID, &semaphore, 1);
}

int main() {
    // Signal Handler für Strg+C registrieren
    signal(SIGINT, sigintHandler);

    // Erzeugen des Shared Memory
    key_t key = ftok(".", 's');
    shmemID = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmemID == -1) {
        perror("Fehler beim Erzeugen des Shared Memory");
        return 1;
    }

    // Anhängen des Shared Memory
    struct shared_data* sharedData = shmat(shmemID, NULL, 0);
    if (sharedData == (void*) -1) {
        perror("Fehler beim Anhängen des Shared Memory");
        return 1;
    }

    // Initialisieren des Shared Memory
    sharedData->count = 0;

    // Erzeugen des Semaphors
    semID = semget(key, 1, IPC_CREAT | 0666);
    if (semID == -1) {
        perror("Fehler beim Erzeugen des Semaphors");
        return 1;
    }

    // Initialisieren des Semaphors
    semctl(semID, 0, SETVAL, 1);

    while (!shouldExit) {
        int i, sum = 0, min = MAX_VOLTAGE, max = 0;

        // Random-Nummer initialisieren und generieren
        srand(time(NULL));

        // Messwerte generieren und im Shared Memory speichern
        semWait(semID);
        sharedData->count = ARRAY_SIZE;
        for (i = 0; i < ARRAY_SIZE; i++) {
            sharedData->voltage[i] = rand() % MAX_VOLTAGE;
        }
        semSignal(semID);

        // Ausgabe der Messwerte und Überprüfung auf Grenzwert
        printf("Generierte Messwerte:\n");
        semWait(semID);
        for (i = 0; i < sharedData->count; i++) {
            printf("%d ", sharedData->voltage[i]);
            if (sharedData->voltage[i] >= 1000) {
                printf("(Überschreitet Grenze)\n");
            } else {
                printf("\n");
            }
        }
        semSignal(semID);

        // Statistische Auswertung der Messwerte
        semWait(semID);
        for (i = 0; i < sharedData->count; i++) {
            sum += sharedData->voltage[i];
            if (sharedData->voltage[i] < min) {
                min = sharedData->voltage[i];
            }
            if (sharedData->voltage[i] > max) {
                max = sharedData->voltage[i];
            }
        }

        // Durchschnitt berechnen
        double average = (double) sum / sharedData->count;

        // Ausgabe der statistischen Werte
        printf("\nStatistische Auswertung der Messwerte:\n");
        printf("Summe: %d\n", sum);
        printf("Durchschnitt: %.2f\n", average);
        printf("Minimum: %d\n", min);
        printf("Maximum: %d\n", max);
        semSignal(semID);

        sleep(5);  // 5 Sekunden warten
    }

    // Shared Memory aufräumen
    shmdt(sharedData);
    shmctl(shmemID, IPC_RMID, NULL);

    // Semaphore aufräumen
    semctl(semID, 0, IPC_RMID);

    return 0;
}
