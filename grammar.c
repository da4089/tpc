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
static const char cvsid[] = "$Id: grammar.c,v 1.9 1999/12/14 02:49:47 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    /* The generates table.  Once initialized, generates[generator][generated]
     * will be nonzero if the nonterminal[generator] can generate
     * nonterminal[generated]. */
    char **generates;

    /* The number of kernels in the receiver */
    int kernel_count;

    /* The kernels */
    kernel_t *kernels;
};



/* Constructs a table mapping nonterminal indices to a null-terminated
 * array of productions.  Returns NULL if something goes wrong. */
static production_t **compute_productions_by_nonterminal(
    int nonterminal_count,
    int production_count,
    production_t *productions)
{
    production_t **table;
    production_t *productions_end = productions + production_count;
    production_t *production;

    /* Allocate memory for a new table */
    table = (production_t **)calloc(nonterminal_count, sizeof(production_t *));
    if (table == NULL)
    {
	return NULL;
    }

    /* Popluate it */
    for (production = productions; production < productions_end; production++)
    {
	int index = production_get_nonterminal_index(*production);
	production_t *array;
	int count = 0;

	/* If there is no entry for the nonterminal then create one */
	if ((array = table[index]) == NULL)
	{
	    if ((array = (production_t *)malloc(2 * sizeof(production_t))) == NULL)
	    {
		/* FIX THIS: memory leak */
		free(table);
		return NULL;
	    }

	    count = 0;
	}
	else
	{
	    /* Count the number of entries so far */
	    for (count = 0; array[count] != NULL; count++);

	    /* Enlarge the array */
	    if ((array = (production_t *)realloc(array, (count + 2) * sizeof(production_t))) == NULL)
	    {
		/* FIX THIS: memory leak */
		free(table);
		return NULL;
	    }
	}

	array[count] = *production;
	array[count + 1] = NULL;
	table[index] = array;
    }

    return table;
}

/* Verifies that each nonterminal has at least on production rule */
static int verify_productions_by_nonterminal(grammar_t self)
{
    int index;
    int result = 0;

    /* Do a sanity check -- there should be no empty entries */
    for (index = 0; index < self -> nonterminal_count; index++)
    {
	if (self -> productions_by_nonterminal[index] == NULL)
	{
	    fprintf(stderr, "error: no production for nonterminal ");
	    component_print(self -> nonterminals[index], stderr);
	    fprintf(stderr, "\n");
	    result = -1;
	}
    }

    return result;
}


/* Indicate that the nonterminal `generator' spontaneously generates
 * the nonterminal `generated' in the grammar */
static void mark_generates(grammar_t self, int generator, int generated)
{
    int index;

    /* Make sure the table entry exists */
    if (self -> generates[generator] == NULL)
    {
	self -> generates[generator] = (char *)calloc(self -> nonterminal_count, sizeof(char));
    }
    else if (self -> generates[generator][generated] != 0)
    {
	return;
    }

    /* Set the flag */
    self -> generates[generator][generated] = 1;

    /* Propagate the flag to any nonterminal which generates us */
    for (index = 0; index < self -> nonterminal_count; index++)
    {
	if (self -> generates[index] != NULL && self -> generates[index][generator] != 0)
	{
	    mark_generates(self, index, generated);
	}
    }

    /* Propagate the flags to the nonterminal we've just generated */
    if (self -> generates[generated] != NULL)
    {
	for (index = 0; index < self -> nonterminal_count; index++)
	{
	    if (self -> generates[generated][index] != 0)
	    {
		mark_generates(self, generator, index);
	    }
	}
    }
}

/* Constructs the `generates' table */
static void compute_generates(grammar_t self)
{
    int index;

    /* Create the `generates' table */
    self -> generates = (char **)calloc(self -> nonterminal_count, sizeof(char *));

    /* Go through each of the productions and figure out which things generate which */
    for (index = 0; index < self -> production_count; index++)
    {
	production_t production = self -> productions[index];
	int nonterminal = production_get_nonterminal_index(production);
	component_t component = production_get_component(production, 0);

	if (component_is_nonterminal(component))
	{
	    mark_generates(self, nonterminal, component_get_index(component));
	}
    }
}

/* Encode a production number and offset in a single integer. */
static int encode(grammar_t self, int index, int offset)
{
    return self -> production_count * (offset + 1) - index - 1;
}

/* Decodes an integer into an production number and offset */
static int decode(grammar_t self, int code, int *index_out)
{
    *index_out = self -> production_count - (code % self -> production_count) - 1;
    return code / self -> production_count;
}


/* Locates or creates a kernel for the given goto pairs and returns its index*/
int intern_kernel(grammar_t self, int count, int *pairs)
{
    int index;
    kernel_t kernel;

    /* If there are no pairs then don't do anything special */
    if (count == 0)
    {
	return -1;
    }

    /* See if we've already got a matching kernel */
    for (index = 0; index < self -> kernel_count; index++)
    {
	if (kernel_matches(self -> kernels[index], count, pairs))
	{
	    free(pairs);
	    return index;
	}
    }

    /* Not there, so create a new kernel */
    kernel = kernel_alloc(count, pairs);

    /* And put it in the table */
    self -> kernels = (kernel_t *)realloc(
	self -> kernels, (self -> kernel_count + 1) * sizeof(kernel_t));
    self -> kernels[self -> kernel_count] = kernel;

    /* Return its index */
    return self -> kernel_count++;
}

/* Adds an entry to the pairs table */
static void add_pairs_entry(int *counts, int **table, int index, int pair)
{
    int i;

    /* Figure out where to add the entry (common case is at the end so
     * we count backwards) */
    for (i = counts[index]; i > 0; i--)
    {
	/* If it's already there then bail out now */
	if (pair == table[index][i - 1])
	{
	    return;
	}

	/* Is this where we should insert? */
	if (pair < table[index][i - 1])
	{
	    break;
	}
    }

    /* Expand the table */
    table[index] = (int *)realloc(table[index], (counts[index] + 1) * sizeof(int));

    /* See if we need to make a hole */
    if (i != counts[index])
    {
	memmove(table[index] + i + 1, table[index] + i, sizeof(int) * (counts[index] - i));
    }

    /* Insert the pair */
    table[index][i] = pair;
    counts[index]++;
}

/* Returns the index to use for the given component */
int component_index(grammar_t self, component_t component)
{
    if (component_is_nonterminal(component))
    {
	return component_get_index(component);
    }

    return self -> nonterminal_count + component_get_index(component);
}

/* Computes the pairs table for the given kernel */
void compute_pairs(grammar_t self, kernel_t kernel, int *counts, int **table)
{
    int count;
    int *pairs;
    int index;

    /* Get the kernel pairs */
    count = kernel_get_pairs(kernel, &pairs);

    /* Add an entry for each kernel item */
    for (index = 0; index < count; index++)
    {
	int production_index;
	int offset = decode(self, pairs[index], &production_index);
	production_t production = self -> productions[production_index];
	component_t component = production_get_component(production, offset);

	/* Insert an entry for the component into the table */
	if (component != NULL)
	{
	    add_pairs_entry(
		counts, table,
		component_index(self, component),
		encode(self, production_index, offset + 1));
	}
    }

    /* Compute the closure of each kernel item using the generates table */
    for (index = 0; index < count; index++)
    {
	int production_index;
	int offset = decode(self, pairs[index], &production_index);
	production_t production = self -> productions[production_index];
	component_t component = production_get_component(production, offset);

	/* Look up the next component */
	if (component != NULL && component_is_nonterminal(component))
	{
	    int ci = component_get_index(component);
	    char *generates = self -> generates[ci];
	    int j;

	    /* Make sure the entry gets put in even if it doesn't generate anything else */
	    if (generates == NULL)
	    {
		production_t *probe;

		/* Generate each production */
		for (probe = self -> productions_by_nonterminal[ci]; *probe != NULL; probe++)
		{
		    add_pairs_entry(
			counts, table,
			component_index(self, production_get_component(*probe, 0)),
			encode(self, production_get_index(*probe), 1));
		}
	    }
	    else
	    {
		/* Go through the generates entry for this component */
		for (j = 0; j < self -> nonterminal_count; j++)
		{
		    if (generates[j] || ci == j)
		    {
			production_t *probe;
			
			/* Generate each production */
			for (probe = self -> productions_by_nonterminal[j]; *probe != NULL; probe++)
			{
			    add_pairs_entry(
				counts, table,
				component_index(self, production_get_component(*probe, 0)),
				encode(self, production_get_index(*probe), 1));
			}
		    }
		}
	    }
	}
    }
}

/* Computes the LR(0) kernels for the grammar */
int compute_lr0_kernels(grammar_t self)
{
    int *pairs;
    int count = grammar_get_component_count(self);
    int *pairs_counts;
    int **pairs_table;
    int *goto_table;
    int i, j;

    /* Construct the first kernel to seed the table */
    pairs = (int *)malloc(sizeof(int));
    pairs[0] = encode(self, 0, 0);
    intern_kernel(self, 1, pairs);

    /* Allocate some room for the goto table */
    pairs_counts = (int *)calloc(count, sizeof(int *));
    pairs_table = (int **)calloc(count, sizeof(int *));

    /* Do the goto table thing for each kernel */
    for (i = 0; i < self -> kernel_count; i++)
    {
	kernel_t kernel = self -> kernels[i];

	/* Compute the pairs from the kernel */
	compute_pairs(self, kernel, pairs_counts, pairs_table);

	/* Allocate room for the resulting goto table */
	goto_table = (int *)calloc(count, sizeof(int));

	/* Translate the pairs into kernel indices */
	for (j = 0; j < count; j++)
	{
	    goto_table[j] = intern_kernel(self, pairs_counts[j], pairs_table[j]);
	    pairs_counts[j] = 0;
	    pairs_table[j] = NULL;
	}

	/* Set the kernel's goto table */
	kernel_set_goto_table(kernel, goto_table);
    }

    return 0;
}

/* Allocates and initializes a new nonterminal grammar_t */
grammar_t grammar_alloc(
    int production_count, production_t *productions,
    int terminal_count, component_t *terminals,
    int nonterminal_count, component_t *nonterminals)
{
    grammar_t self;

    /* Allocate space for a new grammar_t */
    if ((self = (grammar_t)malloc(sizeof(struct grammar))) == NULL)
    {
	return NULL;
    }

    /* Initialize its contents to sane values */
    self -> production_count = production_count;
    self -> productions = productions;
    self -> terminal_count = terminal_count;
    self -> terminals = terminals;
    self -> nonterminal_count = nonterminal_count;
    self -> nonterminals = nonterminals;
    self -> productions_by_nonterminal = NULL;
    self -> generates = NULL;
    self -> kernel_count = 0;
    self -> kernels = NULL;

    /* Compute the productions_by_nonterminal */
    if ((self -> productions_by_nonterminal = 
	compute_productions_by_nonterminal(
	    nonterminal_count,
	    production_count,
	    productions)) == NULL)
    {
	grammar_free(self);
	return NULL;
    }

    /* Make sure that every nonterminal has at least on production that generates it. */
    if (verify_productions_by_nonterminal(self) < 0)
    {
	grammar_free(self);
	return NULL;
    }

    /* Compute the `generates' table */
    compute_generates(self);

    /* Compute the LR(0) kernels */
    if (compute_lr0_kernels(self) < 0)
    {
	grammar_free(self);
	return NULL;
    }

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

    if (self -> terminals != NULL)
    {
	for (index = 0; index < self -> terminal_count; index++)
	{
	    component_free(self -> terminals[index]);
	}
    }

    if (self -> nonterminals != NULL)
    {
	for (index = 0; index < self -> nonterminal_count; index++)
	{
	    component_free(self -> nonterminals[index]);
	}
    }

    free(self);
}

/* Returns the number of components in the grammar */
int grammar_get_component_count(grammar_t self)
{
    return self -> terminal_count + self -> nonterminal_count;
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
    int index;

    for (index = 0; index < self -> kernel_count; index++)
    {
	int *pairs;
	int count = kernel_get_pairs(self -> kernels[index], &pairs);
	int j;

	fprintf(out, "---\nkernel %d\n", index);
	for (j = 0; j < count; j++)
	{
	    int pi;
	    int offset = decode(self, pairs[j], &pi);
	    production_print_with_offset(self -> productions[pi], out, offset);
	    fprintf(out, "\n");
	}
    }
}

/* Pretty-prints the receiver */
void grammar_print(grammar_t self, FILE *out)
{
    int index;
    char **row;

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

    /* Print out the generates table */
    fprintf(out, "\n\ngenerates: (%p)\n", self -> generates);
    for (row = self -> generates; row < self -> generates + self -> nonterminal_count; row++)
    {
	int j;

	fprintf(out, "  ");
	for (j = 0; j < self -> nonterminal_count; j++)
	{
	    if (*row == NULL)
	    {
		fprintf(out, "- ");
	    }
	    else if ((*row)[j] == 0)
	    {
		fprintf(out, "- ");
	    }
	    else
	    {
		fprintf(out, "X ");
	    }
	}
	fprintf(out, "\n");
    }
}

