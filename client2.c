#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int clientSocket;
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
        printf("Min: %d, Max: %d, Mittelwert: %.2f, Summe: %d\n",
               statistics[0], statistics[1], (double)statistics[2], statistics[3]);

        printf("____\n");

        // Wartezeit zwischen den Empfängen
        sleep(5);
    }

    close(clientSocket);  // Client2-Socket schließen

    return 0;
}
