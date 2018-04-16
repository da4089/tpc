/***********************************************************************

  Copyright (C) 1999-2006 by Mantara Software (ABN 17 105 665 594).
  All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above
     copyright notice, this list of conditions and the following
     disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials
     provided with the distribution.

   * Neither the name of the Mantara Software nor the names
     of its contributors may be used to endorse or promote
     products derived from this software without specific prior
     written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

***********************************************************************/

#ifndef PARSER_H
#define PARSER_H

#ifndef lint
static const char cvs_PARSER_H[] = "$Id: parser.h,v 1.6 2006/06/29 10:22:00 phelps Exp $";
#endif /* lint */

/* The parser type */
typedef struct parser *parser_t;

/* The supported output formats */
enum format
{
    /* Generate C tables */
    FORMAT_C,

    /* Generate Golang tables */
    FORMAT_GOLANG,

    /* Generate Python tables */
    FORMAT_PYTHON
};

typedef enum format format_t;


/* The type of the parser callback */
typedef void (*parser_callback_t)(void *arg, grammar_t grammar);

/* Allocates and initializes a new parser_t */
parser_t parser_alloc(parser_callback_t callback, void *arg);

/* Releases the resources consumed by the receiver */
void parser_free(parser_t self);

/* Parses the characters in buffer */
int parser_parse(parser_t self, char *filename, unsigned char *buffer, size_t length);

#endif /* PARSER_H */
