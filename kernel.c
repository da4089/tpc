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
static const char cvsid[] = "$Id: kernel.c,v 1.1 1999/12/13 02:25:56 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include "component.h"
#include "production.h"
#include "grammar.h"
#include "kernel.h"

struct kernel
{
    /* The kernel's grammar */
    grammar_t grammar;

    /* The kernel's index */
    int index;

    /* The kernel's go-to table */
    kernel_t *goto_table;

    /* The propagates tables of kernel items */
    char *propagates;

    /* The follows table */
    char *follows;

    /* The propagates table of kernel items */
    int count;

    /* The kernel's encoded production/offset pairs */
    int pairs[1];
};


/* Allocates space for a kernel with `count' productions */
kernel_t do_alloc(grammar_t grammar, int count)
{
    kernel_t self;

    /* Allocate some space for the receiver */
    if ((self = (kernel_t)malloc(sizeof(struct kernel) + count * sizeof(int))) == NULL)
    {
	return NULL;
    }

    /* Initialize its contents to sane values */
    self -> grammar = grammar;
    self -> index = 0;
    self -> goto_table = NULL;
    self -> propagates = NULL;
    self -> follows = NULL;
    self -> count = count;
    return self;
}

/* Allocates and initializes a new kernel_t containing the given production */
kernel_t kernel_alloc(grammar_t grammar, production_t production)
{
    kernel_t self;

    /* Allocate some space for the new kernel_t */
    if ((self = do_alloc(grammar, 1)) == NULL)
    {
	return NULL;
    }

    /* Record our production/offset */
    self -> pairs[0] = grammar_encode(grammar, production, 0);
    return self;
}

/* Releases the resources consumed by the receiver */
void kernel_free(kernel_t self)
{
    free(self);
}

/* Returns the Kernel's goto table */
kernel_t kernel_get_goto_table(kernel_t self)
{
    int count;
    int **table;
    int index;

    /* Allocate space for the table */
    count = grammar_get_component_count(self -> grammar);
    table = (int **)calloc(count, sizeof(int *));

    /* Go through each production/offset in the kernel and compute its
     * contribution to the go-to table */
    for (index = 0; index < self -> count; index++)
    {
	grammar_compute_goto(self -> grammar, table, self -> pairs[index]);
    }

    /* Print out the resulting tables */
    printf("kernel_get_goto_table(): hello sailor\n");
    abort();
}
