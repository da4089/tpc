/***************************************************************

  Copyright (C) DSTC Pty Ltd (ACN 052 372 577) 1995.
  Unpublished work.  All Rights Reserved.

  The software contained on this media is the property of the
  DSTC Pty Ltd.  Use of this software is strictly in accordance
  with the license agreement in the accompanying LICENSE.DOC
  file.  If your distribution of this software does not contain
  a LICENSE.DOC file then you have no rights to use this
  software in any manner and should contact DSTC at the address
  below to determine an appropriate licensing arrangement.

     DSTC Pty Ltd
     Level 7, Gehrmann Labs
     University of Queensland
     St Lucia, 4072
     Australia
     Tel: +61 7 3365 4310
     Fax: +61 7 3365 4311
     Email: enquiries@dstc.edu.au

  This software is being provided "AS IS" without warranty of
  any kind.  In no event shall DSTC Pty Ltd be liable for
  damage of any kind arising out of or in connection with
  the use or performance of this software.

****************************************************************/

#ifndef lint
static const char cvsid[] = "$Id: component.c,v 1.6 1999/12/13 08:48:37 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include "component.h"

typedef void (*print_func_t)(component_t self, FILE *out);

struct component
{
    /* Pretty-prints the receiver */
    print_func_t print;

    /* The nonterminal's index */
    int index;

    /* The nonterminal's name */
    char name[1];
};

/* Allocates and initializes a new component_t */
static component_t component_alloc(char *name, int index)
{
    component_t self;
    int length = strlen(name);

    /* Allocate memory for the component_t */
    if ((self = (component_t)malloc(sizeof(struct component) + length)) == NULL)
    {
	return NULL;
    }

    /* Initialize its contents */
    self -> index = index;
    memcpy(&self -> name, name, length + 1);
    return self;
}

/* Releases the resources consumed by the receiver */
void component_free(component_t self)
{
    free(self);
}

/* Pretty-prints the receiver */
void component_print(component_t self, FILE *out)
{
    self -> print(self, out);
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
component_t nonterminal_alloc(char *name, int index)
{
    component_t self;

    /* Do some basic initialization */
    if ((self = component_alloc(name, index)) == NULL)
    {
	return NULL;
    }

    /* Initialize the functions */
    self -> print = nonterminal_print;
    return self;
}

/* Allocates and initializes a new terminal component_t */
component_t terminal_alloc(char *name, int index)
{
    component_t self;

    /* Do some basic initialization */
    if ((self = component_alloc(name, index)) == NULL)
    {
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
