/* $Id: Terminal.h,v 1.5 1999/02/16 08:40:01 phelps Exp $ */

#ifndef TERMINAL_H
#define TERMINAL_H

typedef struct Terminal_t *Terminal;

/* Answers a new Terminal */
Terminal Terminal_alloc(char *name, int index);

/* Frees the resources consumed by the receiver */
void Terminal_free(Terminal self);

/* Pretty-prints the receiver */
void Terminal_print(Terminal self, FILE *out);

/* Pretty-prints the receiver for the enum */
void Terminal_printEnum(Terminal self, FILE *out);

/* Answers the receiver's index */
int Terminal_getIndex(Terminal self);

#endif /* TERMINAL_H */
