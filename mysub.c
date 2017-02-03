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
        false,
        "::1");
    assert (r == MOSQ_ERR_SUCCESS);

    r = mosquitto_loop_forever (
        mqtt_client,
        -1,
        1);

    mosquitto_destroy (mqtt_client);
    mqtt_client = NULL;

    r = mosquitto_lib_cleanup ();
    assert (r == MOSQ_ERR_SUCCESS);
}
