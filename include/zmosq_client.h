/*  =========================================================================
    zmosq_client - Zmosq client

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Malamute Project.                         
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef ZMOSQ_CLIENT_H_INCLUDED
#define ZMOSQ_CLIENT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new zmosq_client
ZMSQ_EXPORT zmosq_client_t *
    zmosq_client_new (void);

//  Destroy the zmosq_client
ZMSQ_EXPORT void
    zmosq_client_destroy (zmosq_client_t **self_p);

//  Start the zmosq_client 
ZMSQ_EXPORT void
    zmosq_client_start (zmosq_client_t *self);

//  Stop the zmosq_client  
ZMSQ_EXPORT void
    zmosq_client_stop (zmosq_client_t *self);

//  Set verbosity   
ZMSQ_EXPORT void
    zmosq_client_set_verbose (zmosq_client_t *self);

//  Is client connected to mosquitto broker?
ZMSQ_EXPORT bool
    zmosq_client_mqtt_connected (zmosq_client_t *self);

//  Connect client to mosquitto broker
ZMSQ_EXPORT void
    zmosq_client_mqtt_connect (zmosq_client_t *self, const char *host, int port, int keepalive, const char *bind_address);

//  Get mosquitto broker's hostname, to which client is connected   
ZMSQ_EXPORT const char *
    zmosq_client_host (zmosq_client_t *self);

//  Get mosquitto broker's port, to which client is connected   
ZMSQ_EXPORT int 
    zmosq_client_port (zmosq_client_t *self);

//  Get mosquitto broker's keepalive, to which client is connected   
ZMSQ_EXPORT int 
    zmosq_client_keepalive (zmosq_client_t *self);

//  Get mosquitto broker's bindaddress, to which client is connected   
ZMSQ_EXPORT const char *
    zmosq_client_bindaddress (zmosq_client_t *self);

//  Subscribe to mosquitto broker's topic  
ZMSQ_EXPORT void
    zmosq_client_subscribe (zmosq_client_t *self, const char *topic);

//  Get mosquitto broker's topics to which client is subscribed
ZMSQ_EXPORT zlistx_t *
    zmosq_client_topics (zmosq_client_t *self);

//  Is client connected to malamute broker  
ZMSQ_EXPORT bool
    zmosq_client_mlm_connected (zmosq_client_t *self);

//  Connect client to malamute broker
ZMSQ_EXPORT void
    zmosq_client_mlm_connect (zmosq_client_t *self, const char *mlm_endpoint);

//  Get malamute broker's hostname to which client is connected
ZMSQ_EXPORT const char *
    zmosq_client_mlm_host (zmosq_client_t *self);

//  Publish to malamute broker's stream
ZMSQ_EXPORT void
    zmosq_client_mlm_set_stream (zmosq_client_t *self, const char *stream);

//  Get malamute broker's stream to which client is publishing 
ZMSQ_EXPORT const char *
    zmosq_client_mlm_stream (zmosq_client_t *self);

//  Self test of this class
ZMSQ_EXPORT void
    zmosq_client_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
