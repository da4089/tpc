/* $Id: Parser.h,v 1.2 1999/02/16 09:32:43 phelps Exp $
 *
 * An "upside-down" parser for incremental parsing from stdin
 */

#ifndef PARSER_H
#define PARSER_H

/* The Parser data structure type */
typedef struct Parser_t *Parser;


/* The Accept callback type */
typedef void (*AcceptCallback)(void *context, void *expression);


/* Answers a new Parser */
Parser Parser_alloc(AcceptCallback callback, void *context);

/* Releases the resources consumed by the receiver */
void Parser_free(Parser self);


/* Updates the receiver's state based on the next ::= token */
void Parser_acceptDerives(Parser self);

/* Updates the receiver's state based on the next non-terminal token */
void Parser_acceptNonterminal(Parser self, char *value);

/* Updates the receiver's state based on the next terminal token */
void Parser_acceptTerminal(Parser self, char *value);

/* Updates the receiver's state based on the next stop token */
void Parser_acceptFunction(Parser self, char *value);

/* Updates the receiver's state based on the end of input */
void Parser_acceptEOF(Parser self);

#endif /* PARSER_H */
