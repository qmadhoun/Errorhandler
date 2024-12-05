#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://172.20.10.4:1883" // !! IP-address:Port-ID
#define QOS         1
#define CLIENTID    "FRANK"
#define SUB_TOPIC   "FD/ERROR_IN"
#define PUB_TOPIC   "FD/ERROR_SEND"
#define TOPIC_LEN   120
#define TIMEOUT     500L

#define ERR_OUT_LEN 1024

// this mqtt token is set as global var to ease up this program
volatile MQTTClient_deliveryToken deliveredtoken;

// This function is called upon when a message is successfully delivered through mqtt
void delivered(void *context, MQTTClient_deliveryToken dt) {
    
    printf("Message with token value %d delivery confirmed\n", dt);
    printf( "-----------------------------------------------\n" );    
    deliveredtoken = dt;
}

// This function is called upon when an incoming message from mqtt is arrived
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *error_in = message->payload;
    char  error_out[ ERR_OUT_LEN ] = "";
    
    // print incoming message
    printf( "msgarrvd: error_in: <%s>\n", error_in );   
    
    // format error out msg
    sprintf( error_out, "%s + Some additional text here", error_in );
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

// This function is called upon when the connection to the mqtt-broker is lost
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}


int main() {
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
