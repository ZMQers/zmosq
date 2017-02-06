/*  =========================================================================
    zmosq_server - Zmosq actor

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of the Malamute Project.                         
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef ZMOSQ_SERVER_H_INCLUDED
#define ZMOSQ_SERVER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


//  @interface
//  Create new zmosq_server actor instance.
//  @TODO: Describe the purpose of this actor!
//
//      zactor_t *zmosq_server = zactor_new (zmosq_server_actor, NULL);
//
//  Destroy zmosq_server instance.
//
//      zactor_destroy (&zmosq_server);
//
//  Enable verbose logging of commands and activity:
//
//      zstr_send (zmosq_server, "VERBOSE");
//
//  Connect to mosquitto broker
//
//      zstr_sendx (zmosq_server, "MOSQUITTO-CONNECT", "host", "port", "keepalive", "bind_address", NULL);
//
//  Subscribe on MQQT topic (can be repeated, or more topics can be specified here)
//
//      zstr_sendx (zmosq_server, "MOSQUITTO-SUBSCRIBE", "topic", NULL);
//
//  Connect to malamute broker
//
//      zstr_sendx (zmosq_server, "MLM-CONNECT", "endpoint", NULL);
//
//  Publish on stream
//
//      zstr_sendx (zmosq_server, "MLM-STREAM", "stream", NULL);
//
//  Start zmosq_server actor - all broker related commands must be called before!
//
//      zstr_sendx (zmosq_server, "START", NULL);
//
//  Stop zmosq_server actor.
//
//      zstr_sendx (zmosq_server, "STOP", NULL);
//
//
//  This is the zmosq_server constructor as a zactor_fn;
ZMSQ_EXPORT void
    zmosq_server_actor (zsock_t *pipe, void *args);

//  Self test of this actor
ZMSQ_EXPORT void
    zmosq_server_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
