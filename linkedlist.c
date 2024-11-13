#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct definitie voor een enkel record
struct tbl {
    char naam[51];
    char voornaam[30];
    char geboortedatum[11];
    char straat[50];
    int huisnummer;
    char stad[50];
    int postcode;
    struct tbl *next;
};

// Pointer naar het begin van de linked list
struct tbl *head = NULL;

// Functie om het eerste record toe te voegen
void insert_first(char *naam, char *voornaam, char *geboortedatum, char *straat, int huisnummer, char *stad, int postcode) {
    struct tbl *new_record = (struct tbl*) malloc(sizeof(struct tbl));
    strcpy(new_record->naam, naam);
    strcpy(new_record->voornaam, voornaam);
    strcpy(new_record->geboortedatum, geboortedatum);
    strcpy(new_record->straat, straat);
    new_record->huisnummer = huisnummer;
    strcpy(new_record->stad, stad);
    new_record->postcode = postcode;
    new_record->next = head;
    head = new_record;
}

// Functie om een volgend record toe te voegen aan de linked list
void insert_next(struct tbl *list, char *naam, char *voornaam, char *geboortedatum, char *straat, int huisnummer, char *stad, int postcode) {
    struct tbl *new_record = (struct tbl*) malloc(sizeof(struct tbl));
    strcpy(new_record->naam, naam);
    strcpy(new_record->voornaam, voornaam);
    strcpy(new_record->geboortedatum, geboortedatum);
    strcpy(new_record->straat, straat);
    new_record->huisnummer = huisnummer;
    strcpy(new_record->stad, stad);
    new_record->postcode = postcode;
    new_record->next = NULL;
    list->next = new_record;
}

// Functie om alle records in de linked list af te drukken
void print_list() {
    struct tbl *temp = head;
    int count = 1;
    printf("Lijst:\n\n");
    while (temp != NULL) {
        printf("%d: %s\t%s\t%s\t%s\t%d\t%s\t%d\n", count, temp->naam, temp->voornaam, temp->geboortedatum, temp->straat, temp->huisnummer, temp->stad, temp->postcode);
        temp = temp->next;
        count++;
    }
    printf("Einde van de Lijst:\n");
}

// Functie om te zoeken naar een naam in de linked list
int search_list(struct tbl **list, char *naam) {
    struct tbl *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->naam, naam) == 0) {
            *list = temp;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

int main(int argc , char* argv[]) {
    char buffer[100];
    char filename[100];
    char naam[51], voornaam[30], geboortedatum[11], straat[50], stad[50];
    int huisnummer, postcode;

    if (argc != 2) {
        strcpy(filename, "naamlijst.txt");
    } else {
        strcpy(filename, argv[1]);
    }

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("Error opening file %s\n", filename);
        return 1;
    }

    while (fgets(buffer, 100, fp) != NULL) {
        if (buffer[0] == '#') {
            continue;
        }

        int fields_read = sscanf(buffer, "%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%d", naam, voornaam, geboortedatum, straat, &huisnummer, stad, &postcode);

        if (fields_read != 7) {
            printf("Fout: onjuiste invoer gevonden: %s\n", buffer);
            continue;
        }

        if (head == NULL) {
            insert_first(naam, voornaam, geboortedatum, straat, huisnummer, stad, postcode);
        } else {
            struct tbl *current = head;
            while (current->next != NULL) {
                current = current->next;
            }
            insert_next(current, naam, voornaam, geboortedatum, straat, huisnummer, stad, postcode);
        }
    }

    fclose(fp);

    // Print alle records
    print_list();

    // Zoek naar een specifieke naam
    char search_name[50];
    printf("Voer een naam in om te zoeken: ");
    scanf("%s", search_name);

    struct tbl *found = NULL;
    if (search_list(&found, search_name)) {
        printf("Record gevonden: %s\t%s\t%s\t%s\t%d\t%s\t%d\n",
               found->naam, found->voornaam, found->geboortedatum, found->straat, found->huisnummer, found->stad, found->postcode);
    } else {
        printf("Geen record gevonden voor de naam %s.\n", search_name);
    }

    return 0;
}
