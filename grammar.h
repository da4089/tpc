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

#ifndef GRAMMAR_H
#define GRAMMAR_H

#ifndef lint
static const char cvs_GRAMMAR_H[] = "$Id: grammar.h,v 1.16 2006/06/29 10:22:00 phelps Exp $";
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

/* Print out the parse tables in C format */
void grammar_print_c_tables(grammar_t self, FILE *out);

/* Print out the parse tables in python format */
void grammar_print_python_tables(grammar_t self, char *module, FILE *out);

/* Print out the parse tables in Golang format */
void grammar_print_golang_tables(grammar_t self, char *module, FILE *out);

#endif /* GRAMMAR_H */
