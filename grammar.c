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
static const char cvsid[] = "$Id: grammar.c,v 1.3 1999/12/13 02:24:31 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include "component.h"
#include "production.h"
#include "grammar.h"
#include "kernel.h"

struct grammar
{
    /* The number of productions */
    int production_count;

    /* The productions */
    production_t *productions;

    /* The number of terminal symbols */
    int terminal_count;

    /* The terminals */
    component_t *terminals;

    /* The number of nonterminal symbols */
    int nonterminal_count;

    /* The nonterminals */
    component_t *nonterminals;

    /* A table of productions listed by their left-hand-side */
    production_t **productions_by_nonterminal;

    /* The number of kernels in the receiver */
    int kernel_count;

    /* The kernels */
    kernel_t *kernels;
};


/* Construct the set of LR(0) kernels */
static void compute_lr0_kernels(grammar_t self)
{
    kernel_t seed = kernel_alloc(self, self -> productions[0]);
    
    /* FIX THIS: not done yet... */
    kernel_get_goto_table(seed);
}

/* Compute the set of LALR(0) states */
static void compute_lalr_states(grammar_t self)
{
    /* Compute the LR0 kernels */
    compute_lr0_kernels(self);

    printf("compute_lalr_states() not yet implemented\n");
    exit(1);
}

/* Allocates and initializes a new nonterminal grammar_t */
grammar_t grammar_alloc()
{
    grammar_t self;

    /* Allocate space for a new grammar_t */
    if ((self = (grammar_t)malloc(sizeof(struct grammar))) == NULL)
    {
	return NULL;
    }

    /* Initialize its contents to sane values */
    self -> production_count = 0;
    self -> productions = NULL;
    self -> terminal_count = 0;
    self -> terminals = NULL;
    self -> nonterminal_count = 0;
    self -> nonterminals = NULL;
    self -> productions_by_nonterminal = NULL;
    return self;
}

/* Releases the resources consumed by the receiver */
void grammar_free(grammar_t self)
{
    int index;

    if (self -> productions != NULL)
    {
	for (index = 0; index < self -> production_count; index++)
	{
	    production_free(self -> productions[index]);
	}

	free(self -> productions);
    }

    free(self);
}

/* Adds another production to the grammar */
void grammar_add_production(grammar_t self, production_t production)
{
    int index;
    production_t *array, *probe;

    /* Add the production to the table of productions */
    self -> productions = (production_t *)realloc(
	self -> productions, (self -> production_count + 1) * sizeof(production_t));
    self -> productions[self -> production_count++] = production;

    /* Add the production to the map of nonterminals to productions */
    index = component_get_index(production_get_nonterminal(production));

    /* Make sure the table is big enough for this entry */
    if (! (index < self -> nonterminal_count))
    {
	self -> productions_by_nonterminal = (production_t **)realloc(
	    self -> productions_by_nonterminal,
	    (index + 1) * sizeof(production_t *));

	/* Null out all of the new entries */
	while (self -> nonterminal_count < index + 1)
	{
	    self -> productions_by_nonterminal[self -> nonterminal_count++] = NULL;
	}
    }

    /* If there is no entry for the nonterminal then create one */
    if ((array = self -> productions_by_nonterminal[index]) == NULL)
    {
	self -> productions_by_nonterminal[index] = (production_t *)malloc(sizeof(production_t) * 2);
	self -> productions_by_nonterminal[index][0] = production;
	self -> productions_by_nonterminal[index][1] = NULL;
	return;
    }

    /* Otherwise count the number of entries */
    for (probe = array; *probe != NULL; probe++);

    /* Allocate a bigger array */
    self -> productions_by_nonterminal[index] = (production_t *)realloc(
	array, sizeof(production_t) * (probe - array + 2));

    /* And install the new production at the end of the array */
    probe = probe - array + self -> productions_by_nonterminal[index];
    *(probe++) = production;
    *probe = NULL;
}

/* Sets the grammar's set of terminals */
void grammar_set_components(
    grammar_t self,
    int terminal_count,
    component_t *terminals,
    int nonterminal_count,
    component_t *nonterminals)
{
    self -> terminal_count = terminal_count;
    self -> terminals = terminals;
    self -> nonterminal_count = nonterminal_count;
    self -> nonterminals = nonterminals;
}

/* Returns the number of components in the grammar */
int grammar_get_component_count(grammar_t self)
{
    return self -> terminal_count + self -> nonterminal_count;
}

/* Construct a single number to represent a production_t and offset */
int grammar_encode(grammar_t self, production_t production, int offset)
{
    return offset * self -> production_count + production_get_index(production);
}

/* Answers the production and offset encoded by the integer */
int grammar_decode(grammar_t self, int code, production_t *production_out)
{
    *production_out = self -> productions[code % self -> production_count];
    return code / self -> production_count;
}

/* Inserts the go-to contribution of the encoded production/offset into the table */
void grammar_compute_goto(grammar_t self, int **table, int code)
{
    fprintf(stderr, "grammar_compute_goto(): not yet implemented\n");
    exit(1);
}

/* Print out the kernels */
void grammar_print_kernels(grammar_t self, FILE *out)
{
    fprintf(out, "{grammar_print_kernels()}\n");
}

/* Pretty-prints the receiver */
void grammar_print(grammar_t self, FILE *out)
{
    int index;

    /* Print out the productions by nonterminal table */
    for (index = 0; index < self -> nonterminal_count; index++)
    {
	production_t *probe;

	fprintf(out, "\n[%d]:\n", index);
	if ((probe = self -> productions_by_nonterminal[index]) != NULL)
	{
	    while (*probe != NULL)
	    {
		fprintf(out, "  ");
		production_print(*(probe++), out);
		fprintf(out, "\n");
	    }
	}
    }
}

