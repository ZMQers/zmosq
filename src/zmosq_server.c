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
        free (self);
        *self_p = NULL;
    }
}


//  Start this actor. Return a value greater or equal to zero if initialization
//  was successful. Otherwise -1.

static int
zmosq_server_start (zmosq_server_t *self)
{
    assert (self);

    //  TODO: Add startup actions

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

static int s_api_reader (
           zloop_t *loop, zsock_t *reader, void *arg)
{
    zmosq_server_t* self = (zmosq_server_t*) arg;
    zmosq_server_recv_api (self);
    return 0;
}

static int
s_mqtt_read (zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    fprintf (stderr, "D: s_mqtt_read: ");
    struct mosquitto *mqtt_client = (struct mosquitto*) arg;
    mosquitto_loop_read (mqtt_client, 1);
    return 0;
}


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
	assert(obj);
		if(message->payloadlen){
			printf("%s ", message->topic);
			fwrite(message->payload, 1, message->payloadlen, stdout);
				printf("\n");
		}else{
				printf("%s (null)\n", message->topic);
		}
		fflush(stdout);
}


//  --------------------------------------------------------------------------
//  This is the actor which runs in its own thread.

void
zmosq_server_actor (zsock_t *pipe, void *args)
{
    zmosq_server_t * self = zmosq_server_new (pipe, args);
    if (!self)
        return;          //  Interrupted


    //  Signal actor successfully initiated
    zsock_signal (self->pipe, 0);

    int r = mosquitto_lib_init ();
    assert (r == MOSQ_ERR_SUCCESS);

    mosquitto_t *mqtt_client = mosquitto_new (
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

    zloop_t *loop = zloop_new ();
    zloop_reader (loop, self->pipe, s_api_reader, (void*) self);
    zmq_pollitem_t it = {NULL, mosquitto_socket (mqtt_client), ZMQ_POLLIN, 0};
    int id = zloop_poller (loop, &it, s_mqtt_read, (void*) mqtt_client);
    printf ("%d\n",id);
    r = zloop_start (loop);
    assert (r == 0);

    
    mosquitto_destroy (mqtt_client);
    mqtt_client = NULL;

    r = mosquitto_lib_cleanup ();
    assert (r == MOSQ_ERR_SUCCESS);


    

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

    zactor_destroy (&zmosq_server);
    //  @end

    printf ("OK\n");
}
