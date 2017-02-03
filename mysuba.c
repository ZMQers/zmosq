#include <mosquitto.h>
#include <czmq.h>

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
    zsys_debug ("s_message");
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

    int r;

    zsys_init ();

    r = mosquitto_lib_init ();
    assert (r == MOSQ_ERR_SUCCESS);

    zuuid_t *uuid = zuuid_new ();

    struct mosquitto *mosq = mosquitto_new (
        zuuid_str_canonical (uuid),
        false,
        NULL
    );
    assert (mosq);
    zuuid_destroy (&uuid);

    mosquitto_connect_callback_set (mosq, s_connect);
    mosquitto_message_callback_set (mosq, s_message);

    mosquitto_loop_start (mosq);

    r = mosquitto_connect_bind_async (
        mosq,
        "::1",
        1883,
        10,
        "::1");
    assert (r == MOSQ_ERR_SUCCESS);
    
    mosquitto_subscribe (mosq, NULL, "#", 0);

    while (!zsys_interrupted)
        zclock_sleep (500);

    mosquitto_loop_stop (mosq, true);

    r = mosquitto_lib_cleanup ();
    assert (r == MOSQ_ERR_SUCCESS);
}
