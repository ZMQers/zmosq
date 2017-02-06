/*  =========================================================================
    zmosq_server - Zmosq actor

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Malamute Project.                         
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    zmosq_server - Zmosq actor
@discuss
@end
*/

#include "zmsq_classes.h"

//  Structure of our actor
typedef struct mosquitto mosquitto_t;

struct _zmosq_server_t {
    zsock_t *pipe;              //  Actor command pipe
    zsock_t *mqtt_reader;
    zsock_t *mqtt_writer;
    zuuid_t *uuid;
    mosquitto_t *mosq;
    char *host;                 // mosquitto_connect_bind_async
    int port;                   //
    int keepalive;              //
    char *bind_address;         //
    zlist_t *topics;            //  MQQT topics to subscribe to
    zpoller_t *poller;          //  Socket poller
    bool terminated;            //  Did caller ask us to quit?
    bool verbose;               //  Verbose logging enabled?
    //  TODO: Declare properties
};


//  --------------------------------------------------------------------------
//  Create a new zmosq_server instance

static zmosq_server_t *
zmosq_server_new (zsock_t *pipe, void *args)
{
    zmosq_server_t *self = (zmosq_server_t *) zmalloc (sizeof (zmosq_server_t));
    assert (self);

    self->pipe = pipe;
    self->terminated = false;
    self->poller = zpoller_new (self->pipe, NULL);

    self->uuid = zuuid_new ();
    char *endpoint = zsys_sprintf (" inproc://%s-mqtt", zuuid_str_canonical (self->uuid));

    endpoint [0] = '@';
    self->mqtt_writer = zsock_new_pair (endpoint);
    endpoint [0] = '>';
    self->mqtt_reader  = zsock_new_pair (endpoint);
    zpoller_add (self->poller, self->mqtt_reader);

    zstr_free (&endpoint);

    self->mosq = mosquitto_new (
        zuuid_str_canonical (self->uuid),
        false,
        self);

    self->topics = zlist_new ();
    zlist_autofree (self->topics);

    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the zmosq_server instance

static void
zmosq_server_destroy (zmosq_server_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zmosq_server_t *self = *self_p;

        zpoller_destroy (&self->poller);
        zuuid_destroy (&self->uuid);
        zsock_destroy (&self->mqtt_writer);
        zsock_destroy (&self->mqtt_reader);
        if (self->mosq)
            mosquitto_destroy (self->mosq);
        zstr_free (&self->host);
        zstr_free (&self->bind_address);
        zlist_destroy (&self->topics);
        free (self);
        *self_p = NULL;
    }
}

static void
s_mosquitto_init ()
{
    static bool initialized = false;
    if (!initialized) {
        int r = mosquitto_lib_init ();
        if (r != MOSQ_ERR_SUCCESS) {
            zsys_error ("Cannot initialize mosquitto library: %s", mosquitto_strerror (r));
            exit (EXIT_FAILURE);
        }
        initialized = true;
    }
}

//  Start this actor. Return a value greater or equal to zero if initialization
//  was successful. Otherwise -1.

static int
zmosq_server_start (zmosq_server_t *self)
{
    assert (self);
    assert (self->mosq);

    mosquitto_loop_start (self->mosq);
    int r;
    r = mosquitto_connect_bind_async (
        self->mosq,
        self->host,
        self->port,
        self->keepalive,
        self->bind_address);

    if (r != MOSQ_ERR_SUCCESS) {
        zsys_error ("Can't connect to mosquito endpoint, run START again");
        mosquitto_loop_stop (self->mosq, true);
    }

    return 0;
}


//  Stop this actor. Return a value greater or equal to zero if stopping 
//  was successful. Otherwise -1.

static int
zmosq_server_stop (zmosq_server_t *self)
{
    assert (self);
    assert (self->mosq);

    //  TODO: Add shutdown actions
    mosquitto_loop_stop (self->mosq, true);
    mosquitto_disconnect (self->mosq);

    return 0;
}

//  Here we handle incoming message from the node
static void
zmosq_server_recv_api (zmosq_server_t *self)
{
    //  Get the whole message of the pipe in one go
    zmsg_t *request = zmsg_recv (self->pipe);
    if (!request)
       return;        //  Interrupted

    char *command = zmsg_popstr (request);
    if (streq (command, "START"))
        zmosq_server_start (self);
    else
    if (streq (command, "STOP")) {
        zmosq_server_stop (self);
    }
    else
    if (streq (command, "VERBOSE"))
        self->verbose = true;
    else
    if (streq (command, "MOSQUITTO-CONNECT")) {
        self->host = zmsg_popstr (request);
        assert (self->host);

        char *foo = zmsg_popstr (request);
        self->port = atoi (foo);
        zstr_free (&foo);

        foo = zmsg_popstr (request);
        self->keepalive = atoi (foo);
        if (self->keepalive <= 3)
            self->keepalive = 3;
        zstr_free (&foo);

        self->bind_address = zmsg_popstr (request);
        if (!self->bind_address)
            self->bind_address = strdup (self->host);
    }
    else
    if (streq (command, "MOSQUITTO-SUBSCRIBE")) {
        char *topic = zmsg_popstr (request);
        while (topic) {
            zlist_append (self->topics, topic);
            zstr_free (&topic);
        }
    }
    else
    if (streq (command, "$TERM")) {
        //  The $TERM command is send by zactor_destroy() method
        self->terminated = true;
        zmosq_server_stop (self);
    }
    else {
        zsys_error ("invalid command '%s'", command);
        assert (false);
    }
    zstr_free (&command);
    zmsg_destroy (&request);
}

static void
s_connect (struct mosquitto *mosq, void *obj, int result) {

    assert (obj);
    zmosq_server_t *self = (zmosq_server_t*) obj;

    if (!result) {
        char *topic = (char*) zlist_first (self->topics);
        while (topic) {
            int r = mosquitto_subscribe (mosq, NULL, "#", 0);
            assert (r == MOSQ_ERR_SUCCESS);
            topic = (char*) zlist_next (self->topics);
        }
    }
}

static void
s_message (struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	assert(obj);

    zmosq_server_t *self = (zmosq_server_t*) obj;
    assert (self);
    zsock_t *mqtt_writer = self->mqtt_writer;
    assert (mqtt_writer);

    zmsg_t *msg = zmsg_new ();
    zmsg_addstr (msg, message->topic);
    if (message->payload)
        zmsg_addmem (msg, message->payload, message->payloadlen);
    zmsg_send (&msg, mqtt_writer);
}

//  --------------------------------------------------------------------------
//  This is the actor which runs in its own thread.

void
zmosq_server_actor (zsock_t *pipe, void *args)
{
    s_mosquitto_init ();

    zmosq_server_t * self = zmosq_server_new (pipe, args);
    if (!self)
        return;          //  Interrupted

    //  Signal actor successfully initiated
    zsock_signal (self->pipe, 0);

    mosquitto_connect_callback_set (self->mosq, s_connect);
	mosquitto_message_callback_set (self->mosq, s_message);


    while (!self->terminated)
    {
        void *which = zpoller_wait (self->poller, -1);
        if (which == pipe)
            zmosq_server_recv_api (self);

        if (which == self->mqtt_reader) {
            zmsg_t *msg = zmsg_recv (self->mqtt_reader);
            zmsg_send (&msg, pipe);
        }
    }

    mosquitto_lib_cleanup ();
    zmosq_server_destroy (&self);
}

//  --------------------------------------------------------------------------
//  Self test of this actor.

void
zmosq_server_test (bool verbose)
{
    printf (" * zmosq_server: ");
    fflush (stdout);

    //  @selftest
    //  Simple create/destroy test
    zactor_t *zmosq_server = zactor_new (zmosq_server_actor, NULL);
    zstr_sendx (zmosq_server, "MOSQUITTO-CONNECT", "127.0.0.1", "1883", "10", "127.0.0.1", NULL);
    zstr_sendx (zmosq_server, "MOSQUITTO-SUBSCRIBE", "TEST", "TEST2", NULL);
    zstr_sendx (zmosq_server, "START", NULL);

    mosquitto_t *client = mosquitto_new (
        "zmosq_server_test_client",
        false,
        NULL
        );
    assert (client);

    int r = mosquitto_connect_bind (
        client,
        "127.0.0.1",
        1883,
        10,
        "127.0.0.1");
    assert (r == MOSQ_ERR_SUCCESS);

    for (int i =0; i != 20; i++) {
        mosquitto_loop (client, 500, 1);
        r = mosquitto_publish (
            client,
            NULL,
            "TOPIC",
            6,
            "HELLO\0",
            0,
            false);
        assert (r == MOSQ_ERR_SUCCESS);
        mosquitto_loop (client, 500, 1);
    }

    zstr_sendx (zmosq_server, "START", NULL);
    // check at least 7 messages
    for (int i =0; i != 7; i++) {
        zmsg_t *msg = zmsg_recv (zmosq_server);
        char *topic, *body;
        topic = zmsg_popstr (msg);
        body = zmsg_popstr (msg);
        assert (streq (topic, "TOPIC"));
        assert (streq (body, "HELLO"));
        zstr_free (&topic);
        zstr_free (&body);
        zmsg_destroy (&msg);
    }

    mosquitto_disconnect (client);
    mosquitto_destroy (client);

    zpoller_t *poller = zpoller_new (zmosq_server, NULL);
    while (true) {
        void *which = zpoller_wait (poller, 100);
        if (!which)
            break;
        zmsg_t *msg = zmsg_recv (zmosq_server);
        zmsg_destroy (&msg);
    }
    zactor_destroy (&zmosq_server);
    //  @end

    printf ("OK\n");
}
