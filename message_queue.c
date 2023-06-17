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
#include <sys/msg.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

// Struktur für die Nachricht in der Message Queue
struct message {
    long mtype;
    int value;
};

int shouldExit = 0;
int msgQueueID;  // ID der Message Queue

// Signal Handler für Strg+C
void sigintHandler(int signum) {
    printf("\nProgramm wurde abgebrochen.\n");
    fflush(stdout);
    shouldExit = 1;
    msgctl(msgQueueID, IPC_RMID, NULL);  // Message Queue aufräumen
}

int main() {
    // Signal Handler für Strg+C registrieren
    signal(SIGINT, sigintHandler);

    // Erzeugen der Message Queue
    key_t key = ftok(".", 'm');
    msgQueueID = msgget(key, IPC_CREAT | 0666);
    if (msgQueueID == -1) {
        perror("Fehler beim Erzeugen der Message Queue");
        return 1;
    }

    while (!shouldExit) {
        int voltage[ARRAY_SIZE];
        int i, sum = 0, min = MAX_VOLTAGE, max = 0;

        // Random-Nummer initialisieren und generieren
        srand(time(NULL));

        // Messwerte generieren und in Array speichern
        for (i = 0; i < ARRAY_SIZE; i++) {
            voltage[i] = rand() % MAX_VOLTAGE;
        }

        // Ausgabe der Messwerte und Überprüfung auf Grenzwert
        printf("Generierte Messwerte:\n");
        for (i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", voltage[i]);
            if (voltage[i] >= 1000) {
                printf("(Überschreitet Grenze)\n");
            } else {
                printf("\n");
            }
        }

        // Messwerte in die Message Queue senden
        printf("Sende Messwerte in die Message Queue...\n");
        struct message msg;
        for (i = 0; i < ARRAY_SIZE; i++) {
            if (voltage[i] < 1000) {
                msg.mtype = 1;  // Nachrichtentyp festlegen
                msg.value = voltage[i];
                if (msgsnd(msgQueueID, &msg, sizeof(msg.value), IPC_NOWAIT) == -1) {
                    perror("Fehler beim Senden der Nachricht");
                }
            }
        }

        // Statistische Auswertung der Messwerte
        for (i = 0; i < ARRAY_SIZE; i++) {
            sum += voltage[i];
            if (voltage[i] < min) {
                min = voltage[i];
            }
            if (voltage[i] > max) {
                max = voltage[i];
            }
        }

        // Durchschnitt berechnen
        double average = (double) sum / ARRAY_SIZE;

        // Ausgabe der statistischen Werte
        printf("\nStatistische Auswertung der Messwerte:\n");
        printf("Summe: %d\n", sum);
        printf("Durchschnitt: %.2f\n", average);
        printf("Minimum: %d\n", min);
        printf("Maximum: %d\n", max);

        sleep(5);  // 5 Sekunden warten
    }

    // Message Queue aufräumen
    msgctl(msgQueueID, IPC_RMID, NULL);

    return 0;
}
