/* $Id: Kernel.h,v 1.2 1999/02/08 19:44:04 phelps Exp $
 *
 * A Kernel represents the Productions of a parser state which can't
 * be derived from the other productions in the kernel
 */

#ifndef KERNEL_H
#define KERNEL_H

typedef struct Kernel_t *Kernel;

#include "Grammar.h"

/* Answers a new Kernel containing the given Production */
Kernel Kernel_alloc(Grammar grammar, Production production);

/* Frees the resources consumed by the receiver */
void Kernel_free(Kernel self);

/* Prints debugging information about the receiver */
void Kernel_debug(Kernel self, FILE *out);

/* Answers the receiver's GotoTable */
Kernel *Kernel_getGotoTable(Kernel self);

#endif /* KERNEL_H */
