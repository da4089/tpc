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

#ifndef GRAMMAR_H
#define GRAMMAR_H

#ifndef lint
static const char cvs_GRAMMAR_H[] = "$Id: grammar.h,v 1.14 1999/12/20 15:04:54 phelps Exp $";
#endif /* lint */

/* The grammar type */
typedef struct grammar *grammar_t;


/* Allocates and initializes a new nonterminal grammar_t */
grammar_t grammar_alloc(
    int production_count, production_t *productions,
    int terminal_count, component_t *terminals,
    int nonterminal_count, component_t *nonterminals);

/* Releases the resources consumed by the receiver */
void grammar_free(grammar_t self);


/* Returns the number of components in the grammar */
int grammar_get_component_count(grammar_t self);

/* Construct a single number to represent a production_t and offset */
int grammar_encode(grammar_t self, production_t production, int offset);

/* Answers the production and offset encoded by the integer */
int grammar_decode(grammar_t self, int code, production_t *production_out);

/* Inserts the go-to contribution of the encoded production/offset into the table */
void grammar_compute_goto(grammar_t self, int **table, int code);

/* Print out the kernels */
void grammar_print_kernels(grammar_t self, FILE *out);

/* Prints out the parse tables */
void grammar_print_table(grammar_t self, FILE *out);

#endif /* GRAMMAR_H */
