/* $Id: Terminal.h,v 1.1 1999/02/08 13:05:09 phelps Exp $ */

#ifndef TERMINAL_H
#define TERMINAL_H

typedef struct Terminal_t *Terminal;

/* Answers a new Terminal */
Terminal Terminal_alloc(char *name, int index);

/* Frees the resources consumed by the receiver */
void Terminal_free(Terminal self);

/* Pretty-prints the receiver */
void Terminal_print(Terminal self, FILE *out);


#endif /* TERMINAL_H */
