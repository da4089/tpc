/* $Id: Lexer.h,v 1.2 1999/02/08 09:24:28 phelps Exp $
 *
 * An "upside-down" lexer for one-character-at-a-time parsing from
 * stdin
 */

#ifndef LEXER_H
#define LEXER_H

/* The Lexer data structure */
typedef struct Lexer_t *Lexer;

#include "Parser.h"

/* Answers a new Lexer */
Lexer Lexer_alloc(AcceptCallback callback, void *context);

/* Releases the resources consumed by the receiver */
void Lexer_free(Lexer self);


/* Updates the recevier's state based on the given character */
void Lexer_acceptChar(Lexer self, int ch);

#endif /* LEXER_H */
