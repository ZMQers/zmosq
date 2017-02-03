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
typedef struct mosquitto* mosquitto_t;

struct _zmosq_server_t {
    zsock_t *pipe;              //  Actor command pipe
    zpoller_t *poller;          //  Socket poller
    bool terminated;            //  Did caller ask us to quit?
    bool verbose;               //  Verbose logging enabled?
    
    int port;                   //  network port, default 1883
    const char *host;           //  hostname or ip addr. of the broker
    mosquitto_t client;         //  mosquito instance
    int keepalive;              //  how long should broker wait to send ping, [sec]
    const *char bind_adr;       //  hostname or ip of the local interface
    //  TODO: Declare properties
};


//  --------------------------------------------------------------------------
//  Create a new zmosq_server instance

static zmosq_server_t *
zmosq_server_new (zsock_t *pipe, void *args)
{
    zmosq_server_t *self = (zmosq_server_t *) zmalloc (sizeof (zmosq_server_t));
    assert (self);

    if (self)
    {
        self->client = mosquitto_new ("client", false, NUL);
        if (self->client)
        {
            self->pipe = pipe;
            self->terminated = false;
            self->verbose = false;
            self->poller = zpoller_new (self->pipe, NULL);
            self->port = 1883;
        }
    }
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
        
        zstr_free(&self->host);

        //  Free object itself
        mosquitto_destroy (&self->client);
        self->client = NULL;
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}

void
zmosq_connect (struct mosquitto *mosq, void *obj, int result)
{
    assert (self);
    int rv = mosquitto_subscribe (self->client, NULL, "test", 0);
    assert (rv != MOSQ_ERR_SUCCESS);

}

//  Start this actor. Return a value greater or equal to zero if initialization
//  was successful. Otherwise -1.

static int
zmosq_server_start (zmosq_server_t *self)
{
    mosquitto_connect_callback_set (self->client, zmosq_connect);
 	mosquitto_message_callback_set (self->client, zmosq_message);
    
    rv = mosquitto_connect_bind (self->client,
                                     self->host,
                                     self->port,
                                     self->keepalive,
                                     self->bind_adr);
    if (rv != MOSQ_ERR_SUCCESS)
        return -1;
    else
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

    while (!self->terminated) {
        zsock_t *which = (zsock_t *) zpoller_wait (self->poller, 0);

        if (which == NULL) {
            if (zpoller_terminated(poller) || zsys_interrupted) {
                zsys_info ("server is terminating");
                break;
            }
        }
        
        if (which == self->pipe)
            zmosq_server_recv_api (self);
       //  Add other sockets when you need them.
    }
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
