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
static const char cvsid[] = "$Id: production.c,v 1.10 2000/03/15 05:42:15 phelps Exp $";
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
production_t production_alloc(
    int index,
    component_t nonterminal,
    int component_count,
    component_t *components,
    char *reduction)
{
    production_t self;

    /* Allocate memory for the new production_t */
    if ((self = (production_t)malloc(sizeof(struct production))) == NULL)
    {
	return NULL;
    }

    /* Initialize its contents to sane values */
    self -> index = index;
    self -> nonterminal = nonterminal;
    self -> count = component_count;
    self -> components = components;
    self -> reduction = reduction;
    return self;
}

/* Releases the resources consumed by the receiver */
void production_free(production_t self)
{
    int index;

    if (self -> nonterminal != NULL)
    {
	component_free(self -> nonterminal);
    }

    if (self -> components != NULL)
    {
	for (index = 0; index < self -> count; index++)
	{
	    component_free(self -> components[index]);
	}

	free(self -> components);
    }

    if (self -> reduction != NULL)
    {
	free(self -> reduction);
    }

    free(self);
}

/* Returns the production's index */
int production_get_index(production_t self)
{
    return self -> index;
}

/* Returns the nonterminal index of the production's left-hand side */
int production_get_nonterminal_index(production_t self)
{
    return component_get_index(self -> nonterminal);
}

/* Returns the nth component of the production's right-hand-side */
component_t production_get_component(production_t self, int index)
{
    if (index < self -> count)
    {
	return self -> components[index];
    }

    return NULL;
}

/* Pretty-prints the receiver */
void production_print(production_t self, FILE *out)
{
    production_print_with_offset(self, out, -1);
}

/* Pretty-prints the receiver with a `*' after the nth element */
void production_print_with_offset(production_t self, FILE *out, int offset)
{
    int index;

    /* Print the left-hand-side */
    component_print(self -> nonterminal, out);

    /* Print the `derives' operator */
    fprintf(out, "::= ");

    /* Print the right-hand-side */
    for (index = 0; index < self -> count; index++)
    {
	if (index == offset)
	{
	    fprintf(out, "* ");
	}

	component_print(self -> components[index], out);
    }

    /* Watch for a final `*' */
    if (self -> count == offset)
    {
	fprintf(out, "* ");
    }
}

/* Prints the production as a struct */
void production_print_c_struct(production_t self, FILE *out)
{
    fprintf(out, "    { %s, %d, %d }",
	    self -> reduction,
	    component_get_index(self -> nonterminal),
	    self -> count);
}

/* Prints the production as a python tuple */
void production_print_python_tuple(production_t self, char *module, FILE *out)
{
    if (module == NULL)
    {
	fprintf(out, "    (%s, %d, %d)",
		self -> reduction,
		component_get_index(self -> nonterminal),
		self -> count);
    }
    else
    {
	fprintf(out, "    (%s.%s, %d, %d)",
		module,
		self -> reduction,
		component_get_index(self -> nonterminal),
		self -> count);
    }
}
