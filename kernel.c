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
static const char cvsid[] = "$Id: kernel.c,v 1.2 1999/12/13 16:51:52 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include "component.h"
#include "production.h"
#include "grammar.h"
#include "kernel.h"

struct kernel
{
    /* The propagates table of kernel items */
    int count;

    /* The kernel's encoded production/offset pairs */
    int *pairs;

    /* The kernel's go-to table */
    int *goto_table;
};

/* Allocates and initializes a new kernel_t containing the given pairs */
kernel_t kernel_alloc(int count, int *pairs)
{
    kernel_t self;

    /* Allocate some space for the new kernel_t */
    if ((self = (kernel_t)malloc(sizeof(struct kernel))) == NULL)
    {
	return NULL;
    }

    /* Initialize its contents to sane values */
    self -> count = count;
    self -> pairs = pairs;
    self -> goto_table = NULL;
    return self;
}

/* Releases the resources consumed by the receiver */
void kernel_free(kernel_t self)
{
    if (self -> pairs != NULL)
    {
	free(self -> pairs);
    }

    if (self -> goto_table != NULL)
    {
	free(self -> goto_table);
    }

    free(self);
}

/* Returns nonzero if the kernel matches the pairs */
int kernel_matches(kernel_t self, int count, int *pairs)
{
    int index;

    /* Do the easy test first */
    if (self -> count != count)
    {
	return 0;
    }

    /* Go through and compare element by element */
    for (index = 0; index < count; index++)
    {
	if (self -> pairs[index] != pairs[index])
	{
	    return 0;
	}
    }

    /* The match! */
    return 1;
}

/* Returns the kernel's pairs */
int kernel_get_pairs(kernel_t self, int **pairs_out)
{
    *pairs_out = self -> pairs;
    return self -> count;
}

/* Sets the receiver's goto table */
void kernel_set_goto_table(kernel_t self, int *goto_table)
{
    self -> goto_table = goto_table;
}
