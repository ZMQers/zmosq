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
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
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
