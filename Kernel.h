/* $Id: Kernel.h,v 1.1 1999/02/08 18:28:06 phelps Exp $
 *
 * A Kernel represents the Productions of a parser state which can't
 * be derived from the other productions in the kernel
 */

#ifndef KERNEL_H
#define KERNEL_H

typedef struct Kernel_t *Kernel;

/* Answers a new Kernel containing the given Production */
Kernel Kernel_alloc(Grammar grammar, Production production);

#endif /* KERNEL_H */
