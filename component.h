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

#ifndef COMPONENT_H
#define COMPONENT_H

#ifndef lint
static const char cvs_COMPONENT_H[] = "$Id: component.h,v 1.1 1999/12/11 16:12:23 phelps Exp $";
#endif /* lint */

/* The component type */
typedef struct component *component_t;


/* Allocates and initializes a new nonterminal component_t */
component_t nonterminal_alloc(char *name, int index);

/* Allocates and initializes a new terminal component_t */
component_t terminal_alloc(char *name, int index);

/* Releases the resources consumed by the receiver */
void component_free(component_t self);

/* Pretty-prints the receiver */
void component_print(component_t self, FILE *out);

/* Returns the receiver's index */
int component_get_index(component_t self);

#endif /* NONTERMINAL_H */
