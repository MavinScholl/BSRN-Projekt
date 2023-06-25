#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

char prozessID[100];

int clientSocket;

void savePID() {
    char aktuell[10];
    sprintf(aktuell, "%d", getpid());
    strcat(prozessID, aktuell);

    FILE *pid = fopen("pid.txt", "w");
    fputs(prozessID, pid);
    fclose(pid);
}

void handleSignal(int signal) {
    if (signal == SIGINT) {
        printf("Programmabbruch durch SIGINT (Strg+C)\n");
        close(clientSocket);
        exit(0);
    }
}

int main() {
    signal(SIGINT, handleSignal);

    while (1) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fehler beim Erzeugen des Kindprozesses");
            return 1;
        } else if (pid == 0) {
            // Kindprozess
            savePID();

            struct sockaddr_in serverAddr;

            // Client-Socket erstellen
            if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("Fehler beim Erzeugen des Client-Sockets");
                return 1;
            }

            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(12346);  // Beispiel-Portnummer
            serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Beispiel-IP-Adresse

            // Verbindung zum Server2 herstellen
            if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
                perror("Fehler beim Herstellen der Verbindung zum Server2");
                return 1;
            }

            printf("Verbindung zum Server2 hergestellt\n");

            while (1) {
                int statistics[4];

                // Daten von Server2 empfangen
                if (recv(clientSocket, statistics, sizeof(statistics), 0) == -1) {
                    perror("Fehler beim Empfangen der Daten von Server2");
                    break;
                }

                // Daten ausgeben
                printf("\nMin: %d\nMax: %d\nMittelwert: %.2lf\nSumme: %d\n",
                       statistics[0], statistics[1], (double)statistics[2] /100, statistics[3]);

                printf("_______________\n");

                // Wartezeit zwischen den Empfängen
                sleep(10);
            }

            close(clientSocket);  // Client2-Socket schließen

            return 0;
        } else {
            // Elternprozess
            savePID();
            // Wartezeit zwischen den Forks
            sleep(1);
            break;  // Den Elternprozess aus der Schleife beenden
        }
    }

    while (1) {
        // Warten auf den Abschluss der Kindprozesse
        int status;
        pid_t finishedPid = waitpid(-1, &status, 0);

        if (finishedPid == -1) {
            break;  // Wenn keine Kindprozesse mehr vorhanden sind, die Schleife beenden
        }
    }

    return 0;
}
