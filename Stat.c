#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

int main() {
    int voltage[ARRAY_SIZE];
    size_t i;
    int summe = 0;
    int mittelwert = 0;
    int min = voltage[0];
    int max = voltage[0];

    // STAT
    

    for (i = 0; i < ARRAY_SIZE; i++) {
        // Summe für Mittelwert berechnen
        mittelwert = mittelwert + voltage[i];

        // Summe berechnen
        summe = summe + voltage[i];

        // Kleinster Wert
        if (voltage[i] < min) {
            min = voltage[i];
        }

        // Größter Wert
        if (voltage[i] > max) {
            max = voltage[i];
        }
    }

    // Mittelwert berechnen
    mittelwert = mittelwert / ARRAY_SIZE;

    return 0;
}

