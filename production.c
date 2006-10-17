/* -*- mode: c; c-file-style: "elvin" -*- */
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

#ifndef lint
static const char cvsid[] = "$Id: production.c,v 1.13 2006/10/17 15:01:04 phelps Exp $";
#endif /* lint */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "component.h"
#include "production.h"

struct production
{
    /* The production's index */
    int index;

    /* The production's nonterminal (left-hand-side) */
    component_t nonterminal;

    /* The number of components on the right-hand-side */
    int count;

    /* The components */
    component_t *components;

    /* The production's reduction */
    char *reduction;
};

/* Allocates and initializes a new production_t */
production_t
production_alloc(int index,
                 component_t nonterminal,
                 int component_count,
                 component_t *components,
                 char *reduction)
{
    production_t self;

    /* Allocate memory for the new production_t */
    if ((self = (production_t)malloc(sizeof(struct production))) == NULL) {
        return NULL;
    }

    /* Initialize its contents to sane values */
    self->index = index;
    self->nonterminal = nonterminal;
    self->count = component_count;
    self->components = components;
    self->reduction = reduction;
    return self;
}

/* Releases the resources consumed by the receiver */
void
production_free(production_t self)
{
    if (self->components != NULL) {
        free(self->components);
    }

    free(self);
}

/* Returns the production's index */
int
production_get_index(production_t self)
{
    return self->index;
}

/* Returns the nonterminal index of the production's left-hand side */
int
production_get_nonterminal_index(production_t self)
{
    return component_get_index(self->nonterminal);
}

/* Returns the nth component of the production's right-hand-side */
component_t
production_get_component(production_t self, int index)
{
    if (index < self->count) {
        return self->components[index];
    }

    return NULL;
}

/* Pretty-prints the receiver */
void
production_print(production_t self, FILE *out)
{
    production_print_with_offset(self, out, -1);
}

/* Pretty-prints the receiver with a `*' after the nth element */
void
production_print_with_offset(production_t self, FILE *out, int offset)
{
    int index;

    /* Print the left-hand-side */
    component_print(self->nonterminal, out);

    /* Print the `derives' operator */
    fprintf(out, "::= ");

    /* Print the right-hand-side */
    for (index = 0; index < self->count; index++) {
        if (index == offset) {
            fprintf(out, "* ");
        }

        component_print(self->components[index], out);
    }

    /* Watch for a final `*' */
    if (self->count == offset) {
        fprintf(out, "* ");
    }
}

/* Prints the production as a struct */
void
production_print_c_struct(production_t self, FILE *out)
{
    fprintf(out, "    { %s, %d, %d }",
            self->reduction,
            component_get_index(self->nonterminal),
            self->count);
}

/* Prints the production as a python tuple */
void
production_print_python_tuple(production_t self, char *module, FILE *out)
{
    if (module == NULL) {
        fprintf(out, "    (%s, %d, %d)",
                self->reduction,
                component_get_index(self->nonterminal),
                self->count);
    } else {
        fprintf(out, "    (%s.%s, %d, %d)",
                module,
                self->reduction,
                component_get_index(self->nonterminal),
                self->count);
    }
}
