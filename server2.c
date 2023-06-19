#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ARRAY_SIZE 10

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

    while (1) {
        int numbers[ARRAY_SIZE];

        // Datei öffnen und Zahlen lesen
        FILE* file = fopen("Messwerte.txt", "r");
        if (file == NULL) {
            perror("Fehler beim Öffnen der Datei");
            break;
        }

        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (fscanf(file, "%d", &numbers[i]) != 1) {
                perror("Fehler beim Lesen der Zahlen aus der Datei");
                fclose(file);
                break;
            }
        }

        fclose(file);

        // Daten an Client2 senden
        if (send(clientSocket, numbers, sizeof(numbers), 0) == -1) {
            perror("Fehler beim Senden der Daten an Client2");
            break;
        }
    }

    close(clientSocket);  // Client2-Socket schließen
    close(serverSocket);  // Server-Socket schließen

    return 0;
}
