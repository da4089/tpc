/* $Id: Nonterminal.h,v 1.2 1999/02/08 16:31:24 phelps Exp $ */

#ifndef NONTERMINAL_H
#define NONTERMINAL_H

typedef struct Nonterminal_t *Nonterminal;

/* Answers a new Nonterminal */
Nonterminal Nonterminal_alloc(char *name, int index);

/* Frees the resources consumed by the receiver */
void Nonterminal_free(Nonterminal self);

/* Pretty-prints the receiver */
void Nonterminal_print(Nonterminal self, FILE *out);

/* Answers the receiver's index */
int Nonterminal_getIndex(Nonterminal self);

#endif /* NONTERMINAL_H */
