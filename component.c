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
static const char cvsid[] = "$Id: component.c,v 1.13 2006/10/17 14:28:28 phelps Exp $";
#endif /* lint */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "component.h"

typedef void (*print_func_t)(component_t self, FILE *out);

struct component
{
    /* The name of the file which first referred to this component */
    char *filename;

    /* The line number of the file */
    int line;

    /* Pretty-prints the receiver */
    print_func_t print;

    /* The nonterminal's index */
    int index;

    /* The nonterminal's name */
    char name[1];
};

/* Allocates and initializes a new component_t */
static component_t component_alloc(
    char *filename,
    int line,
    char *name,
    int index)
{
    component_t self;
    int length = strlen(name);

    /* Allocate memory for the component_t */
    if ((self = (component_t)malloc(sizeof(struct component) + length)) == NULL) {
        return NULL;
    }

    /* Initialize its contents to sane values */
    self -> filename = NULL;
    self -> line = line;
    self -> index = index;

    /* Copy the name into place */
    memcpy(self -> name, name, length + 1);

    /* Copy the filename */
    if (filename != NULL) {
        if ((self -> filename = strdup(filename)) == NULL) {
            free(self);
            return NULL;
        }
    }

    return self;
}

/* Releases the resources consumed by the receiver */
void component_free(component_t self)
{
    free(self);
}

/* Returns the filename and line number of the component's first mention */
int component_get_origin(component_t self, char **filename_out)
{
    *filename_out = self -> filename;
    return self -> line;
}

/* Pretty-prints the receiver */
void component_print(component_t self, FILE *out)
{
    self -> print(self, out);
}

/* Prints the receiver as a C enum entry */
void component_print_c_enum(component_t self, FILE *out)
{
    if (self -> index == 0) {
        fprintf(out, "    TT_EOF = 0");
    } else {
        fprintf(out, ",\n    TT_%s", self -> name);
    }
}

/* Prints the receiver as a python assignment statement */
void component_print_python_assign(component_t self, FILE *out)
{
    if (self -> index == 0) {
        fprintf(out, "TT_EOF = 0\n");
    } else {
        fprintf(out, "TT_%s = %d\n", self -> name, self -> index);
    }
}

/* Returns the receiver's name */
char *component_get_name(component_t self)
{
    return self -> name;
}

/* Returns the receiver's index */
int component_get_index(component_t self)
{
    return self -> index;
}



/* Pretty-print function for a nonterminal */
static void nonterminal_print(component_t self, FILE *out)
{
    fprintf(out, "<%s> ", self -> name);
}

/* Pretty-print function for a terminal */
static void terminal_print(component_t self, FILE *out)
{
    fprintf(out, "%s ", self -> name);
}



/* Allocates and initializes a new nonterminal component_t */
component_t nonterminal_alloc(char *filename, int line, char *name, int index)
{
    component_t self;

    /* Do some basic initialization */
    if ((self = component_alloc(filename, line, name, index)) == NULL) {
        return NULL;
    }

    /* Initialize the functions */
    self -> print = nonterminal_print;
    return self;
}

/* Allocates and initializes a new terminal component_t */
component_t terminal_alloc(char *filename, int line, char *name, int index)
{
    component_t self;

    /* Do some basic initialization */
    if ((self = component_alloc(filename, line, name, index)) == NULL) {
        return NULL;
    }

    /* Initialize the functions */
    self -> print = terminal_print;
    return self;
}

/* Returns nonzero if the component is a nonterminal */
int component_is_nonterminal(component_t self)
{
    return self -> print == nonterminal_print;
}
