#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

int main() {
    int voltage[ARRAY_SIZE];
    size_t i, r;

    // Random-Nummer initialisieren und generieren
    srand(time(NULL));

    // Hier fängt die Messung an. 10 Nummern werden generiert und in ein Array gespeichert
    for (i = 0; i < ARRAY_SIZE; i++) {
        voltage[i] = rand() % MAX_VOLTAGE;
    }

    // Ausgabe der Messwerte
    for (r = 0; r < ARRAY_SIZE; r++) {
        if (voltage[r] < 1000) {
            printf("Messwert: %d\n", voltage[r]);
        } else {
            printf("Der Messwert %d überschreitet die Grenze von 1000.\n", voltage[r]);
        }
    }

    return 0;
}
