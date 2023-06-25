#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

char prozessID[100];

int pipefd_read[2];  // Pipe-Dateideskriptoren für das Lesen
int pipefd_write1[2];  // Pipe-Dateideskriptoren für das Schreiben in die erste Pipe
int pipefd_write2[2];  // Pipe-Dateideskriptoren für das Schreiben in die zweite Pipe
int pipefd_write3[2];  // Pipe-Dateideskriptoren für das Schreiben in die dritte Pipe

void cleanup(int signum) {
    printf("Prozess wird beendet. Aufräumarbeiten...\n");
    
    close(pipefd_read[0]);  // Pipe schließen
    close(pipefd_read[1]);
    close(pipefd_write1[0]);
    close(pipefd_write1[1]);
    close(pipefd_write2[0]);
    close(pipefd_write2[1]);
    close(pipefd_write3[0]);
    close(pipefd_write3[1]);
    fflush(stdout);
    exit(0);
}

void savePID() {
    char aktuell[10];
    sprintf(aktuell, "%d", getpid());
    strcat(prozessID, aktuell);

    FILE *pid = fopen("pid.txt", "w");
    fputs(prozessID, pid);
    fclose(pid);
}

int main() {
    if (pipe(pipefd_read) == -1) {
        perror("Fehler beim Erzeugen der Lesepipe");
        return 1;
    }

    if (pipe(pipefd_write1) == -1) {
        perror("Fehler beim Erzeugen der ersten Schreibpipe");
        return 1;
    }

    if (pipe(pipefd_write2) == -1) {
        perror("Fehler beim Erzeugen der zweiten Schreibpipe");
        return 1;
    }

    if (pipe(pipefd_write3) == -1) {
        perror("Fehler beim Erzeugen der dritten Schreibpipe");
        return 1;
    }

    signal(SIGINT, cleanup);

    while (1) {
        pid_t pid = fork();  // Fork-Aufruf

        if (pid == -1) {
            perror("Fehler beim Erzeugen des Kindprozesses");
            return 1;
        } else if (pid == 0) {
            // Kindprozess

            // Ignoriere SIGINT-Signal im Kindprozess
            signal(SIGINT, SIG_IGN);

            int voltage[ARRAY_SIZE];
        int character, i, sum = 0, min = MAX_VOLTAGE, max = 0;
        int validCount = 0; // Zähler für gültige Messwerte
        savePID();

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

        // Schreibe gültige Messwerte in die Pipes
        printf("Sende gültige Messwerte über die Pipes...\n");
        for (i = 0; i < ARRAY_SIZE; i++) {
            if (voltage[i] < 1000) {
                if (write(pipefd_write1[1], &voltage[i], sizeof(int)) == -1) {
                    perror("Fehler beim Schreiben in die erste Pipe");
                }
                if (write(pipefd_write2[1], &voltage[i], sizeof(int)) == -1) {
                    perror("Fehler beim Schreiben in die zweite Pipe");
                }
            }
        }

        // Datei erstellen und gültige Messwerte aus der ersten Pipe lesen und schreiben
        FILE *file = fopen("Messwerte.txt", "w");
        if (file == NULL) {
            perror("Fehler beim Erstellen der Datei");
            return 2;
        }

        printf("\nSchreibe gültige Messwerte in die Datei...\n");
        for (i = 0; i < validCount; i++) {
            int value;
            if (read(pipefd_write1[0], &value, sizeof(int)) == -1) {
                perror("Fehler beim Lesen aus der ersten Pipe");
                break;
            }
            fprintf(file, "%d\n", value);
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

        // Messwerte aus der zweiten Pipe lesen und Statistiken berechnen
        printf("\nStatistische Auswertung der gültigen Messwerte:\n");
        if (validCount > 0) {
            int receivedValues[validCount];
            printf("Empfange Messwerte aus der zweiten Pipe...\n");
            for (i = 0; i < validCount; i++) {
                if (read(pipefd_write2[0], &receivedValues[i], sizeof(int)) == -1) {
                    perror("Fehler beim Lesen aus der zweiten Pipe");
                }
            }

            for (i = 0; i < validCount; i++) {
                sum += receivedValues[i];
                if (receivedValues[i] < min) {
                    min = receivedValues[i];
                }
                if (receivedValues[i] > max) {
                    max = receivedValues[i];
                }
            }
            double average = (double) sum / validCount;
            

            // Schreibe Statistiken in die dritte Pipe
            if (write(pipefd_write3[1], &sum, sizeof(int)) == -1) {
                perror("Fehler beim Schreiben der Summe in die dritte Pipe");
            }
            if (write(pipefd_write3[1], &average, sizeof(double)) == -1) {
                perror("Fehler beim Schreiben des Durchschnitts in die dritte Pipe");
            }
            if (write(pipefd_write3[1], &min, sizeof(int)) == -1) {
                perror("Fehler beim Schreiben des Minimums in die dritte Pipe");
            }
            if (write(pipefd_write3[1], &max, sizeof(int)) == -1) {
                perror("Fehler beim Schreiben des Maximums in die dritte Pipe");
            }
        } else {
            printf("Keine gültigen Messwerte vorhanden.\n");
        }

        // Lese Statistiken aus der dritten Pipe und gebe sie auf der Konsole aus
        printf("\nEmpfange Statistiken aus der dritten Pipe...\n");
        int receivedSum, receivedMin, receivedMax;
        double receivedAverage;

        if (read(pipefd_write3[0], &receivedSum, sizeof(int)) == -1) {
            perror("Fehler beim Lesen der Summe aus der dritten Pipe");
        }
        if (read(pipefd_write3[0], &receivedAverage, sizeof(double)) == -1) {
            perror("Fehler beim Lesen des Durchschnitts aus der dritten Pipe");
        }
        if (read(pipefd_write3[0], &receivedMin, sizeof(int)) == -1) {
            perror("Fehler beim Lesen des Minimums aus der dritten Pipe");
        }
        if (read(pipefd_write3[0], &receivedMax, sizeof(int)) == -1) {
            perror("Fehler beim Lesen des Maximums aus der dritten Pipe");
        }

        printf("Empfangene Statistiken:\n");
        printf("Summe: %d\n", receivedSum);
        printf("Durchschnitt: %.2f\n", receivedAverage);
        printf("Minimum: %d\n", receivedMin);
        printf("Maximum: %d\n", receivedMax);

        sleep(30);


            exit(0);  // Kindprozess beenden

            savePID();
            fflush(stdout);

            exit(0);  // Kindprozess beenden
        } else {
            // Elternprozess
            // Wartezeit zwischen den Forks
            sleep(10);
        }
    }
    
    return 0;
}
