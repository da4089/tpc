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
static const char cvs_PRODUCTION_H[] = "$Id: production.h,v 1.1 1999/12/11 16:58:49 phelps Exp $";
#endif /* lint */

/* The production type */
typedef struct production *production_t;

/* Allocates and initializes a new empty production_t */
production_t production_alloc(int index);

/* Releases the resources consumed by the receiver */
void production_free(production_t self);

/* Sets the receiver's nonterminal */
void production_set_nonterminal(production_t self, component_t nonterminal);

/* Adds another component to the end of the production's list */
void production_add_component(production_t self, component_t component);

/* Sets the receiver's function */
void production_set_function(production_t self, char *function);

/* Pretty-prints the receiver */
void production_print(production_t self, FILE *out);

/* Pretty-prints the receiver with a `*' after the nth element */
void production_print_with_offset(production_t self, FILE *out, int offset);

#endif /* PRODUCTION_H */
