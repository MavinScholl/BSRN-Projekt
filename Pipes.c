#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

int shouldExit = 0;
int pipefd[2];  // Pipe-Dateideskriptoren

void sigintHandler(int signum) {
    printf("\nProgramm wurde abgebrochen.\n");
    fflush(stdout);
    shouldExit = 1;
    close(pipefd[0]);  // Pipe schließen
    close(pipefd[1]);
}

int main() {
    signal(SIGINT, sigintHandler);

    if (pipe(pipefd) == -1) {
        perror("Fehler beim Erzeugen der Pipe");
        return 1;
    }

    while (!shouldExit) {
        int voltage[ARRAY_SIZE];
        int i, sum = 0, min = MAX_VOLTAGE, max = 0;

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
            }
        }

        printf("Sende Messwerte über die Pipe...\n");
        for (i = 0; i < ARRAY_SIZE; i++) {
            if (voltage[i] < 1000) {
                if (write(pipefd[1], &voltage[i], sizeof(int)) == -1) {
                    perror("Fehler beim Schreiben in die Pipe");
                }
            }
        }

        for (i = 0; i < ARRAY_SIZE; i++) {
            sum += voltage[i];
            if (voltage[i] < min) {
                min = voltage[i];
            }
            if (voltage[i] > max) {
                max = voltage[i];
            }
        }

        double average = (double) sum / ARRAY_SIZE;

        printf("\nStatistische Auswertung der Messwerte:\n");
        printf("Summe: %d\n", sum);
        printf("Durchschnitt: %.2f\n", average);
        printf("Minimum: %d\n", min);
        printf("Maximum: %d\n", max);

        sleep(5);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    return 0;
}
