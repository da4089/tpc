#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "Lexer.h"
#include "Grammar.h"

/* The list of long options */
static struct option long_options[] =
{
    { "debug", no_argument, NULL, 'd' },
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, '\0' }
};


/* Print the production rule */
static void Callback(void *debug, Grammar grammar)
{
    if (debug != NULL)
    {
	Grammar_printKernels(grammar, stdout);
    }
    else
    {
	Grammar_printTable(grammar, stdout);
    }
}

/* Prints out the Usage message */
static void Usage(int argc, char *argv[])
{
    fprintf(stderr, "usage: %s [OPTION]...\n", argv[0]);
    fprintf(stderr, "  -d,          --debug\n");
    fprintf(stderr, "  -v,          --version\n");
    fprintf(stderr, "  -h,          --help\n");
}


/* Parse args and go */
int main(int argc, char *argv[])
{
    Lexer lexer;
    int choice;
    int debug = 0;
    int ch;

    /* Read options from the command line */
    while ((choice = getopt_long(argc, argv, "dvh", long_options, NULL)) > 0)
    {
	switch (choice)
	{
	    /* --debug or -d */
	    case 'd':
	    {
		debug = 1;
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
		Usage(argc, argv);
		exit(0);
	    }

	    /* bogus option */
	    default:
	    {
		Usage(argc, argv);
		exit(1);
	    }
	}
    }
    
    /* Create the lexer */
    lexer = Lexer_alloc((AcceptCallback)Callback, (void *)debug);
    /* Read characters from stdin and give them to the Lexer */
    while ((ch = fgetc(stdin)) != EOF)
    {
	Lexer_acceptChar(lexer, ch);
    }

    Lexer_acceptChar(lexer, EOF);
    return 0;
}
