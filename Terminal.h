/* $Id: Terminal.h,v 1.2 1999/02/08 16:31:24 phelps Exp $ */

#ifndef TERMINAL_H
#define TERMINAL_H

typedef struct Terminal_t *Terminal;

/* Answers a new Terminal */
Terminal Terminal_alloc(char *name, int index);

/* Frees the resources consumed by the receiver */
void Terminal_free(Terminal self);


#endif /* TERMINAL_H */
