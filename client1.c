#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ARRAY_SIZE 10

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Client-Socket erstellen
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Fehler beim Erzeugen des Client-Sockets");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // Beispiel-Portnummer
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Beispiel-IP-Adresse

    // Verbindung zum Server1 herstellen
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Fehler beim Herstellen der Verbindung zum Server1");
        return 1;
    }

    printf("Verbindung zum Server1 hergestellt\n");

    while (1) {
        int numbers[ARRAY_SIZE];

        // Daten von Server1 empfangen
        if (recv(clientSocket, numbers, sizeof(numbers), 0) == -1) {
            perror("Fehler beim Empfangen der Daten von Server1");
            break;
        }

                // Datei leeren
        FILE* file = fopen("Messwerte.txt", "w");
        fclose(file);

        // Zahlen in die Datei schreiben
        file = fopen("Messwerte.txt", "w");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            if (numbers[i] <= 1000) {
                fprintf(file, "%d\n", numbers[i]);
            }
        }
        fclose(file);


        // Wartezeit zwischen den Empfängen
        sleep(10);
    }

    close(clientSocket);  // Client1-Socket schließen

    return 0;
}
