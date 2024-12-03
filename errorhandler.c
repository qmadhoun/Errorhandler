#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct definitie voor een enkel error record
struct ErrMessage {
    char code[10];
    char message[250];
    struct ErrMessage *next;
};

// Pointer naar het begin van de linked list
struct ErrMessage *head = NULL;

// Functie om het eerste record toe te voegen
void insert_first(char *code, char *message) {
    struct ErrMessage *new_record = (struct ErrMessage *)malloc(sizeof(struct ErrMessage));
    if (!new_record) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(new_record->code, code);
    strcpy(new_record->message, message);
    new_record->next = head;
    head = new_record;
}

// Functie om een volgend record toe te voegen aan de linked list
void insert_next(struct ErrMessage *list, char *code, char *message) {
    struct ErrMessage *new_record = (struct ErrMessage *)malloc(sizeof(struct ErrMessage));
    if (!new_record) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(new_record->code, code);
    strcpy(new_record->message, message);
    new_record->next = NULL;
    list->next = new_record;
}

// Functie om alle error records in de linked list af te drukken
void print_list() {
    struct ErrMessage *temp = head;
    int count = 1;
    printf("Error Messages:\n\n");
    while (temp != NULL) {
        printf("%d: Code: %s\tMessage: %s\n", count, temp->code, temp->message);
        temp = temp->next;
        count++;
    }
    printf("End of List:\n");
}

// Functie om te zoeken naar een error code in de linked list
int search_list(struct ErrMessage **list, char *code) {
    struct ErrMessage *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->code, code) == 0) {
            *list = temp;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char buffer[300];
    char filename[100];
    char code[10], message[250];

    // Bepaal het juiste bestand op basis van de taalparameter
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <language: EN|FR|NLD>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "EN") == 0) {
        strcpy(filename, "/mnt/data/Error_msg_EN.txt");
    } else if (strcmp(argv[1], "FR") == 0) {
        strcpy(filename, "/mnt/data/Error_msg_FR.txt");
    } else if (strcmp(argv[1], "NLD") == 0) {
        strcpy(filename, "/mnt/data/Error_msg_NLD.txt");
    } else {
        fprintf(stderr, "Unsupported language. Use EN, FR, or NLD.\n");
        return 1;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    // Lees het bestand regel per regel
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (buffer[0] == '#' || buffer[0] == '\n') {
            continue; // Sla opmerkingen en lege regels over
        }

        int fields_read = sscanf(buffer, "%9s %[^\n]", code, message);
        if (fields_read != 2) {
            fprintf(stderr, "Invalid line format: %s\n", buffer);
            continue;
        }

        if (head == NULL) {
            insert_first(code, message);
        } else {
            struct ErrMessage *current = head;
            while (current->next != NULL) {
                current = current->next;
            }
            insert_next(current, code, message);
        }
    }

    fclose(fp);

    // Druk alle geladen records af
    print_list();

    // Zoek naar een specifieke error code
    char search_code[10];
    printf("Enter an error code to search for: ");
    scanf("%9s", search_code);

    struct ErrMessage *found = NULL;
    if (search_list(&found, search_code)) {
        printf("Found Record: Code: %s\tMessage: %s\n", found->code, found->message);
    } else {
        printf("No record found for the code %s.\n", search_code);
    }

    return 0;
}
