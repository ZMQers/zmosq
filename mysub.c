#include "../lib/mosquitto.h"
#include <czmq.h>

typedef struct mosquitto* mosquitto_t;

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

    mosquitto_destroy (mqtt_client);
    mqtt_client = NULL;

    r = mosquitto_lib_cleanup ();
    assert (r == MOSQ_ERR_SUCCESS);
}
