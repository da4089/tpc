#include <stdio.h>
#include <stdlib.h>
#include "Lexer.h"
#include "Grammar.h"

/* Print the production rule */
static void Frumble(void *ignored, Grammar grammar)
{
    Grammar_debug(grammar, stdout);
    printf("\n");
}

/* Parse args and go */
int main(int argc, char *argv[])
{
    Lexer lexer = Lexer_alloc((AcceptCallback)Frumble, NULL);
    int ch;

    /* Read characters from stdin and give them to the Lexer */
    while ((ch = fgetc(stdin)) != EOF)
    {
	Lexer_acceptChar(lexer, ch);
    }

    Lexer_acceptChar(lexer, EOF);
    return 0;
}
