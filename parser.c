/***************************************************************

  Copyright (C) DSTC Pty Ltd (ACN 052 372 577) 1995.
  Unpublished work.  All Rights Reserved.

  The software contained on this media is the property of the
  DSTC Pty Ltd.  Use of this software is strictly in accordance
  with the license agreement in the accompanying LICENSE.DOC
  file.  If your distribution of this software does not contain
  a LICENSE.DOC file then you have no rights to use this
  software in any manner and should contact DSTC at the address
  below to determine an appropriate licensing arrangement.

     DSTC Pty Ltd
     Level 7, Gehrmann Labs
     University of Queensland
     St Lucia, 4072
     Australia
     Tel: +61 7 3365 4310
     Fax: +61 7 3365 4311
     Email: enquiries@dstc.edu.au

  This software is being provided "AS IS" without warranty of
  any kind.  In no event shall DSTC Pty Ltd be liable for
  damage of any kind arising out of or in connection with
  the use or performance of this software.

****************************************************************/

#ifndef lint
static const char cvsid[] = "$Id: parser.c,v 1.1 1999/12/11 14:41:24 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"

#define INITIAL_BUFFER_SIZE 512
#define INITIAL_STACK_SIZE 32

/* The type of a lexer state */
typedef int (*lexer_state_t)(parser_t self, int ch);


/* The parser data structure */
struct parser
{
    /* The parser's callback */
    parser_callback_t callback;

    /* The callback's user-supplied argument */
    void *rock;

    /* The parser's current lexical state */
    lexer_state_t lex_state;

    /* The token buffer */
    char *token;

    /* The end of the token buffer */
    char *token_end;

    /* The next character in the token buffer */
    char *point;
};


/* Function prototypes for the lexer states */
static int lex_start(parser_t self, int ch);
static int lex_comment(parser_t self, int ch);
static int lex_colon(parser_t self, int ch);
static int lex_colon_colon(parser_t self, int ch);
static int lex_id(parser_t self, int ch);
static int lex_string(parser_t self, int ch);
static int lex_error(parser_t self, int ch);

static int accept_eof(parser_t self)
{
    printf("EOF\n");
    return 0;
}

static int accept_error(parser_t self, char *token)
{
    printf("ERROR: %s\n", token);
    return 0;
}

static int accept_lbracket(parser_t self)
{
    printf("LBRACKET "); fflush(stdout);
    return 0;
}

static int accept_rbracket(parser_t self)
{
    printf("RBRACKET "); fflush(stdout);
    return 0;
}

static int accept_derives(parser_t self)
{
    printf("DERIVES "); fflush(stdout);
    return 0;
}

static int accept_id(parser_t self, char *id)
{
    printf("ID(\"%s\") ", id); fflush(stdout);
    return 0;
}

static int accept_string(parser_t self, char *string)
{
    printf("STRING(\"%s\") ", string); fflush(stdout);
    return 0;
}


/* Expands the token buffer */
static int grow_buffer(parser_t self)
{
    size_t new_length = (self -> token_end - self -> token) * 2;
    char *new_token;

    /* Allocate a bigger buffer */
    if ((new_token = (char *)realloc(self -> token, new_length)) == NULL)
    {
	return -1;
    }

    /* Update the other pointers */
    self -> point = self -> point - self -> token + new_token;
    self -> token = new_token;
    self -> token_end = new_token + new_length;
    return 0;
}

/* Appends a character to the end of the token buffer */
static int append_char(parser_t self, int ch)
{
    /* Make sure there's enough room */
    if (! (self -> point < self -> token_end))
    {
	if (grow_buffer(self) < 0)
	{
	    return -1;
	}
    }

    *(self -> point++) = ch;
    return 0;
}


/* Awaiting the first character of a token */
static int lex_start(parser_t self, int ch)
{
    switch (ch)
    {
	/* Watch for the end-of-file marker */
	case EOF:
	{
	    self -> lex_state = lex_start;
	    return accept_eof(self);
	}

	/* Watch for a string */
	case '"':
	{
	    self -> point = self -> token;
	    self -> lex_state = lex_string;
	    return 0;
	}

	/* Watch for a comment */
	case '#':
	{
	    self -> lex_state = lex_comment;
	    return 0;
	}

	/* Watch for a colon */
	case ':':
	{
	    self -> point = self -> token;
	    if (append_char(self, ch) < 0)
	    {
		return -1;
	    }

	    self -> lex_state = lex_colon;
	    return 0;
	}

	/* Watch for a left square bracket */
	case '[':
	{
	    self -> lex_state = lex_start;
	    return accept_lbracket(self);
	}

	/* Watch for a right square bracket */
	case ']':
	{
	    self -> lex_state = lex_start;
	    return accept_rbracket(self);
	}
    }

    /* Skip whitespace */
    if (isspace(ch))
    {
	self -> lex_state = lex_start;
	return 0;
    }

    /* Alpha or underscore is the beginning of an ID */
    if (ch == '_' || isalpha(ch))
    {
	self -> point = self -> token;
	return lex_id(self, ch);
    }

    /* Anything else is bogus */
    self -> point = self -> token;
    return lex_error(self, ch);
}

static int lex_comment(parser_t self, int ch)
{
    /* Watch for EOF */
    if (ch == EOF)
    {
	return lex_start(self, ch);
    }

    /* Watch for newline */
    if (ch == '\n')
    {
	self -> lex_state = lex_start;
	return 0;
    }

    /* Anything else is part of the comment (and ignored) */
    return 0;
}

/* We've seen a single colon.  Look for a second */
static int lex_colon(parser_t self, int ch)
{
    /* Look for another colon */
    if (ch == ':')
    {
	if (append_char(self, ch) < 0)
	{
	    return -1;
	}

	self -> lex_state = lex_colon_colon;
	return 0;
    }

    /* Anything else is an error */
    return lex_error(self, ch);
}

static int lex_colon_colon(parser_t self, int ch)
{
    /* Look for an equals sign */
    if (ch == '=')
    {
	self -> lex_state = lex_start;
	return accept_derives(self);
    }

    /* Anything else is an error */
    return lex_error(self, ch);
}

static int lex_id(parser_t self, int ch)
{
    /* Watch for additional id characters */
    if (ch == '_' || ch == '-' || isalnum(ch))
    {
	if (append_char(self, ch) < 0)
	{
	    return -1;
	}

	self -> lex_state = lex_id;
	return 0;
    }

    /* Null-terminate the ID token */
    if (append_char(self, 0) < 0)
    {
	return -1;
    }

    /* Accept the token */
    if (accept_id(self, self -> token) < 0)
    {
	return -1;
    }

    /* Scan the character again from the start state */
    return lex_start(self, ch);
}

static int lex_string(parser_t self, int ch)
{
    /* Watch for illegal EOF in the string */
    if (ch == EOF)
    {
	return lex_error(self, ch);
    }

    /* Watch for the closing quote */
    if (ch == '"')
    {
	/* Null-terminate the token */
	if (append_char(self, 0) < 0)
	{
	    return -1;
	}

	/* Accept it */
	self -> lex_state = lex_start;
	return accept_string(self, self -> token);
    }

    /* Anything else is part of the string */
    if (append_char(self, ch) < 0)
    {
	return -1;
    }

    self -> lex_state = lex_string;
    return 0;
}

/* We've encountered a bogus token.  Continue reading it until we have 
 * a complete token to play with before complaining */
static int lex_error(parser_t self, int ch)
{
    /* Watch for the end of the token */
    if (ch == EOF || isspace(ch))
    {
	/* Null-terminate the token */
	if (append_char(self, 0) < 0)
	{
	    return -1;
	}

	/* Acknowledge a broken token */
	if (accept_error(self, self -> token) < 0)
	{
	    return -1;
	}

	/* Rescan this character */
	return lex_start(self, ch);
    }

    /* This must still be part of the token */
    if (append_char(self, ch) < 0)
    {
	return -1;
    }

    self -> lex_state = lex_error;
    return 0;
}


/* Allocates and initializes a new parser_t */
parser_t parser_alloc(parser_callback_t callback, void *rock)
{
    parser_t self;

    /* Allocate some memory for the new parser_t */
    if ((self = (parser_t)malloc(sizeof(struct parser))) == NULL)
    {
	return NULL;
    }

    /* Initialize all the fields to sane values */
    self -> callback = callback;
    self -> rock = rock;
    self -> lex_state = lex_start;
    self -> token = NULL;
    self -> token_end = NULL;
    self -> point = NULL;

    /* Allocate some room for the token buffer */
    if ((self -> token = (unsigned char *)malloc(INITIAL_BUFFER_SIZE)) == NULL)
    {
	parser_free(self);
	return NULL;
    }

    self -> token_end = self -> token + INITIAL_BUFFER_SIZE;
    return self;
}

/* Releases the resources consumed by the receiver */
void parser_free(parser_t self)
{
    if (self -> token != NULL)
    {
	free(self -> token);
    }

    free(self);
}

/* Parses the characters in the buffer */
int parser_parse(parser_t self, unsigned char *buffer, size_t length)
{
    unsigned char *pointer;

    /* An empty buffer indicates end of file */
    if (length == 0)
    {
	return self -> lex_state(self, EOF);
    }

    /* Otherwise send each character through the lexer */
    for (pointer = buffer; pointer < buffer + length; pointer++)
    {
	if (self -> lex_state(self, *pointer) < 0)
	{
	    self -> lex_state = lex_start;
	    return -1;
	}
    }

    return 0;
}

#include <unistd.h>
int main(int argc, char *argv[])
{
    parser_t parser;

    if ((parser = parser_alloc(NULL, NULL)) == NULL)
    {
	perror("parser_alloc(): failed");
	exit(1);
    }

    while (1)
    {
	char buffer[2048];
	ssize_t length;

	if ((length = read(STDIN_FILENO, buffer, 2048)) < 0)
	{
	    perror("read(): failed");
	    exit(1);
	}

	parser_parse(parser, buffer, length);

	if (length == 0)
	{
	    return 0;
	}
    }
}
