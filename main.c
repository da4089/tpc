#include <stdio.h>
#include <stdlib.h>
#include "Lexer.h"

/* Parse args and go */
int main(int argc, char *argv[])
{
    Lexer lexer = Lexer_alloc(NULL, NULL);
    int ch;

    /* Read characters from stdin and give them to the Lexer */
    while ((ch = fgetc(stdin)) != EOF)
    {
	Lexer_acceptChar(lexer, ch);
    }

    Lexer_acceptChar(lexer, EOF);
}
