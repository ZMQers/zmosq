#include <mosquitto.h>
#include <czmq.h>

static void
s_connect (struct mosquitto *mosq, void *obj, int result) {
    fprintf (stderr, "D: s_connect, result=%d\n", result);

    if (!result) {
        fprintf (stderr, "D: s_connect, !result\n");
        int r = mosquitto_subscribe (mosq, NULL, "#", 0);
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
    zsock_t *mqtt_writter = (zsock_t*) obj;

    zmsg_t *msg = zmsg_new ();
    zmsg_addstr (msg, message->topic);
    if (message->payload)
        zmsg_addstr (msg, message->payload);
    zmsg_send (&msg, mqtt_writter);
}

static void
s_mosquitto_actor (zsock_t *pipe, void *args)
{
    int r;

    r = mosquitto_lib_init ();
    assert (r == MOSQ_ERR_SUCCESS);

    zuuid_t *uuid = zuuid_new ();
    char *endpoint = zsys_sprintf (" inproc://%s-mqtt", zuuid_str_canonical (uuid));
    endpoint [0] = '@';
    zsock_t *mqtt_writter = zsock_new_pair (endpoint);
    endpoint [0] = '>';
    zsock_t *mqtt_reader  = zsock_new_pair (endpoint);
    zstr_free (&endpoint);

    zpoller_t *poller = zpoller_new (pipe, mqtt_reader, NULL);

    struct mosquitto *mosq = mosquitto_new (
        zuuid_str_canonical (uuid),
        false,
        (void*) mqtt_writter
    );
    assert (mosq);
    zuuid_destroy (&uuid);

    mosquitto_connect_callback_set (mosq, s_connect);
    mosquitto_message_callback_set (mosq, s_message);

    zsock_signal (pipe, 0);

    mosquitto_loop_start (mosq);
    r = mosquitto_connect_bind_async (
        mosq,
        "127.0.0.1",
        1883, 
        10,
        "127.0.0.1");
    assert (r == MOSQ_ERR_SUCCESS);

    while (!zsys_interrupted)
    {
        void *which = zpoller_wait (poller, -1);
        if (which == pipe)
            break;

        if (which == mqtt_reader) {
            zmsg_t *msg = zmsg_recv (mqtt_reader);
            zsys_debug ("=============== s_mosquitto_actor ==========================");
            zmsg_print (msg);
            zmsg_destroy (&msg);
        }
    }

    mosquitto_loop_stop (mosq, true);

    zsock_destroy (&mqtt_reader);
    zsock_destroy (&mqtt_writter);

    r = mosquitto_lib_cleanup ();
    assert (r == MOSQ_ERR_SUCCESS);
}

int main () {
    
    zactor_t *zmosq = zactor_new (s_mosquitto_actor, NULL);

    while (!zsys_interrupted)
        zclock_sleep (500);

    zactor_destroy (&zmosq);
}
