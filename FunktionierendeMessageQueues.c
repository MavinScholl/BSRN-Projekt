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

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

int shouldExit = 0;
int msgq_read;  // Message Queue für das Lesen
int msgq_write1;  // Message Queue für das Schreiben in die erste Queue
int msgq_write2;  // Message Queue für das Schreiben in die zweite Queue
int msgq_write3;  // Message Queue für das Schreiben in die dritte Queue

struct statistics {
    long mtype;
    int sum;
    double average;
    int min;
    int max;
};
struct message {
    long mtype;
    int voltage;
};

void sigintHandler(int signum) {
    printf("\nProgramm wurde abgebrochen.\n");
    fflush(stdout);
    shouldExit = 1;
    msgctl(msgq_read, IPC_RMID, NULL);  // Message Queue entfernen
    msgctl(msgq_write1, IPC_RMID, NULL);
    msgctl(msgq_write2, IPC_RMID, NULL);
    msgctl(msgq_write3, IPC_RMID, NULL);
}

int main() {
    signal(SIGINT, sigintHandler);

    // Message Queues erstellen
    key_t key_read = ftok(".", 'r');
    key_t key_write1 = ftok(".", '1');
    key_t key_write2 = ftok(".", '2');
    key_t key_write3 = ftok(".", '3');

    if ((msgq_read = msgget(key_read, IPC_CREAT | 0666)) == -1) {
        perror("Fehler beim Erstellen der Lesenachrichtenwarteschlange");
        return 1;
    }

    if ((msgq_write1 = msgget(key_write1, IPC_CREAT | 0666)) == -1) {
        perror("Fehler beim Erstellen der ersten Schreibnachrichtenwarteschlange");
        return 1;
    }

    if ((msgq_write2 = msgget(key_write2, IPC_CREAT | 0666)) == -1) {
        perror("Fehler beim Erstellen der zweiten Schreibnachrichtenwarteschlange");
        return 1;
    }

    if ((msgq_write3 = msgget(key_write3, IPC_CREAT | 0666)) == -1) {
        perror("Fehler beim Erstellen der dritten Schreibnachrichtenwarteschlange");
        return 1;
    }

    while (!shouldExit) {
        int voltage[ARRAY_SIZE];
        int character, i, sum = 0, min = MAX_VOLTAGE, max = 0;
        int validCount = 0; // Zähler für gültige Messwerte

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

        // Schreibe gültige Messwerte in die Message Queues
        printf("Sende gültige Messwerte über die Message Queues...\n");
        for (i = 0; i < ARRAY_SIZE; i++) {
            if (voltage[i] < 1000) {
                struct message msg;
                msg.mtype = 1;
                msg.voltage = voltage[i];

                if (msgsnd(msgq_write1, &msg, sizeof(struct message) - sizeof(long), 0) == -1) {
                    perror("Fehler beim Senden der Nachricht an die erste Warteschlange");
                }
                if (msgsnd(msgq_write2, &msg, sizeof(struct message) - sizeof(long), 0) == -1) {
                    perror("Fehler beim Senden der Nachricht an die zweite Warteschlange");
                }
            }
        }

        // Datei erstellen und gültige Messwerte aus der ersten Message Queue lesen und schreiben
        FILE *file = fopen("Messwerte.txt", "w");
        if (file == NULL) {
            perror("Fehler beim Erstellen der Datei");
            return 2;
        }

        printf("\nSchreibe gültige Messwerte in die Datei...\n");
        struct message receivedMsg;
        for (i = 0; i < validCount; i++) {
            if (msgrcv(msgq_write1, &receivedMsg, sizeof(struct message) - sizeof(long), 0, 0) == -1) {
                perror("Fehler beim Empfangen der Nachricht aus der ersten Warteschlange");
                break;
            }
            fprintf(file, "%d\n", receivedMsg.voltage);
        }
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

        // Messwerte aus der zweiten Message Queue lesen und Statistiken berechnen
        printf("\nStatistische Auswertung der gültigen Messwerte:\n");
        if (validCount > 0) {
            struct message receivedMsgs[validCount];
            printf("Empfange Messwerte aus der zweiten Warteschlange...\n");
            for (i = 0; i < validCount; i++) {
                if (msgrcv(msgq_write2, &receivedMsgs[i], sizeof(struct message) - sizeof(long), 0, 0) == -1) {
                    perror("Fehler beim Empfangen der Nachricht aus der zweiten Warteschlange");
                }
            }

            for (i = 0; i < validCount; i++) {
                sum += receivedMsgs[i].voltage;
                if (receivedMsgs[i].voltage < min) {
                    min = receivedMsgs[i].voltage;
                }
                if (receivedMsgs[i].voltage > max) {
                    max = receivedMsgs[i].voltage;
                }
            }
            double average = (double) sum / validCount;
            

            // Schreibe Statistiken in die dritte Message Queue
            struct statistics stats;
            stats.mtype = 1;
            stats.sum = sum;
            stats.average = average;
            stats.min = min;
            stats.max = max;

            if (msgsnd(msgq_write3, &stats, sizeof(struct statistics) - sizeof(long), 0) == -1) {
                perror("Fehler beim Senden der Statistiken an die dritte Warteschlange");
            }
        } else {
            printf("Keine gültigen Messwerte vorhanden.\n");
        }

        // Lese Statistiken aus der dritten Message Queue und gebe sie auf der Konsole aus
        printf("\nEmpfange Statistiken aus der dritten Warteschlange...\n");
        struct statistics receivedStats;

        if (msgrcv(msgq_write3, &receivedStats, sizeof(struct statistics) - sizeof(long), 0, 0) == -1) {
            perror("Fehler beim Empfangen der Statistiken aus der dritten Warteschlange");
        }

        printf("Empfangene Statistiken:\n");
        printf("Summe: %d\n", receivedStats.sum);
        printf("Durchschnitt: %.2f\n", receivedStats.average);
        printf("Minimum: %d\n", receivedStats.min);
        printf("Maximum: %d\n", receivedStats.max);

        sleep(30);
    }

    // Message Queues schließen
    msgctl(msgq_write1, IPC_RMID, NULL);
    msgctl(msgq_write2, IPC_RMID, NULL);
    msgctl(msgq_write3, IPC_RMID, NULL);

    return 0;
}
