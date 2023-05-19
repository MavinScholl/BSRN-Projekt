#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define MAX_VOLTAGE 1200
#define ARRAY_SIZE 10
#
int main() {
    int voltage[ARRAY_SIZE];
    
    char buffer[2];
    int i, r, character;
    FILE *file;


    //Random nummer initialisieren und generieren
    srand(time(NULL));
   
    for (size_t i = 0; i < 10; i++)
    {     voltage[i]=  rand()%MAX_VOLTAGE;
     }

     for (size_t r = 0; r < ARRAY_SIZE; r++)
     {
        if (voltage[r]<1000)
    printf("Messwert: %d\n", voltage[r]);
    else printf("Der Messwert %d überschreit die Grenze von 1000.\n", voltage[r]);
     }
     
 //Ab hier fängt log an
 
  file = fopen("Messwerte.txt", "w");
    if (file == NULL) {
        printf("Die Datei wurde nicht richtig erstellt.\n");
        return 1;
    }
    else printf("Die Datei wurde richtig erstellt.\n");
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (voltage[i]< 1000)
        {
             fprintf(file, "%d\n", voltage[i]);
             r++;
             
        }
        
       
    }
    printf("%d Messwerte wurden erforglreich in die Datei geschrieben.\n", r);

    fclose(file);

    printf("\nText ausgabe zum Ǜberprüfen des Inhalts .\n");

     file = fopen("Messwerte.txt", "r");
    if (file == NULL) {
        printf("Die Datei konnte nicht geöffnet werden.\n");
        return 1;
    }

    while ((character = fgetc(file)) != EOF) {
        putchar(character);
    }

    fclose(file);



   // printf("Die Daten wurden erfolgreich in die Datei geschrieben.\n");


  

   

    return 0;
}
