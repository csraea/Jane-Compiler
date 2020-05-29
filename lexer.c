#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

/* Symbol names, briefy (merely for convenience) */
const char *SYM_NAMES[] = {
    [VALUE] = "VALUE", [ID] = "ID", [SCAN] = "SCAN", [PRINT] = "PRINT",         /* basic functionality */
    [PLUS] = "PLUS", [MINUS] = "MINUS", [MUL] = "MUL", [DIV] = "DIV", [POWER] = "POWER",        /* arithmetic operations */
    [LT] = "LT", [GT] = "GT", [EQUAL] = "EQUAL", [LEQ] = "LEQ", [GEQ] = "GEQ", [NET] = "NET", [LOGIC] = "LOGIC",      /* logical operaions */
    [LPAR] = "LPAR", [RPAR] = "RPAR", [COMMA] = "COMMA", [SEMICOL] = "SEMICOL",     /* punctuation marks */
    [IF] = "IF", [THEN] = "THEN", [ELSE] = "ELSE", [WHILE] = "WHILE",       /* advanced functionality: conditions, cycles */
    [VAR] = "VAR", [LET] = "LET",  [SETVAL] = "SETVAL",      /* variable operations */
    [SEOF] = "SEOF", [SERROR] = "SERROR"    /* special cases */
};

/* Global variables, coherent to "public" */
Symbol lex_symbol;      /* logical & lexical meaning */
int lex_attr;       /* entries related to the symbol */

char *lex_ids[LEX_IDS_MAX];     /* An array where all the variables (IDs) are being stored */
int lex_ids_size;       /* A number of variables being used (number of IDs) */


/* Entry variables */
static char *input;     // Input string
static char c;          // Input symbol which is being proceeded
static int ic;          // Index of the next symbol (c + 1)


/* Initialization of the lexical analyzer; "string" - is the input string to analyze */
void init_lexer(char *string) {
    input = string;

    /* 0s because there are no variables and symbols discovered at the beginning */
    ic = 0;
    lex_ids_size = 0;
}


/* Save the identifier 'ID' into the table of identifiers if the new 'ID' was encountered (new variable discovered) */
int store_id(char *id) {
    int i = 0;
    while (i < lex_ids_size) {
        if (strcmp(id, lex_ids[i]) == 0)
            return i;
        i++;
    }
    lex_ids[i] = strdup(id);
    lex_ids_size++;
    return i;           /* Return the table index of the recently saved identifier */
}


/* Reading & inspection of the next symbol
 * Function call adjusts the new values for "lex_symbol" and "lex_attr" */
void next_symbol() {
    c = input[ic];
    ic++;
    while (isspace(c)) {        /* skip all the spaces: , \t, \n, and others */
        c = input[ic];
        ic++;
    }
    switch (c) {
        case ';': lex_symbol = SEMICOL; break;      // semicolumn
        case ',': lex_symbol = COMMA; break;        // comma
        case '+': lex_symbol = PLUS;  break;        // plus
        case '-': lex_symbol = MINUS; break;        // minus
        case '*': lex_symbol = MUL;   break;        // multiplication
        case '/': lex_symbol = DIV;   break;        // division
        case '^': lex_symbol = POWER; break;        // in the power of
        case '(': lex_symbol = LPAR;  break;        // left parenthesis
        case ')': lex_symbol = RPAR;  break;        // right parenthesis
        case '=': lex_symbol = EQUAL; break;        // equal symbol, for logic operations
        case '\0': lex_symbol = SEOF; break;        // end of input string
        case ':':
            if(input[ic] == '='){                   // initialize variable with expression or value
                ic++;
                lex_symbol = SETVAL;
            } else lex_symbol = SERROR;
            break;
        case '>':
            if(input[ic] == '='){       /* greater than or equal to { >= } */ 
                ic++;
                lex_symbol = GEQ;
            } else {      /* greater than { > }*/ 
                lex_symbol = GT; 
            }    
            break;
        case '<':
            if(input[ic] == '='){       /* less than or equal to { <= } */ 
                ic++;
                lex_symbol = LEQ;
            } else if(input[ic] == '>'){      /* not equal to { <> } */
                ic++;
                lex_symbol = NET;
            } else {        /* less than { < } */
                lex_symbol = LT;
            }
            break;
        case '&':
		    if (input[ic] == '&') lex_symbol = SAND;
		    else lex_symbol = SERROR;
		    ic++;
		    break;
	    case '|':
		    if (input[ic] == '|') lex_symbol = SOR;
		    else lex_symbol = SERROR;
		    ic++;
		    break;
        case 'T': case 't':
            lex_symbol = BOOLVALUE;
            lex_attr = 1;
            ic++;
            break;
        case 'F': case 'f':
            lex_symbol = BOOLVALUE;
            lex_attr = 0;
            ic++;
            break;    
        default:
            if (isdigit(c)) {          /* possibly the number */
                int id_start = ic - 1;      /* ID index - beginning */
                do {
                    c = input[ic];
                    ic++;
                } while (isalnum(c));
                ic--;
                int id_len = ic - id_start;
                char *id = (char*)calloc(id_len + 1, sizeof(char));
                memcpy(id, &input[id_start], id_len);
                id[id_len] = 0;
                lex_attr = atoi(id);
                lex_symbol = VALUE;
                free(id);
            } else if (isalpha(c)) {        /* possibly a variable or a keyword */
                int id_start = ic - 1;        /* ID index - beginning */
                do {
                    c = input[ic];
                    ic++;
                } while (isalnum(c));
                ic--;       /* Return backwards for 1 position */
                /* Copy the ID */
                int id_len = ic - id_start;
                char *id = malloc(id_len + 1);
                memcpy(id, &input[id_start], id_len);
                id[id_len] = 0;
                /* Check the key words */
                if (strcmp(id, "scan") == 0) lex_symbol = SCAN;
                else if (strcmp(id, "print") == 0) lex_symbol = PRINT;
                else if (strcmp(id, "if") == 0) lex_symbol = IF;
                else if (strcmp(id, "then") == 0) lex_symbol = THEN;
                else if (strcmp(id, "else") == 0) lex_symbol = ELSE;
                else if (strcmp(id, "while") == 0) lex_symbol = WHILE;
                else if (strcmp(id, "var") == 0) lex_symbol = VAR;
                else if (strcmp(id, "let") == 0) lex_symbol = LET;
                else if (strcmp(id, "logic") == 0) lex_symbol = LOGIC;
                else {          /* Store the new ID into the table */
                    lex_attr = store_id(id);
                    lex_symbol = ID;
                }
                free(id);
            } else {        /* Handle the error case */
                lex_symbol = SERROR;
            } 
    }
}


/* Get the simplified name of the lexical unit */
const char *symbol_name(Symbol symbol) {
    return SYM_NAMES[symbol];
}


/* Display all the lexical units from the input string */
void print_tokens() {
    printf("\nLexical analysis results\n");
    do {
        next_symbol();
        printf("  [%2d] %s", lex_symbol, symbol_name(lex_symbol));
        if (lex_symbol == VALUE) printf(" <%d>", lex_attr);
        if (lex_symbol == ID) printf(" <%d> -> %s", lex_attr, lex_ids[lex_attr]);
        printf("\n");
    } while (lex_symbol != SEOF);
}
