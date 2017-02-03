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

//  Self test of this class
ZMSQ_EXPORT void
    zmosq_client_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
