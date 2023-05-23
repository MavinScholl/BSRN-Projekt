#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10


int shouldExit = 0;

// Signal Handler für Strg+C---> Überprüfen warum es nicht funktioniert. Statt die Nachricht kommt nur ein Zeichen bzw. ^C.
void sigintHandler(int signum) {
    printf("\nProgramm wurde abgebrochen.\n");
    fflush(stdout);  
    shouldExit = 1;
}
int main() {
    // Signal Handler für Strg+C registrieren

    signal(SIGINT, sigintHandler);
while (!shouldExit) {
    int voltage[ARRAY_SIZE];
    
    char buffer[2];
    int i, r, character, mittelwert, summe, min, max;
    FILE *file;


    //Random nummer initialisieren und generieren
    srand(time(NULL));

    //Hier fängt die Messung an. 10 Nummer werden generiert und in Array gespeichert
    for (size_t i = 0; i < 10; i++)
    {     voltage[i]=  rand()%MAX_VOLTAGE;
     }

     for (size_t r = 0; r < ARRAY_SIZE; r++)
     {
        if (voltage[r]<1000)

    //Ausgabe der Messwerte
    printf("Messwert: %d\n", voltage[r]);

    //Fehlermeldung falls Messwert größer als 1000
    else printf("Der Messwert %d überschreit die Grenze von 1000.\n", voltage[r]);
     }
     
 //Ab hier fängt log an

 //Datei erstellen
  file = fopen("Messwerte.txt", "w");
    if (file == NULL) {
        // Falls Datei nicht erstellt werden konnte, Fehlermeldung ausgeben
        printf("Die Datei wurde nicht richtig erstellt.\n");
        return 1;
    }

    // Falls Datei erstellt werden konnte, Meldung ausgeben
    else printf("Die Datei wurde richtig erstellt.\n");

    // Messwerte in Datei schreiben
    r = 0; // Anzahl der geschriebenen Messwerte zurücksetzen
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (voltage[i]< 1000) {
             fprintf(file, "%d\n", voltage[i]);
             r++;
             
        }
        
       
    }
    printf("%d Messwerte wurden erforglreich in die Datei geschrieben.\n", r);

    // Datei schließen. Wenn die Datei nicht geschlossen wird kann sie nicht gelesen werden.
    fclose(file);

    printf("\nText ausgabe zum Ǜberprüfen des Inhalts .\n");


    //Datei öffnen
     file = fopen("Messwerte.txt", "r");

     //Fehlermeldung falls Datei nicht geöffnet werden konnte
    if (file == NULL) {
        printf("Die Datei konnte nicht geöffnet werden.\n");
        return 1;
    }

    //Falls Datei geöffnet werden konnte, jede einzelne char ausgeben
    while ((character = fgetc(file)) != EOF) {
        putchar(character);
    }

    // Datei schließen.  
    fclose(file);

    //Hier fängt Stat an.
    summe = 0;
    mittelwert = 0;
    min = voltage[0];

    for (i=0; i<ARRAY_SIZE; i++) {
        // Summe für Mittelwert berechnen
        mittelwert = mittelwert + voltage[i];

        // Summe berechnen
        summe= summe + voltage[i];

        // Kleinster Wert
        if (voltage[i]<min)
        {
            min = voltage[i];
        }
        max = voltage[0];

        // Größter Wert
        if (voltage[i]>max)
        {
            max = voltage[i];
        }
        
            }

            // Mittelwert berechnen
    mittelwert = mittelwert / ARRAY_SIZE;

    //Hier fängt report an. Ausgabe Mittelwert
    printf("\nDer Mittelwert ist: %d\n", mittelwert);
    
    // Ausgabe kleinster Wert
    printf("Der kleinste Wert ist: %d\n", min);
    
    // Ausgabe größter Wert
    printf("Der größte Wert ist: %d\n", max);
    
    // Ausgabe Summe
    printf("Die Summe ist: %d\n\n", summe);

    // 1Beliebte Zeit warten
        sleep(5);


    


    //Puffert wird geelert, Anzeige wird direkt ausgegeben
    fflush(stdout); 


  

}

    return 0;
}
