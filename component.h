/***********************************************************************

  Copyright (C) 1999-2006 by Mantara Software (ABN 17 105 665 594).
  All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above
     copyright notice, this list of conditions and the following
     disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials
     provided with the distribution.

   * Neither the name of the Mantara Software nor the names
     of its contributors may be used to endorse or promote
     products derived from this software without specific prior
     written permission. 

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

***********************************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H

#ifndef lint
static const char cvs_COMPONENT_H[] = "$Id: component.h,v 1.9 2006/06/29 10:22:00 phelps Exp $";
#endif /* lint */

/* The component type */
typedef struct component *component_t;


/* Allocates and initializes a new nonterminal component_t */
component_t nonterminal_alloc(char *filename, int line, char *name, int index);

/* Allocates and initializes a new terminal component_t */
component_t terminal_alloc(char *filename, int line, char *name, int index);

/* Releases the resources consumed by the receiver */
void component_free(component_t self);

/* Returns the filename and line number of the component's first mention */
int component_get_origin(component_t self, char **filename_out);

/* Pretty-prints the receiver */
void component_print(component_t self, FILE *out);

/* Prints the receiver as a C enum entry */
void component_print_c_enum(component_t self, FILE *out);

/* Prints the receiver as a python assignment */
void component_print_python_assign(component_t self, FILE *out);

/* Returns the receiver's name */
char *component_get_name(component_t self);

/* Returns the receiver's index */
int component_get_index(component_t self);

/* Returns nonzero if the component is a nonterminal */
int component_is_nonterminal(component_t self);

#endif /* COMPONENET_H */
