/* $Id: Kernel.h,v 1.7 1999/02/12 05:40:03 phelps Exp $
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

/* Sets the receiver's index */
void Kernel_setIndex(Kernel self, int index);

/* Answers the receiver's index */
int Kernel_getIndex(Kernel self);

/* Answers non-zero if the receiver equals the kernel */
int Kernel_equals(Kernel self, Kernel kernel);

/* Answers the receiver's GotoTable */
Kernel *Kernel_getGotoTable(Kernel self);

/* Computes the receiver's closure */
void Kernel_computeClosure(Kernel self);

/* Prepare to propagate the follows info */
void Kernel_propagatePrepare(Kernel self);

/* Propagate follows information from this kernel to the ones it derives */
void Kernel_propagateFollows(Kernel self, int *isDone);

/* Adds a Terminal to the receiver's follows-set. Returns non-zero if
 * it wasn't already there */
int Kernel_addFollowsTerminal(Kernel self, int pair, Terminal terminal);


#endif /* KERNEL_H */
