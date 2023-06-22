#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ARRAY_SIZE 10

void calculateStatistics(int numbers[], int size, int* min, int* max, double* average, int* sum) {
    *min = numbers[0];
    *max = numbers[0];
    *sum = 0;

    for (int i = 0; i < size; i++) {
        if (numbers[i] < *min) {
            *min = numbers[i];
        }
        if (numbers[i] > *max) {
            *max = numbers[i];
        }
        *sum += numbers[i];
    }

    *average = (double)*sum / size;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Server-Socket erstellen
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Fehler beim Erzeugen des Server-Sockets");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12346);  // Beispiel-Portnummer
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Server-Socket an Port binden
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Fehler beim Binden des Server-Sockets");
        return 1;
    }

    // Auf eingehende Verbindungen warten
    if (listen(serverSocket, 1) == -1) {
        perror("Fehler beim Warten auf eingehende Verbindungen");
        return 1;
    }

    printf("Server2 läuft und wartet auf Verbindung von Client2...\n");

    // Verbindung von Client2 akzeptieren
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen)) == -1) {
        perror("Fehler beim Akzeptieren der Verbindung");
        return 1;
    }

    printf("Client2 verbunden: %s\n", inet_ntoa(clientAddr.sin_addr));

    int numbers[ARRAY_SIZE];
    int i = 0;

    while (1) {
        // Datei öffnen und Zahlen lesen
        FILE* file = fopen("Messwerte.txt", "r");
        if (file == NULL) {
            perror("Fehler beim Öffnen der Datei");
            return 1; // Fehlerbehandlung durch Rückgabe
        }

        i = 0; // Reset der Zähler

        while (fscanf(file, "%d", &numbers[i]) == 1) {
            i++;
            if (i >= ARRAY_SIZE) {
                break; // Maximale Anzahl von Zahlen erreicht
            }
        }

        if (feof(file)) {
            // Dateiende erreicht, alle verfügbaren Zahlen wurden gelesen
            fclose(file);

            // Statistiken berechnen
            int min, max, sum;
            double average;
            calculateStatistics(numbers, i, &min, &max, &average, &sum);

            // Statistiken an Client2 senden
            int statistics[4] = { min, max, (int)average, sum };
            if (send(clientSocket, statistics, sizeof(statistics), 0) == -1) {
                perror("Fehler beim Senden der Daten an Client2");
                return 1; // Fehlerbehandlung durch Rückgabe
            }
        } else {
            // Fehler beim Lesen der Zahlen aus der Datei
            perror("Fehler beim Lesen der Zahlen aus der Datei TEST");
            fclose(file);
            return 1; // Fehlerbehandlung durch Rückgabe
        }

        sleep(10); // 10 Sekunden warten
    }

    close(clientSocket);  // Client2-Socket schließen
    close(serverSocket);  // Server-Socket schließen

    return 0;
}
