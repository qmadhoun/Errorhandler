#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ErrorMessage {
    char code[10];
    char message[250];
    struct ErrorMessage *next;
};

struct ErrorMessage *head = NULL;

void insert_first(char *code, char *message) {
    struct ErrorMessage *new_record = (struct ErrorMessage *)malloc(sizeof(struct ErrorMessage));
    strcpy(new_record->code, code);
    strcpy(new_record->message, message);
    new_record->next = head;
    head = new_record;  
}

void insert_next(struct ErrorMessage *list, char *code, char *message) {
    struct ErrorMessage *new_record = (struct ErrorMessage*) malloc(sizeof(struct ErrorMessage));
    strcpy(new_record->code, code);
    strcpy(new_record->message, message);
    new_record->next = NULL;
    list->next = new_record;
}

void print_list() {
    struct ErrorMessage *temp = head;
    int count = 1;
    printf("Lijst:\n\n");
    while (temp != NULL) {
        printf("%d: Code: %s\tMessage: %s\n", count, temp->code, temp->message);
        temp = temp->next;
        count++;
    }
    printf("Einde van de Lijst:\n");
}

int search_list(struct ErrorMessage **list, char *code) {
    struct ErrorMessage *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->code, code) == 0) {
            *list = temp;
            return 1;
        }
        temp = temp->next;
    }
    return 0;}

int main(int argc , char* argv[]) {
    char buffer[300];
    char filename[100] = "";
    char code[10], message[250];

    if (argc != 2) {
        printf("%s language: EN|FR|NLD", argv[0]);
        return 1;
    } 

    if (strcmp(argv[1] , "EN") == 0)
    {
        strcpy(filename, "/home/qmadhounrpi5/Downloads/Error_msg_EN.txt");
    }

    else if (strcmp(argv[1] , "FR") == 0)
    {
        strcpy(filename, "/home/qmadhounrpi5/Downloads/Error_msg_FR.txt");
    }
    else if (strcmp(argv[1] , "NLD") == 0)
    {
        strcpy(filename, "/home/qmadhounrpi5/Downloads/Error_msg_NLD.txt");
    }

    else{
        printf("Error: Unsupported language, choose EN,FR,NLD\n");
        return 1;
    }


    
    

    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("Error opening file %s\n", filename);
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (buffer[0] == '#') {
            continue;
        }

        int fields_read = sscanf(buffer, "%9s %[^\n]", code, message);

        if (fields_read != 2) {
            printf("Invalid line fromat: %s\n", buffer);
            continue;
        }

        if (head == NULL) {
            insert_first(code , message);
        } else {
            struct ErrorMessage *current = head;
            while (current->next != NULL) {
                current = current->next;
            }
            insert_next(current, code, message);
        }
    }

    fclose(fp);

    // Print alle records
    print_list();

    // Zoek naar een specifieke code
    char search_code[50];
    printf("Enter a code to search for: ");
    scanf("%9s", search_code);

    struct ErrorMessage *found = NULL;
    if (search_list(&found, search_code)) {
        printf("Found record: Code: %s\tMessage: %s\n" , found->code, found->message);
    } else {
        printf("No record found for the searched code %s.\n", search_code);
    }

    return 0;
}