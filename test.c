#include "../lib/mosquitto.h"
#include <czmq.h>

void
my_message_callback (struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    int i;
    bool res;

    if (message->payloadlen) {
        printf ("%s ", message->topic);
        fwrite (message->payload, 1, message->payloadlen, stdout);
        fflush (stdout);
        printf ("\n");
    }

}

void my_log_callback (struct mosquitto *mosq, void *obj, int level, const char *str)
{
    zsys_debug ("(%d) %s", level, str);
}

void my_subscribe_callback (struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i = 0;

    printf ("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for (i=1; i < qos_count; i++) {
        printf (", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_connect_callback (struct mosquitto *mosq, void *obj, int result)
{
    int i;
    struct mosq_config *cfg;

    assert(obj);
    cfg = (struct mosq_config *)obj;

    if (!result) {
        mosquitto_subscribe (mosq, NULL, "a/b", 2);
    }
    else {
        zsys_error ("%s\n", mosquitto_connack_string (result));
    }
}

int
main (int argc, char **argv) {
    bool debug = true;

    mosquitto_lib_init ();

    struct mosquitto *client = mosquitto_new (NULL, true, NULL);
    assert (client);

    if (debug) {
        mosquitto_log_callback_set (client, my_log_callback);
        mosquitto_subscribe_callback_set (client, my_subscribe_callback);
    }

    mosquitto_connect_callback_set (client, my_connect_callback);
    mosquitto_message_callback_set (client, my_message_callback);

    int rc = mosquitto_connect_bind (client, "localhost", 1883, 60, NULL);
    if (rc > 0) {
        zsys_error ("client_connect () failed.");
        return EXIT_FAILURE;
    }

    rc = mosquitto_loop_forever (client, -1, 1);

    mosquitto_destroy (client);
    mosquitto_lib_cleanup ();

    return EXIT_SUCCESS;
}
