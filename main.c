/* -*- mode: c; c-file-style: "elvin" -*- */
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include "component.h"
#include "production.h"
#include "grammar.h"
#include "parser.h"

#define BUFFER_SIZE 4096

char *input_filename = NULL;
char *output_filename = NULL;
format_t format = FORMAT_C;
char *module = NULL;
int debug = 0;

/* The list of long options */
static struct option long_options[] =
{
    { "output", required_argument, NULL, 'o' },
    { "c", no_argument, NULL, 'c' },
    { "python", optional_argument, NULL, 'p' },
    { "debug", no_argument, NULL, 'd' },
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, '\0' }
};

/* Print out the parse tables */
static void print_tables(grammar_t grammar, FILE *out)
{
    /* Write the parse table to the file */
    switch (format) {
    case FORMAT_C:
        grammar_print_c_tables(grammar, out);
        break;

    case FORMAT_PYTHON:
        grammar_print_python_tables(grammar, module, out);
        break;

    default:
        /* Should never get here */
        fprintf(stderr, "*** Unrecognized format %d\n", format);
    }
}

/* Print the production rule */
static void parser_cb(void *ignored, grammar_t grammar)
{
    FILE *file;

    /* Print the kernels if debug is on */
    if (debug) {
	grammar_print_kernels(grammar, stderr);
    }

    /* If an output filename was specified then write to it */
    if (output_filename != NULL) {
	/* Try to open the output file */
	if ((file = fopen(output_filename, "w")) == NULL) {
	    perror("unable to open file for write");
	    exit(1);
	}

	print_tables(grammar, file);
	fclose(file);
	return;
    }

    /* Otherwise just print it to stdout */
    print_tables(grammar, stdout);
}

/* Prints out the usage message */
static void usage(int argc, char *argv[])
{
    fprintf(stderr, "usage: %s [OPTION]... [FILE]\n", argv[0]);
    fprintf(stderr, "  -o file,     --output=file\n");
    fprintf(stderr, "  -c,          --c\n");
    fprintf(stderr, "  -p,          --python[=import-module]\n");
    fprintf(stderr, "  -d,          --debug\n");
    fprintf(stderr, "  -q,          --quiet\n");
    fprintf(stderr, "  -v,          --version\n");
    fprintf(stderr, "  -h,          --help\n");
}


/* Parse args and go */
int main(int argc, char *argv[])
{
    parser_t parser;
    int choice;
    int fd;

    /* Read options from the command line */
    while ((choice = getopt_long(argc, argv, "o:cp?dqvh",
                                 long_options, NULL)) != -1) {
	switch (choice) {
        case 'o':
            /* --output or -o */
            output_filename = optarg;
            break;

        case 'c':
            /* --c or -c */
            format = FORMAT_C;
            break;

        case 'p':
	    /* --python or -p */
            format = FORMAT_PYTHON;
            module = optarg;
            break;

        case 'd':
	    /* --debug or -d */
            debug = 1;
            break;

        case 'q':
	    /* --quiet or -q */
            close(STDERR_FILENO);
            break;

        case 'v':
	    /* --version or -v */
            printf("%s version %s\n", PACKAGE, VERSION);
            exit(0);

        case 'h':
	    /* --help or -h */
            usage(argc, argv);
            exit(0);

        default:
            /* bogus option */
            usage(argc, argv);
            exit(1);
	}
    }

    /* Look for an input file name */
    if (optind < argc) {
	input_filename = argv[optind++];
    }

    /* Make sure we don't have any extra args */
    if (optind < argc) {
	usage(argc, argv);
	exit(1);
    }
    
    /* Create the parser */
    if ((parser = parser_alloc(parser_cb, NULL)) == NULL) {
	perror("parser_alloc(): failed");
	exit(1);
    }

    /* Open up the input file */
    if (input_filename != NULL) {
	if ((fd = open(input_filename, O_RDONLY)) < 0)
	{
	    perror("unable to open file for read");
	    exit(1);
	}
    } else {
	fd = STDIN_FILENO;
    }

    /* Read characters from stdin and give them to the Lexer */
    while (1) {
	unsigned char buffer[BUFFER_SIZE];
	ssize_t length;

	if ((length = read(fd, buffer, BUFFER_SIZE)) < 0) {
	    perror("read(): failed");
	    abort();
	}

	/* Parse what we've read so far */
	if (parser_parse(parser, input_filename, buffer, length) < 0) {
	    close(fd);
	    exit(1);
	}

	/* Watch for EOF */
	if (length == 0) {
	    close(fd);
	    exit(0);
	}
    }
}
