#include "../lib/mosquitto.h"
#include <czmq.h>

typedef struct mosquitto* mosquitto_t;

int main () {
    char *topic = "bara"; 
    const void *payload = "can you hear me";        
        
    mosquitto_t client = mosquitto_new (
        "exp-client",
        false,
        NULL
    );
    assert (client);

    // create client
    int rv = mosquitto_connect (client,"localhost", 1883, 5);
    if (rv != MOSQ_ERR_SUCCESS)
        printf ("connect: ok\n");

    // publish message
    rv = mosquitto_publish (client, NULL, topic, strlen (payload), payload, 0, true);
    if (rv != MOSQ_ERR_SUCCESS)
        printf ("publish error");

    rv = mosquitto_disconnect (client);
    if (rv != MOSQ_ERR_SUCCESS)
        printf ("disconnect error");
    
    mosquitto_destroy (client);
    client = NULL;   
}

