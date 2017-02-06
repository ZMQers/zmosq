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

    //  TODO: Initialize properties
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

        //  TODO: Free actor properties

        //  Free object itself
        zpoller_destroy (&self->poller);
        zuuid_destroy (&self->uuid);
        zsock_destroy (&self->mqtt_writer);
        zsock_destroy (&self->mqtt_reader);
        if (self->mosq)
            mosquitto_destroy (self->mosq);
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
            zsys_error ("Cannot initializa mosquitto library: %s", mosquitto_strerror (r));
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

    return 0;
}


//  Stop this actor. Return a value greater or equal to zero if stopping 
//  was successful. Otherwise -1.

static int
zmosq_server_stop (zmosq_server_t *self)
{
    assert (self);

    //  TODO: Add shutdown actions

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
    if (streq (command, "STOP"))
        zmosq_server_stop (self);
    else
    if (streq (command, "VERBOSE"))
        self->verbose = true;
    else
    if (streq (command, "$TERM"))
        //  The $TERM command is send by zactor_destroy() method
        self->terminated = true;
    else {
        zsys_error ("invalid command '%s'", command);
        assert (false);
    }
    zstr_free (&command);
    zmsg_destroy (&request);
}

static void
s_connect (struct mosquitto *mosq, void *obj, int result) {
    zsys_debug ("D: s_connect, result=%d", result);

    if (!result) {
        zsys_debug ("D: s_connect, !result");
        int r = mosquitto_subscribe (mosq, NULL, "#", 0);
        assert (r == MOSQ_ERR_SUCCESS);
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

    mosquitto_loop_start (self->mosq);
    int r;
    r = mosquitto_connect_bind_async (
        self->mosq,
        "127.0.0.1",
        1883, 
        10,
        "127.0.0.1");
    assert (r == MOSQ_ERR_SUCCESS);

    while (!zsys_interrupted)
    {
        void *which = zpoller_wait (self->poller, -1);
        if (which == pipe)
            zmosq_server_recv_api (self);

        if (which == self->mqtt_reader) {
            zmsg_t *msg = zmsg_recv (self->mqtt_reader);
            zsys_debug ("=============== s_mosquitto_actor ==========================");
            zmsg_print (msg);
            zmsg_destroy (&msg);
        }
    }

    mosquitto_loop_stop (self->mosq, true);

    assert (r == MOSQ_ERR_SUCCESS);
    
    mosquitto_lib_cleanup ();
    zmosq_server_destroy (&self);
}

//  --------------------------------------------------------------------------
//  Self test of this actor.

void
zmosq_server_test (bool verbose)
{
    printf (" * zmosq_server: ");
    //  @selftest
    //  Simple create/destroy test
    zactor_t *zmosq_server = zactor_new (zmosq_server_actor, NULL);
    zstr_sendx (zmosq_server, "START");

    zactor_destroy (&zmosq_server);
    //  @end

    printf ("OK\n");
}
