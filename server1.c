#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define ARRAY_SIZE 10

int serverSocket, clientSocket;

void generateRandomNumbers(int numbers[], int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        numbers[i] = rand() % 1201;  // Zufallszahl zwischen 0 und 1200 generieren
    }
}

void cleanup() {
    close(clientSocket);  // Client1-Socket schließen
    close(serverSocket);  // Server1-Socket schließen
    // Weitere Aufräumarbeiten, falls erforderlich
    exit(0);
}

void handleSignal(int signal) {
    if (signal == SIGINT) {
        printf("Programm wird beendet... Ressourcen werden freigegeben.\n");
        cleanup();
    }
}

int main() {
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Signal-Handler für SIGINT registrieren
    signal(SIGINT, handleSignal);

    // Server-Socket erstellen
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Fehler beim Erzeugen des Server-Sockets");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // Beispiel-Portnummer
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

    printf("Server1 läuft und wartet auf Verbindung von Client1...\n");

    // Verbindung von Client1 akzeptieren
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen)) == -1) {
        perror("Fehler beim Akzeptieren der Verbindung");
        return 1;
    }

    printf("Client1 verbunden: %s\n", inet_ntoa(clientAddr.sin_addr));

    while (1) {
        pid_t pid = fork();  // Fork-Aufruf

        if (pid == -1) {
            perror("Fehler beim Erzeugen des Kindprozesses");
            cleanup();  // Aufräumarbeiten bei Fehler
        } else if (pid == 0) {
            // Kindprozess

            // Signal-Handler für SIGINT im Kindprozess ignorieren
            signal(SIGINT, SIG_IGN);

            int numbers[ARRAY_SIZE];

            // Zufallszahlen generieren
            generateRandomNumbers(numbers, ARRAY_SIZE);

            // Daten an Client1 senden
            if (send(clientSocket, numbers, sizeof(numbers), 0) == -1) {
                perror("Fehler beim Senden der Daten an Client1");
                cleanup();  // Aufräumarbeiten bei Fehler
            }

            exit(0);  // Kindprozess beenden
        } else {
            // Elternprozess
            // Wartezeit zwischen den Forks
            sleep(10);
        }
    }
}
