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

#ifndef PARSER_H
#define PARSER_H

#ifndef lint
static const char cvs_PARSER_H[] = "$Id: parser.h,v 1.3 1999/12/11 18:02:41 phelps Exp $";
#endif /* lint */

/* The parser type */
typedef struct parser *parser_t;

/* The type of the parser callback */
typedef void (*parser_callback_t)(void *arg, grammar_t grammar);

/* Allocates and initializes a new parser_t */
parser_t parser_alloc(parser_callback_t callback, void *arg);

/* Releases the resources consumed by the receiver */
void parser_free(parser_t self);

/* Parses the characters in buffer */
int parser_parse(parser_t self, unsigned char *buffer, size_t length);

#endif /* PARSER_H */
