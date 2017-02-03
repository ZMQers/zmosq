/*  =========================================================================
    zmosq_client - Zmosq client

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Malamute Project.                         
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    zmosq_client - Zmosq client
@discuss
@end
*/

#include "zmsq_classes.h"

//  Structure of our class

struct _zmosq_client_t {
    zactor_t *zmosq_server;
    bool mqtt_connected;
    char *mqtt_host;
    char *mqtt_bindaddress;
    int mqtt_port;
    int mqtt_keepalive;
    zlistx_t *topics;
    bool mlm_connected; 
    char *mlm_host;
    char *mlm_stream;
};


//  --------------------------------------------------------------------------
//  Create a new zmosq_client

zmosq_client_t *
zmosq_client_new (void)
{
    zmosq_client_t *self = (zmosq_client_t *) zmalloc (sizeof (zmosq_client_t));
    assert (self);
    //  Initialize class properties here
    self->zmosq_server = zactor_new (zmosq_server_actor, NULL);
    self->mqtt_connected = false;
    self->mqtt_host = strdup ("");
    self->mqtt_bindaddress = strdup ("");
    self->mqtt_port = -1;
    self->mqtt_keepalive = -1;
    self->topics = zlistx_new ();
    zlistx_set_destructor (self->topics, (czmq_destructor *) zstr_free);
    self->mlm_connected = false;
    self->mlm_host = strdup ("");
    self->mlm_stream = strdup ("");
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the zmosq_client

void
zmosq_client_destroy (zmosq_client_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zmosq_client_t *self = *self_p;
        //  Free class properties here
        zactor_destroy (&self->zmosq_server);
        zstr_free (&self->mqtt_host);
        zstr_free (&self->mqtt_bindaddress);
        zlistx_destroy (&self->topics);
        zstr_free (&self->mlm_host);
        zstr_free (&self->mlm_stream);
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  --------------------------------------------------------------------------
//  Start the zmosq_client

void
zmosq_client_start (zmosq_client_t *self)
{
    assert (self);
    zstr_sendx (self->zmosq_server, "START", NULL);
}


//  --------------------------------------------------------------------------
//  Stop the zmosq_client

void
zmosq_client_stop (zmosq_client_t *self)
{
    assert (self);
    zstr_sendx (self->zmosq_server, "STOP", NULL);
}


//  --------------------------------------------------------------------------
//  Set verbosity

void
zmosq_client_set_verbose (zmosq_client_t *self)
{
    assert (self);
    zstr_send (self->zmosq_server, "VERBOSE");
}


//  --------------------------------------------------------------------------
//  Is client connected to mosquitto broker?

bool
zmosq_client_mqtt_connected (zmosq_client_t *self)
{
    assert (self);
    return self->mqtt_connected;
}


//  --------------------------------------------------------------------------
//  Connect client to mosquitto broker

void
zmosq_client_mqtt_connect (zmosq_client_t *self, const char *host, int port, int keepalive, const char *bind_address)
{
    assert (self);
    assert (host);
    assert (bind_address);
    zstr_sendx (self->zmosq_server, "MOSQUITTO-CONNECT", host, port, keepalive, bind_address, NULL);
    // host
    zstr_free (&self->mqtt_host);
    self->mqtt_host = strdup (host);
    // port
    self->mqtt_port = port;
    // keepalive
    self->mqtt_keepalive = keepalive;
    // bindaddress
    zstr_free (&self->mqtt_bindaddress);
    self->mqtt_bindaddress = strdup (bind_address);
    // connected
    self->mqtt_connected = true;
}


//  --------------------------------------------------------------------------
//  Get mosquitto broker's hostname, to which client is connected

const char *
zmosq_client_host (zmosq_client_t *self)
{
    assert (self);
    return self->mqtt_host;
}


//  --------------------------------------------------------------------------
//  Get mosquitto broker's port, to which client is connected

int
zmosq_client_port (zmosq_client_t *self)
{
    assert (self);
    return self->mqtt_port;
}


//  --------------------------------------------------------------------------
//  Get mosquitto broker's keepalive, to which client is connected

int
zmosq_client_keepalive (zmosq_client_t *self)
{
    assert (self);
    return self->mqtt_keepalive;
}


//  --------------------------------------------------------------------------
//  Get mosquitto broker's bindaddress, to which client is connected   

const char *
zmosq_client_bindaddress (zmosq_client_t *self)
{
    assert (self);
    return self->mqtt_bindaddress;
}


//  --------------------------------------------------------------------------
//  Subscribe to mosquitto broker's topic

void
zmosq_client_subscribe (zmosq_client_t *self, const char *topic)
{
    assert (self);
    assert (topic);
    zlistx_add_end (self->topics, (void *) topic);
    zstr_sendx (self->zmosq_server, "MOSQUITTO-SUBSCRIBE", topic, NULL);
}


//  --------------------------------------------------------------------------
//  Get mosquitto broker's topics to which client is subscribed

zlistx_t *
zmosq_client_topics (zmosq_client_t *self)
{
    assert (self);
    return zlistx_dup (self->topics);
}


//  --------------------------------------------------------------------------
//  Is client connected to malamute broker

bool
zmosq_client_mlm_connected (zmosq_client_t *self)
{
    assert (self);
    return self->mlm_connected;
}


//  --------------------------------------------------------------------------
//  Connect client to malamute broker

void
zmosq_client_mlm_connect (zmosq_client_t *self, const char *mlm_endpoint)
{
    assert (self);
    assert (mlm_endpoint);
    zstr_sendx (self->zmosq_server, "MLM-CONNECT", mlm_endpoint, NULL);
    zstr_free (&self->mlm_host);
    self->mlm_host = strdup (mlm_endpoint);
    self->mlm_connected = true;
}


//  --------------------------------------------------------------------------
//  Get malamute broker's hostname to which client is connected

const char *
zmosq_client_mlm_host (zmosq_client_t *self)
{
    assert (self);
    return self->mlm_host;
}


//  --------------------------------------------------------------------------
//  Publish to malamute broker's stream

void
zmosq_client_mlm_set_stream (zmosq_client_t *self, const char *stream)
{
    assert (self);
    assert (stream);

    zstr_sendx (self->zmosq_server, "MLM-PUBLISH", "stream", NULL);
    zstr_free (&self->mlm_stream);
    self->mlm_stream = strdup (stream);
}


//  --------------------------------------------------------------------------
//  Get malamute broker's stream to which client is publishing

const char *
zmosq_client_mlm_stream (zmosq_client_t *self)
{
    assert (self);
    return self->mlm_stream;
}


//  --------------------------------------------------------------------------
//  Self test of this class

void
zmosq_client_test (bool verbose)
{
    printf (" * zmosq_client: ");

    //  @selftest
    //  Simple create/destroy test
    zmosq_client_t *self = zmosq_client_new ();
    assert (self);
    zmosq_client_destroy (&self);
    //  @end
    printf ("OK\n");
}
