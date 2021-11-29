%{
#include <stdio.h>
#include "quad.h"
extern int yylex();
void yyerror(char *msg);

// int yydebug = 1; 
%}

%union {
	char charval;
	char *strval;
	int intval;
	quadop qoval;
	struct {
		quadop result;
		ilist *true;
		ilist *false;
		// ...
	} exprval;
	struct {
		ilist *next;
		ilist *next_break;
		ilist *next_continue;
	} stateval;
	struct {
		unsigned length;
	} listval;
}

%type <exprval> expr
%type <intval> marker
%type <listval> expr_l
%type <qoval> method_call
%type <stateval> statement statement_l goto block

%token CLPR							// class Program
%token INT BOOL						// type
%token VOID							// void
%token <strval> ID							// id
%token IF ELSE						// if else
%token FOR							// for
%token BREAK CONTINUE				// break continue
%token RETURN						// return
%token ADD_ASSIGN SUB_ASSIGN		// '+=' '-='
%token LEQ BEQ EQ NEQ				// '<=' '>=' '==' '!='
%token AND OR						// '&&' '||'
%token <intval> INT_LITERAL BOOL_LITERAL		// ...
%token <charval> CHAR_LITERAL 
%token STRING_LITERAL	// ...

%left '+' '-'
%nonassoc NOT
%left '*' '/' '%'
%left '<' '>' LEQ BEQ
%left EQ NEQ
%left AND
%left OR
%nonassoc UMINUS

%start program

%%
program 
: CLPR '{' field_decl_l method_decl_l '}'
//| CLPR '{' field_decl_l '}'
| CLPR '{' method_decl_l '}'
//| CLPR '{' '}'
;

field_decl_l 
: field_decl
| field_decl_l field_decl
;

field_decl 
: INT field_decl_elem_l ';'
| BOOL field_decl_elem_l ';'
;

field_decl_elem_l 
: field_decl_elem
| field_decl_elem_l ',' field_decl_elem
;

field_decl_elem 
: ID
| ID '[' INT_LITERAL ']'	//{printf("INT %d\n", $3);}
;

method_decl_l 
: method_decl
| method_decl_l method_decl
;

method_decl 
: INT ID '(' arg_l ')' {
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} block {
	if ($7.next_break != NULL)
		printf("break doit être dans une boucle\n");
	if ($7.next_continue != NULL)
		printf("continue doit être dans une boucle\n");
}
| BOOL ID '(' arg_l ')' {
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} block {
	if ($7.next_break != NULL)
		printf("break doit être dans une boucle\n");
	if ($7.next_continue != NULL)
		printf("continue doit être dans une boucle\n");
}
| VOID ID '(' arg_l ')' {
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} block {
	if ($7.next_break != NULL)
		printf("break doit être dans une boucle\n");
	if ($7.next_continue != NULL)
		printf("continue doit être dans une boucle\n");
}
| INT ID '(' ')' {
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} block {
	if ($6.next_break != NULL)
		printf("break doit être dans une boucle\n");
	if ($6.next_continue != NULL)
		printf("continue doit être dans une boucle\n");
}
| BOOL ID '(' ')' {
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} block {
	if ($6.next_break != NULL)
		printf("break doit être dans une boucle\n");
	if ($6.next_continue != NULL)
		printf("continue doit être dans une boucle\n");
}
| VOID ID '(' ')' {
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} block {
	if ($6.next_break != NULL)
		printf("break doit être dans une boucle\n");
	if ($6.next_continue != NULL)
		printf("continue doit être dans une boucle\n");
}
;

arg_l 
: arg
| arg_l ',' arg
;

arg 
: INT ID
| BOOL ID
;

block 
: '{' var_decl_l statement_l '}' {
	$$ = $3;
}
| '{' var_decl_l '}' {
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| '{' statement_l '}' {
	$$ = $2;
}
| '{' '}' {
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
;

var_decl_l 
: var_decl
| var_decl_l var_decl
;

var_decl 
: INT id_l ';'
| BOOL id_l ';'
;

id_l 
: ID
| id_l ',' ID
;

statement_l 
: statement {$$ = $1;}
| statement_l marker statement {
	complete($1.next, $2);
	$$.next = $3.next;
	$$.next_break = concat($1.next_break, $3.next_break);
	$$.next_continue = concat($1.next_continue, $3.next_continue);
}
;

statement 
: ID '=' expr ';' {
	quadop id = quadop_name($1);
	if (1) { // cas int
		gencode(quad_make(Q_MOVE, $3.result, quadop_empty(), id));
	} else { // cas bool
		complete($3.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), id));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($3.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), id));
	}
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' '=' expr ';' {
	quadop id = quadop_name($1);
	if (1) { // cas int
		gencode(quad_make(Q_SETI, id, $3.result, $6.result));	
	} else { // cas bool
		complete($6.true, nextquad);
		gencode(quad_make(Q_SETI, id, $3.result, quadop_bool(1)));
		complete($6.false, nextquad);
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		gencode(quad_make(Q_SETI, id, $3.result, quadop_bool(0)));
	}
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID ADD_ASSIGN expr ';' {
	quadop id = quadop_name($1);
	gencode(quad_make(Q_ADD, id, $3.result, id));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' ADD_ASSIGN expr ';' {
	quadop tmp = newtemp();
	quadop id = quadop_name($1);
	gencode(quad_make(Q_GETI, id, $3.result, tmp));
	gencode(quad_make(Q_ADD, tmp, $6.result, tmp));
	gencode(quad_make(Q_SETI, id, $3.result, tmp));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID SUB_ASSIGN expr ';' {
	quadop id = quadop_name($1);
	gencode(quad_make(Q_SUB, id, $3.result, id));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' SUB_ASSIGN expr ';' {
	quadop tmp = newtemp();
	quadop id = quadop_name($1);
	gencode(quad_make(Q_GETI, id, $3.result, tmp));
	gencode(quad_make(Q_SUB, tmp, $6.result, tmp));
	gencode(quad_make(Q_SETI, id, $3.result, tmp));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| method_call ';' {
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| IF '(' expr ')' marker block {
	complete($3.true, $5);
	$$.next = concat($3.false, $6.next);
	$$.next_break = $6.next_break;
	$$.next_continue = $6.next_continue;
}
| IF '(' expr ')' marker block goto ELSE marker block {
	complete($3.true, $5);
	complete($3.false, $9);
	$$.next = concat(concat($6.next, $7.next), $10.next);
	$$.next_break = concat($6.next_break, $10.next_break);
	$$.next_continue = concat($6.next_continue, $10.next_continue);
}
| FOR ID '=' expr ',' expr {
	// ! créer un nouvel identificateur !
	// ! push une nouvelle table des symboles !
	gencode(quad_make(Q_MOVE, $4.result, quadop_empty(), quadop_name($2)));
} marker {
	gencode(quad_make(Q_BGT, quadop_name($2), $4.result, quadop_empty()));
} block {
	complete($10.next, nextquad);
	complete($10.next_continue, nextquad);
	quadop id = quadop_name($2);
	gencode(quad_make(Q_ADD, id, quadop_cst(1), id));
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($8)));
	$$.next = concat(crelist($8), $10.next_break);
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| RETURN expr ';' {
	if (1) { // cas int
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), $2.result));
		$$.next = NULL;
	} else { // cas bool
		complete($2.true, nextquad);
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_bool(1)));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($2.false, nextquad);
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_bool(0)));
	}
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| RETURN ';' {
	gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_empty()));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| BREAK ';' {
	$$.next = NULL;
	$$.next_continue = NULL;
	$$.next_break = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| CONTINUE ';' {
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| block {
	$$ = $1;
}
;

method_call
: ID '(' ')' {
	quadop qo;
	if (1) // procédure
		qo = quadop_empty();
	else // fonction
		qo = newtemp();
	gencode(quad_make(Q_CALL, quadop_name($1), quadop_empty(), qo));
	$$ = qo;
}
| ID '(' expr_l ')' {
	quadop qo;
	if (1) // procédure
		qo = quadop_empty();
	else // fonction
		qo = newtemp();
	gencode(quad_make(Q_CALL, quadop_name($1), quadop_cst($3.length), qo));
	$$ = qo;
}
;

expr_l 
: expr {
	$$.length = 1;
	gencode(quad_make(Q_PARAM, $1.result, quadop_empty(), quadop_empty()));
}
| expr_l ',' expr {
	$$.length = $1.length + 1;
	gencode(quad_make(Q_PARAM, $3.result, quadop_empty(), quadop_empty()));
}
;

expr 
: ID {
	if (1) { // cas int
		$$.result = quadop_name($1);
	} else { // cas bool
		$$.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, quadop_name($1), quadop_bool(1), quadop_empty()));
		$$.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
} 
| ID '[' expr ']' {
	quadop qo = newtemp();
	gencode(quad_make(Q_GETI, quadop_name($1), $3.result, qo));
	$$.result = qo;
}
| method_call {
	if ($1.type == QO_EMPTY)
		printf("appel de procédure dans expression");
	else
		$$.result = $1;
}
| INT_LITERAL {$$.result = quadop_cst($1);}
| CHAR_LITERAL {$$.result = quadop_cst((int) $1);}
| BOOL_LITERAL {
	if ($1)
		$$.true = crelist(nextquad);
	else
		$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '+' expr {
	quadop qo = newtemp();
	gencode(quad_make(Q_ADD, $1.result, $3.result, qo));
	$$.result = qo;
}
| expr '-' expr {
	quadop qo = newtemp();
	gencode(quad_make(Q_SUB, $1.result, $3.result, qo));
	$$.result = qo;
}
| expr '*' expr {
	quadop qo = newtemp();
	gencode(quad_make(Q_MUL, $1.result, $3.result, qo));
	$$.result = qo;
}
| expr '/' expr {
	quadop qo = newtemp();
	gencode(quad_make(Q_DIV, $1.result, $3.result, qo));
	$$.result = qo;
}
| expr '%' expr {
	quadop qo = newtemp();
	gencode(quad_make(Q_MOD, $1.result, $3.result, qo));
	$$.result = qo;
}
| expr '<' expr {
	$$.true = crelist(nextquad);
	gencode(quad_make(Q_BLT, $1.result, $3.result, quadop_empty()));
	$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '>' expr {
	$$.true = crelist(nextquad);
	gencode(quad_make(Q_BGT, $1.result, $3.result, quadop_empty()));
	$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr LEQ expr {
	$$.true = crelist(nextquad);
	gencode(quad_make(Q_BLE, $1.result, $3.result, quadop_empty()));
	$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr BEQ expr {
	$$.true = crelist(nextquad);
	gencode(quad_make(Q_BGE, $1.result, $3.result, quadop_empty()));
	$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr EQ expr {
	$$.true = crelist(nextquad);
	gencode(quad_make(Q_BEQ, $1.result, $3.result, quadop_empty()));
	$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr NEQ expr {
	$$.true = crelist(nextquad);
	gencode(quad_make(Q_BNE, $1.result, $3.result, quadop_empty()));
	$$.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr AND marker expr {
	complete($1.true, $3);
	$$.false = concat($1.false, $4.false);
	$$.true = $4.true;
}
| expr OR marker expr {
	complete($1.false, $3);
	$$.true = concat($1.true, $4.true);
	$$.false = $4.false;
}
| '-' expr {
	quadop qo = newtemp();
	gencode(quad_make(Q_MINUS, $2.result, quadop_empty(), qo));
	$$.result = qo;
} %prec UMINUS
| '!' expr %prec NOT {
	$$.true = $2.false;
	$$.false = $2.true;
}
| '(' expr ')' {$$ = $2;}
;

marker
: %empty {$$ = nextquad;}
;

goto
: %empty {
	$$.next = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
;

%%

void yyerror(char *msg)
{
	printf("Error: %s\n", msg);
}
