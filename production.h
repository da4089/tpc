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

#ifndef PRODUCTION_H
#define PRODUCTION_H

#ifndef lint
static const char cvs_PRODUCTION_H[] = "$Id: production.h,v 1.7 2000/03/15 05:42:15 phelps Exp $";
#endif /* lint */

/* The production type */
typedef struct production *production_t;

/* Allocates and initializes a new production_t */
production_t production_alloc(
    int index,
    component_t nonterminal,
    int component_count,
    component_t *components,
    char *reduction);

/* Releases the resources consumed by the receiver */
void production_free(production_t self);


/* Returns the nonterminal index of the production's left-hand side */
int production_get_nonterminal_index(production_t self);

/* Returns the production's index */
int production_get_index(production_t self);

/* Returns the nth component of the production's right-hand-side */
component_t production_get_component(production_t self, int index);

/* Pretty-prints the receiver */
void production_print(production_t self, FILE *out);

/* Pretty-prints the receiver with a `*' after the nth element */
void production_print_with_offset(production_t self, FILE *out, int offset);

/* Prints the production as a C struct */
void production_print_c_struct(production_t self, FILE *out);

/* Prints the production as a python tuple */
void production_print_python_tuple(production_t self, char *module, FILE *out);

#endif /* PRODUCTION_H */
