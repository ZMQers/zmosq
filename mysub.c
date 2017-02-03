#include "../lib/mosquitto.h"
#include <czmq.h>

typedef struct mosquitto* mosquitto_t;

static void
s_connect (struct mosquitto *mosq, void *obj, int result) {
    fprintf (stderr, "D: s_connect, result=%d\n", result);

    if (!result) {
        fprintf (stderr, "D: !s_connect, !result\n");
        int r = mosquitto_subscribe (mosq, NULL, "TEST", 0);
        assert (r == MOSQ_ERR_SUCCESS);
    }
    else
    {
        fprintf (stderr, "D: s_connect: %s", mosquitto_connack_string (result));
    }
}

static void
s_message (struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct mosq_config *cfg;
	int i;
	bool res;

	assert(obj);
	cfg = (struct mosq_config *)obj;

		if(message->payloadlen){
			printf("%s ", message->topic);
			fwrite(message->payload, 1, message->payloadlen, stdout);
				printf("\n");
		}else{
				printf("%s (null)\n", message->topic);
		}
		fflush(stdout);
}

static int
s_mqtt_read (zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    fprintf (stderr, "D: s_mqtt_read: ");
    struct mosquitto *mqtt_client = (struct mosquitto*) arg;
    mosquitto_loop_read (mqtt_client, 1);
}

int main () {

    int major, minor, revision;
    int r = mosquitto_lib_version (&major, &minor, &revision);

    printf ("mosquitto: major=%d, minor=%d, revision=%d\n", r, major, minor, revision);

    r = mosquitto_lib_init ();
    assert (r == MOSQ_ERR_SUCCESS);

    mosquitto_t mqtt_client = mosquitto_new (
        "mysub-MVY",
        false,
        NULL
    );
    assert (mqtt_client);

    mosquitto_connect_callback_set (mqtt_client, s_connect);
	mosquitto_message_callback_set (mqtt_client, s_message);

    r = mosquitto_connect_bind (
        mqtt_client,
        "::1",
        1883,
        10,
        "::1");
    assert (r == MOSQ_ERR_SUCCESS);

    /*
    r = mosquitto_loop_forever (
        mqtt_client,
        -1,
        1);
    */

    zloop_t *loop = zloop_new ();
    zmq_pollitem_t it = {NULL, mosquitto_socket (mqtt_client), ZMQ_POLLIN, 0};
    int id = zloop_poller (loop, &it, s_mqtt_read, (void*) mqtt_client);
    r = zloop_start (loop);
    assert (r == 0);

    mosquitto_disconnect (mqtt_client);

    mosquitto_destroy (mqtt_client);
    mqtt_client = NULL;

    r = mosquitto_lib_cleanup ();
    assert (r == MOSQ_ERR_SUCCESS);
}
