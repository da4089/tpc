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
static const char cvsid[] = "$Id: grammar.c,v 1.2 1999/12/11 18:00:44 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include <component.h>
#include <production.h>
#include <grammar.h>

struct grammar
{
    /* The number of productions */
    int production_count;

    /* The productions */
    production_t *productions;

    /* The number of terminal symbols */
    int terminal_count;

    /* The terminal symbols */
    component_t *terminals;

    /* The number of nonterminal symbols */
    int nonterminal_count;

    /* The nonterminals */
    component_t *nonterminals;
};


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
    self -> productions = (production_t *)realloc(
	self -> productions, (self -> production_count + 1) * sizeof(production_t));
    self -> productions[self -> production_count++] = production;
}

/* Sets the grammar's set of terminals */
void grammar_set_terminals(grammar_t self, int count, component_t *terminals)
{
    self -> terminal_count = count;
    self -> terminals = terminals;
}

/* Sets the grammar's set of nonterminals */
void grammar_set_nonterminals(grammar_t self, int count, component_t *nonterminals)
{
    self -> nonterminal_count = count;
    self -> nonterminals = nonterminals;
}

/* Pretty-prints the receiver */
void grammar_print(grammar_t self, FILE *out)
{
    fprintf(out, "{grammar}\n");
}
