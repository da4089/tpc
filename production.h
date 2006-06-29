/***********************************************************************

  Copyright (C) 1999-2006 by Mantara Software (ABN 17 105 665 594).
  All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above
     copyright notice, this list of conditions and the following
     disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials
     provided with the distribution.

   * Neither the name of the Mantara Software nor the names
     of its contributors may be used to endorse or promote
     products derived from this software without specific prior
     written permission. 

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

***********************************************************************/

#ifndef PRODUCTION_H
#define PRODUCTION_H

#ifndef lint
static const char cvs_PRODUCTION_H[] = "$Id: production.h,v 1.8 2006/06/29 10:22:00 phelps Exp $";
#endif /* lint */

/* The production type */
typedef struct production *production_t;

/* Allocates and initializes a new production_t */
production_t production_alloc(
    int index,
    component_t nonterminal,
    int component_count,
    component_t *components,
    char *reduction);

/* Releases the resources consumed by the receiver */
void production_free(production_t self);


/* Returns the nonterminal index of the production's left-hand side */
int production_get_nonterminal_index(production_t self);

/* Returns the production's index */
int production_get_index(production_t self);

/* Returns the nth component of the production's right-hand-side */
component_t production_get_component(production_t self, int index);

/* Pretty-prints the receiver */
void production_print(production_t self, FILE *out);

/* Pretty-prints the receiver with a `*' after the nth element */
void production_print_with_offset(production_t self, FILE *out, int offset);

/* Prints the production as a C struct */
void production_print_c_struct(production_t self, FILE *out);

/* Prints the production as a python tuple */
void production_print_python_tuple(production_t self, char *module, FILE *out);

#endif /* PRODUCTION_H */
