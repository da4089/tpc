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
int debug = 0;

/* The list of long options */
static struct option long_options[] =
{
    { "output", required_argument, NULL, 'o' },
    { "debug", no_argument, NULL, 'd' },
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, '\0' }
};

/* Print the production rule */
static void parser_cb(void *ignored, grammar_t grammar)
{
    FILE *file;

    /* Print the kernels if debug is on */
    if (debug)
    {
	grammar_print_kernels(grammar, stderr);
    }

    /* If an output filename was specified then write to it */
    if (output_filename != NULL)
    {
	/* Try to open the output file */
	if ((file = fopen(output_filename, "w")) == NULL)
	{
	    perror("unable to open file for write");
	    exit(1);
	}

	/* Write the parse table to the file */
	grammar_print_table(grammar, file);
	fclose(file);
	return;
    }

    /* Otherwise just print it to stdout */
    grammar_print_table(grammar, stdout);
}

/* Prints out the usage message */
static void usage(int argc, char *argv[])
{
    fprintf(stderr, "usage: %s [OPTION]... [FILE]\n", argv[0]);
    fprintf(stderr, "  -o file,     --output=file\n");
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
    while ((choice = getopt_long(argc, argv, "o:dqvh", long_options, NULL)) > 0)
    {
	switch (choice)
	{
	    /* --output or -o */
	    case 'o':
	    {
		output_filename = optarg;
		break;
	    }

	    /* --debug or -d */
	    case 'd':
	    {
		debug = 1;
		break;
	    }

	    /* --quiet or -q */
	    case 'q':
	    {
		close(STDERR_FILENO);
		break;
	    }

	    /* --version or -v */
	    case 'v':
	    {
		printf("%s version %s\n", PACKAGE, VERSION);
		exit(0);
	    }

	    /* --help or -h */
	    case 'h':
	    {
		usage(argc, argv);
		exit(0);
	    }

	    /* bogus option */
	    default:
	    {
		usage(argc, argv);
		exit(1);
	    }
	}
    }

    /* Look for an input file name */
    if (optind < argc)
    {
	input_filename = argv[optind++];
    }

    /* Make sure we don't have any extra args */
    if (optind < argc)
    {
	usage(argc, argv);
	exit(1);
    }
    
    /* Create the parser */
    if ((parser = parser_alloc(parser_cb, NULL)) == NULL)
    {
	perror("parser_alloc(): failed");
	exit(1);
    }

    /* Open up the input file */
    if (input_filename != NULL)
    {
	if ((fd = open(input_filename, O_RDONLY)) < 0)
	{
	    perror("unable to open file for read");
	    exit(1);
	}
    }
    else
    {
	fd = STDIN_FILENO;
    }

    /* Read characters from stdin and give them to the Lexer */
    while (1)
    {
	unsigned char buffer[BUFFER_SIZE];
	ssize_t length;

	if ((length = read(fd, buffer, BUFFER_SIZE)) < 0)
	{
	    perror("read(): failed");
	    abort();
	}

	/* Parse what we've read so far */
	if (parser_parse(parser, input_filename, buffer, length) < 0)
	{
	    close(fd);
	    exit(1);
	}

	/* Watch for EOF */
	if (length == 0)
	{
	    close(fd);
	    exit(0);
	}
    }
}
