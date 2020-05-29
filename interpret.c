#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "lexer.h"
#include "generator.h"
#include "ifunc.h"

#define KeySet unsigned long int        /* is used for defining the sets of expected
                                         * symbols during the error checking */
#define E 1 <<      /* bit shift is necessary for checking symbols */

int errors = 0;

/* Sets of possible keys required for specified operations */
KeySet H_Term, H_Pow, H_Mul, H_Expr, H_Eql, H_Print, H_Scan, H_Let, H_Var, H_Logic, H_Inside, H_analyze;
KeySet H_While, H_If;       /* using is not implemented yet */

/* Initialize the interpreter by intialiing each of keysets */
void init_interpret() { 
    H_Term = E VALUE | E ID | E LPAR;
    H_Pow = H_Term;
    H_Mul = H_Pow;
    H_Expr = H_Mul;
    H_Eql = H_Expr;
    H_Print = E PRINT;
    H_Scan = E SCAN;
    H_Let = E LET;
    H_Var = E VAR;
    H_If = E IF;
    H_While = E WHILE;
    H_Logic = E GT | E LT | E GEQ | E LEQ | E EQUAL| E NET;
    H_analyze = H_Print | H_Scan | H_Let | H_Var | H_If | H_While;
    H_Inside = H_analyze;
}

int variables[LEX_IDS_MAX];         /* an array which is used for storing variables */
_Bool crtVarIDs[LEX_IDS_MAX];        /* an array which is responsible for holding the
                                            * info about initialization of each variable
                                            * true - initialized, false - not initialized */


/**********************************************************/
/* * * * * * * * * PARSER AND INTERPRETER * * * * * * * * */
/**********************************************************/


/* G R A M M A R:
 * Term -> VALUE | "(" Expr ")" | ID
 * Power -> Term { ("^") Term }
 * Mul -> Power {("*"|"/") Power}
 * Expr -> Term {("+"|"-") Term}
 * Eql -> Expr ( LEQ | LT | GT | LEQ | GEQ | ET | NET ) Expr
 * Print_Expr -> "print" Expr ";"
 * Print_Logic -> "logic" Expr ";"
 * Scan -> "scan" ID {"," ID} ";"
 * Var -> "var" ID {"," ID} ";"
 * Let -> "let" ID ":=" Expr ";" 
 * Analyze -> Scan | Print | Var | Let | If | While
 * Magic -> { Analyze }
 */
void print(KeySet K), scan(KeySet K), c_var(KeySet K), l_var(KeySet K), analyze(KeySet K);
int term(KeySet K), power(KeySet K), mul(KeySet K), expr(KeySet K);
size_t magic(KeySet K);

void error(const char *msg, KeySet K) {
    fprintf(stderr, "ERROR: %s\n", msg);        /* error alert */
    /* skip the symbols that are not the keywords */
    while (!(E lex_symbol & K)) next_symbol();
}

/* Symbol checkup on input and reading of the next one */
int match(const Symbol expected, KeySet K){
    if (lex_symbol == expected) {
        int attr = lex_attr;
        next_symbol();
        return attr;        /* returns the attribute of the examinated symbol */
    } else {
        char *msg = malloc(100);
        snprintf(msg, 100, "MATCH: Expected symbol: `%s` ; encountered: `%s`",
                symbol_name(expected), symbol_name(lex_symbol));
        error(msg, K);
        return 0;           /* default return value in the error case */
    }
}


void check(const char *msg, KeySet K) {
    if (!(E lex_symbol & K)) error(msg, K);
}



int incr_power(int base, int exponentiation){         /* name `pow` is alredy used in math.h;
                                                 * a function implements "power" arithmetic operation */
    int result = 1;
    for( ; true; base *= base){
        if (exponentiation & 1) result *= base;
        exponentiation >>= 1;
        if (!exponentiation) break;
    }

    return result;
}

/* Term -> VALUE | "(" Expr ")" | ID*/
int term(KeySet K) {
    int value;
	check("TERM: Value or parenthesis is expected", K | H_Term);
    if(lex_symbol == LPAR) {
        next_symbol();
        value = expr(K | H_Term);
        match(RPAR, K);
    } else if(lex_symbol == VALUE) {
        value = lex_attr;
        write_number(value);
        next_symbol();
    } else if(lex_symbol == ID) {
        if(crtVarIDs[lex_attr]){
                value = variables[lex_attr];
                write_var(lex_attr);
                next_symbol();
        } else {
            error("TERM: Something went wrong! Variable was not created!\n", K | H_Term);
            next_symbol();
        }
    } else error("TERM: Something went wrong! Operand is missing!", K);

	return value;
}

/* Power -> Term { ("^") Term } */
int power(KeySet K) {
	int leftOp, rightOp;
	Symbol op;
	leftOp = term(K | E POWER | H_Term);
    check("POWER: Operator `^` is expected", E POWER | H_Term | E SEMICOL | K );
	while ((E lex_symbol) & (E POWER | H_Term))	{
		if ((E lex_symbol) & (E POWER)) {
            op = lex_symbol;
            next_symbol();
        } else error("POWER: Something went wrong! Operator `^` is missing!", H_Term | K);
		rightOp = term(K | E POWER | H_Term);
		switch(op){
			case POWER:
				leftOp = incr_power(leftOp, rightOp);
				write_pow(leftOp);
				break;
			default: assert("POWER: Something went wrong! Unexpected operator!");
		}
        check("POWER: Operator `^` is expected", E POWER | H_Term  | E SEMICOL| K);
	}

	return leftOp;
}

/* Mul -> Power {("*"|"/") Power} */
int mul(KeySet K) {
    int leftOp, rightOp;
    Symbol op;
    leftOp = power(K | E MUL | E DIV | H_Pow);
    check("MUL: Operator `*` or `/` is expected", E MUL | E DIV | H_Pow | E SEMICOL | K);
    while((E lex_symbol) & (E MUL | E DIV | H_Pow)){
        if ((E lex_symbol) & (E MUL | E DIV)) {
            op = lex_symbol;
            next_symbol();
        } else error("MUL: Something went wrong! Operator `*` or `/` is missing!", H_Pow | K);
        rightOp = power(E MUL | E DIV | H_Pow | K);
        if(op == MUL) {
            leftOp = leftOp * rightOp;
            write_mul();
        } else if(op == DIV) {
            leftOp = leftOp / rightOp;
            write_div();
        } else assert("MUL: Something went wrong! Unexpected operator!");

        check("MUL: Operator `*` or `/` is expected", E MUL | E DIV | H_Pow | E SEMICOL | K);
    }

    return leftOp;
}

/* Expr -> Mul {("+"|"-") Mul} */
/* Expr -> Expr ( LT | GT | SETVAL | GEQ| LEQ | NET ) Expr */
int expr(KeySet K) {
    int leftOp, rightOp;
    Symbol op;
    leftOp = mul(E PLUS | E MINUS | E LT | E GT | E EQUAL | E GEQ | E LEQ| E NET | H_Mul | K);
    check("EXPR: Someting went wrong! Operator `+` or `-` is expected", E PLUS | E MINUS | E LT | E GT | E EQUAL | E GEQ | E LEQ | E NET | E SEMICOL | H_Mul | K);
    while ((E lex_symbol) & (E PLUS | E MINUS | E LT | E GT | E EQUAL | E GEQ | E LEQ | E NET | H_Mul)){
        if ((E lex_symbol) & (E PLUS | E MINUS | E LT | E GT | E EQUAL | E GEQ | E LEQ| E NET)) {
            op = lex_symbol;
            next_symbol();
        } else error("EXPR: Someting went wrong! Operator `+` or `-` is expected", H_Mul | K);
        rightOp = mul(K | E PLUS | E MINUS | E LT | E GT | E EQUAL | E GEQ | E LEQ | E NET | H_Mul);
        switch (op) {
            case MINUS:
                leftOp = leftOp - rightOp;
                write_sub();
                break;
            case PLUS:
                leftOp = leftOp + rightOp;
                write_add();
                break;
            // case SAND:
            //     leftOp = leftOp && rightOp;
            //     //leftOp = sand(H_sand);
            //     break;
            // case SOR:
            //     leftOp = leftOp || rightOp;
            //     //leftOp = iexpr(H_or);
            //     break;
            default: assert("EXPR: Something went wrong! Unexpected operator!");
        }
        check("EXPR: Operator `+` or `-` is expected", E PLUS | E MINUS | E SEMICOL | H_Mul | K);
    }

    return leftOp;
}

/* Eql -> Expr ( LT | GT | EQUAL | GEQ | LEQ | NET | SAND | SOR ) Expr */
int logical_expr(KeySet K) {
    int leftOp, rightOp;
    Symbol op;
    leftOp = mul(H_Logic| H_Mul | K);
    check("LOGIC: Logic expression is expected!", H_Logic| E SEMICOL | H_Mul | K);
    while ((E lex_symbol) & (H_Logic| H_Mul)){
        if ((E lex_symbol) & (H_Logic)) {
            op = lex_symbol;
            next_symbol();
        } else error("LOGIC: Logic expression is expected!", H_Mul | K);
        rightOp = mul(K |H_Logic| H_Mul);
        switch (op) {
            case GT:
                leftOp = leftOp > rightOp;
                write_GT();
                break;
            case LT:
                leftOp = leftOp < rightOp;
                write_LT();
                break;
            case LEQ:
                leftOp = leftOp <= rightOp;
                write_LEQ();
                break;
            case GEQ:
                leftOp = leftOp >= rightOp;
                write_GEQ();
                break;
            case EQUAL:
                leftOp = leftOp == rightOp;
                write_EQUAL();
                break;
            case NET:
                leftOp = leftOp != rightOp;
                write_NET();
                break;
            case SAND:
                if (leftOp == 1 && rightOp == 1) leftOp = 1;
                else leftOp = 0;
                break;
            case SOR:
                if (leftOp == 0 && rightOp == 0) leftOp = 0;
                else leftOp = 1;
                break;
            default: assert("LOGIC: Unexpected operator!");
        }
        check("LOGIC: Logic expression is expected!",  E SEMICOL | H_Logic | H_Mul | K);
    }
    return leftOp;
}

/* Print -> "print" Expr ";"*/
void print_Expr(KeySet K){
	match(PRINT, H_Expr | E SEMICOL | K );
    int result = expr(H_Expr | K);
    printf("Result:\t%d\n", result);
    match(SEMICOL, K);
}

void print_Logic(KeySet K){
    match(LOGIC, H_Logic | E SEMICOL | K );
    int result = logical_expr(H_Logic | K);
    printf("Logical expression:\t%s\n", result ? "true" : "false");
    match(SEMICOL, K);
}

void scan_var(int id_idx){         /* get the value of a variable declared before from user */
    int value;
    printf("%s = ", lex_ids[id_idx]);
    scanf("%d", &value);
    variables[id_idx] = value;
}

/* Scan -> "scan" ID {"," ID} ";" */
void scan(KeySet K){
	next_symbol();
	match(ID, E COMMA | E ID | E SEMICOL | K);
    if(crtVarIDs[lex_attr]) scan_var(lex_attr);
    else error("Oops... Something went wrong! Variable was not created!\n", K | SEMICOL); 
	for(; (E lex_symbol) & (E COMMA); ) {
		next_symbol();
		match(ID, K | E COMMA | E ID);
		if(crtVarIDs[lex_attr]) scan_var(lex_attr);
		else error("Oops... Something went wrong! Variable was not created!\n", K | SEMICOL);
	}
    match(SEMICOL, K);
}

/* Var -> "var" ID {"," ID} ";" */
void c_var(KeySet K){
    next_symbol();
    match(ID, E COMMA | E ID | E SEMICOL | K);
    if(!crtVarIDs[lex_attr]){
        crtVarIDs[lex_attr] = true;
        variables[lex_attr] = 0;
    } else error("Oops... Something went wrong! The variable had been already created!\n", K | E SEMICOL);
    while((E lex_symbol) & (E COMMA)){
        next_symbol();
        match(ID, E COMMA | E ID | E SEMICOL | K);
        if(!crtVarIDs[lex_attr]){
            variables[lex_attr] = 0;
            crtVarIDs[lex_attr] = true;
        } else error("Oops... Something went wrong! The variable had been already created!\n", K | E SEMICOL);
    }
    match(SEMICOL, K); 
}

/* Let -> "let" ID ":=" Expr ";"  */
void l_var(KeySet K){
    next_symbol();
    match(ID, E SETVAL | H_Expr);
    int prev_lex_attr = lex_attr;
    match(SETVAL, K | H_Expr);
    if(crtVarIDs[prev_lex_attr]){
		variables[prev_lex_attr] = expr(K | H_Expr);
		write_var(prev_lex_attr);
	} else error("Oops... Something went wrong! Variable was not created!\n", K | E SEMICOL);
	match(SEMICOL, K);
}

/* Analyze -> Read | Print | Create | Set | If | While*/
void analyze(KeySet K){
    if(lex_symbol == SCAN) scan(K | H_analyze);
    else if(lex_symbol == PRINT) print_Expr(K | H_analyze);
    else if(lex_symbol == VAR) c_var(K | H_analyze);
    else if(lex_symbol == LET) l_var(K | H_analyze);
    else if(lex_symbol == LOGIC) print_Logic(K| H_Logic);
    else error("Oops... Something went wrong! No keyword was found!", K);
}

// AND  -> Term [ <&&> AND ]
int sand(KeySet keys) {
    int leftOp, rightOp;
    KeySet allkeys;
    allkeys =(E SAND) | keys;
    leftOp = term(keys | (E SAND));
    check(0,keys | (E SAND));
    if((E lex_symbol) & (E SAND)) {
          next_symbol();
          rightOp = sand(keys);
          leftOp = leftOp && rightOp;
    }
    return leftOp;
}


// Expr -> AND [ <||> Expr ]
int iexpr(KeySet keys) {
	int leftOp, rightOp;
	KeySet allkeys;
	allkeys = (E SOR) | (E LPAR) | keys;
	leftOp = sand(keys | (E SOR) | (E LPAR));
    check(0,keys | (E SOR) | (E LPAR));
    if((E lex_symbol) & (E SOR)) {
          next_symbol();
          rightOp = expr(keys);
          leftOp = leftOp || rightOp;
    }
	return leftOp;
}


/* Magic -> { Analyze }*/
size_t magic(KeySet K) {
	for( ;lex_symbol != SEOF; ) analyze(K | H_analyze);
    return SUCCESS;
}


char *source;
int main(int argc, char** argv) {

    FILE *output_file = fopen("magic.bin", "wb");
    init_generator(output_file);
    
    // enter from keyboard or get from command line args;
    int inp_result = get_input(argc, argv);
    if(inp_result != SUCCESS) {
        fclose(output_file);
        exit(inp_result);
    }

    init_lexer(source);
    init_interpret();
    print_tokens();

    write_begin(lex_ids_size);

    puts("Beginning of interpritation...");
    init_lexer(source);
    next_symbol();


    magic(E SEOF);
    write_result();

    write_end();
    generate_output();
    
    free(source);
    fclose(output_file);

    exit(EXIT_SUCCESS);
}