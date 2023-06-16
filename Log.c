#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10

int main() {
    int voltage[ARRAY_SIZE];
    size_t i, r;
    FILE *file;
    int character;

    // LOG

    // Datei erstellen
    file = fopen("Messwerte.txt", "w");
    if (file == NULL) {
        // Falls Datei nicht erstellt werden konnte, Fehlermeldung ausgeben
        printf("Die Datei wurde nicht richtig erstellt.\n");
        return 1;
    } else {
        // Falls Datei erstellt werden konnte, Meldung ausgeben
        printf("Die Datei wurde richtig erstellt.\n");
    }

    // Messwerte in Datei schreiben
    r = 0; // Anzahl der geschriebenen Messwerte zurücksetzen
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (voltage[i] < 1000) {
            fprintf(file, "%d\n", voltage[i]);
            r++;
        }
    }
    printf("%d Messwerte wurden erfolgreich in die Datei geschrieben.\n", r);

    // Datei schließen. Wenn die Datei nicht geschlossen wird, kann sie nicht gelesen werden.
    fclose(file);

    printf("\nTextausgabe zum Überprüfen des Inhalts.\n");

    // Datei öffnen
    file = fopen("Messwerte.txt", "r");

    // Fehlermeldung, falls Datei nicht geöffnet werden konnte
    if (file == NULL) {
        printf("Die Datei konnte nicht geöffnet werden.\n");
        return 1;
    }

    // Falls Datei geöffnet werden konnte, jede einzelne Zeichen ausgeben
    while ((character = fgetc(file)) != EOF) {
        putchar(character);
    }

    // Datei schließen.
    fclose(file);

    return 0;
}
