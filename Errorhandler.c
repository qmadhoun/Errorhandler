#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://172.20.10.4:1883" // !! IP-address:Port-ID
#define QOS         1
#define CLIENTID    "Qossay"
#define SUB_TOPIC   "ERROR_IN" 
#define PUB_TOPIC   "ERROR_SEND"
#define TOPIC_LEN   120
#define TIMEOUT     500L

#define ERR_OUT_LEN 1024

volatile MQTTClient_deliveryToken deliveredtoken;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct ErrorMessage {
    char code[10];
    char message[250];
    struct ErrorMessage *next;
};
struct ErrorMessage *head = NULL;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void insert_first(char *code, char *message) {
    struct ErrorMessage *new_record = (struct ErrorMessage *)malloc(sizeof(struct ErrorMessage));
    strcpy(new_record->code, code);
    strcpy(new_record->message, message);
    new_record->next = head;
    head = new_record;  
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void insert_next(struct ErrorMessage *list, char *code, char *message) {
    struct ErrorMessage *new_record = (struct ErrorMessage*) malloc(sizeof(struct ErrorMessage));
    strcpy(new_record->code, code);
    strcpy(new_record->message, message);
    new_record->next = NULL;
    list->next = new_record;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
    
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// This function is called upon when a message is successfully delivered through mqtt
void delivered(void *context, MQTTClient_deliveryToken dt) {
    
    printf("Message with token value %d delivery confirmed\n", dt);
    printf( "-----------------------------------------------\n" );    
    deliveredtoken = dt;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// This function is called upon when an incoming message from mqtt is arrived
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *error_in = message->payload;
    char subsystem[50], error_code[10], parameter[100];
    int sev_code;
    char  error_out[ ERR_OUT_LEN ] = "";
    char formatted_message[ERR_OUT_LEN] = "";

    
    
    // print incoming message
    printf( "msgarrvd: error_in: <%s>\n", error_in );   

    int errorfields_read = sscanf(error_in, "%d;%49[^;];%9[^;];%99[^\n]", &sev_code, subsystem, error_code);
    if (errorfields_read < 3) {
        strncpy(error_out, "Invalid message format", ERR_OUT_LEN);
        printf("msgarrvd: error_out: <%s>\n", error_out);
        return 1; 
    }

    if (sev_code  > 4 || sev_code < 0)
    {
        sev_code = 0;
    }
    
    
    // format error out msg
   
    struct ErrorMessage *found = NULL;
     if (search_list(&found, error_code)) {
        // Replace %s in the error message with the parameter
        if (strstr(found->message, "%s")) {
            snprintf(error_out, ERR_OUT_LEN, found->message, parameter);
        } else {
            strcpy(error_out, found->message);
        }
    } else {
        snprintf(error_out, ERR_OUT_LEN, "Unknown error code: %s", error_code);
    }

    time_t now = time(NULL);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    snprintf(formatted_message, ERR_OUT_LEN, "%s;%s;%s;%s;%s", timestamp, sev_code, subsystem, error_code, error_out);
    strncpy(error_out,formatted_message,ERR_OUT_LEN);

    printf( "msgarrvd: error_out: <%s>\n", error_out );   


    // Create a new client to publish the error_out message
    MQTTClient client = (MQTTClient)context;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = error_out;
    pubmsg.payloadlen = strlen( error_out );
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    //Publish the error_out message on PUB TOPIC 
    MQTTClient_publishMessage(client, PUB_TOPIC, &pubmsg, &token);
    printf("Publishing to topic %s\n", PUB_TOPIC);
    
    // Validate that message has been successfully delivered
    int rc = MQTTClient_waitForCompletion(client, token, TIMEOUT );
    printf("Message with delivery token %d delivered, rc=%d\n", token, rc);
    printf( "Msg out:\t<%s>\n", error_out );

    // Close the outgoing message queue
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// This function is called upon when the connection to the mqtt-broker is lost
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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


    //----------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

// Open MQTT client for listening
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Define the correct call back functions when messages arrive
    MQTTClient_setCallbacks(client, client, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribing to topic %s for client %s using QoS%d\n\n", SUB_TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, SUB_TOPIC, QOS);

    // Keep the program running to continue receiving and publishing messages
    for(;;) {
        ;
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;

    
}