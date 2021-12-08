%{
#include <stdio.h>
#include "utils.h"
#include "quad.h"
#include "table.h"
#include "decaf.tab.h"

extern int yylex();
void yyerror(char *msg);
void raler(char *msg);

extern struct s_context *context;

// int yydebug = 1; 
%}

%code requires {
#include "table.h"
}

%union {
	char charval;
	char strval[255];
	int intval;
	quadop qoval;
	struct {
		enum elem_type type;
		union {
			quadop result;
			struct {
				ilist *true;
				ilist *false;
			} boolexpr;
		} u;
	} exprval;
	struct {
		ilist *next;
		ilist *next_break;
		ilist *next_continue;
	} stateval;
	struct s_arglist *alistval;
	enum elem_type etypeval;
}

%type <exprval> expr
%type <intval> marker
%type <qoval> method_call
%type <stateval> statement statement_l statement_l_ goto block
%type <alistval> arg_l arg_l_ expr_l
%type <etypeval> arg

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
: CLPR '{' pushctx decl popctx '}'
;

pushctx
: %empty {
	context = tos_pushctx();
}
;

popctx
: %empty {
	context = tos_popctx();
}
;

decl
: field_decl_l method_decl_l
| method_decl_l
;

field_decl_l 
: field_decl
| field_decl_l field_decl
;

field_decl 
: INT field_decl_int_l ';'
| BOOL field_decl_bool_l ';'
;

field_decl_int_l 
: field_decl_int
| field_decl_int_l ',' field_decl_int
;

field_decl_int 
: ID {
	struct s_entry *ident = tos_newname($1);
	ident->type = elementary_type(T_INT);
}
| ID '[' INT_LITERAL ']' {
	struct s_entry *ident = tos_newname($1);
	ident->type = array_type(T_INT, $3);
}
;

field_decl_bool_l 
: field_decl_bool
| field_decl_bool_l ',' field_decl_bool
;

field_decl_bool 
: ID {
	struct s_entry *ident = tos_newname($1);
	ident->type = elementary_type(T_BOOL);
}
| ID '[' INT_LITERAL ']' {
	struct s_entry *id = tos_newname($1);
	id->type = array_type(T_BOOL, $3);
}
;

method_decl_l 
: method_decl
| method_decl_l method_decl
;

method_decl
: INT ID {
	struct s_entry *id = tos_newname($2);
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name(id->ident)));
} '(' arg_l_ ')' {
	struct s_entry *ident = tos_lookup($2);
	ident->type = function_type(R_INT, $5);
} block {
	YCHK($8.next_break != NULL, "break doit être dans une boucle");
	YCHK($8.next_continue != NULL, "continue doit être dans une boucle");
	if ($5 != NULL)
		context = tos_popctx();
}
| BOOL ID {
	struct s_entry *id = tos_newname($2);
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name(id->ident)));
} '(' arg_l_ ')' {
	struct s_entry *id = tos_lookup($2);
	id->type = function_type(R_BOOL, $5);
} block {
	YCHK($8.next_break != NULL, "break doit être dans une boucle");
	YCHK($8.next_continue != NULL, "continue doit être dans une boucle");
	if ($5 != NULL)
		context = tos_popctx();
}
| VOID ID {
	struct s_entry *id = tos_newname($2);
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name(id->ident)));
} '(' arg_l_ ')' {
	struct s_entry *id = tos_lookup($2);
	id->type = function_type(R_VOID, $5);
} block {
	YCHK($8.next_break != NULL, "break doit être dans une boucle");
	YCHK($8.next_continue != NULL, "continue doit être dans une boucle");
	if ($5 != NULL)
		context = tos_popctx();
}
;

arg_l_
: pushctx arg_l {$$ = $2;}
| %empty {$$ = NULL;}
;

arg_l 
: arg {
	$$ = arglist_addend(NULL, $1);
}
| arg_l ',' arg {
	$$ = arglist_addend($1, $3);
}
;

arg 
: INT ID {
	struct s_entry *ident = tos_newname($2);
	ident->type = elementary_type(T_INT);
	$$ = E_INT;
}
| BOOL ID {
	struct s_entry *ident = tos_newname($2);
	ident->type = elementary_type(T_BOOL);
	$$ = E_BOOL;
}
;

block 
: '{' pushctx var_decl_l_ statement_l_ popctx '}' {
	$$ = $4;
}
;

var_decl_l_
: %empty
| var_decl_l
;

var_decl_l 
: var_decl
| var_decl_l var_decl
;

var_decl 
: INT id_int_l ';'
| BOOL id_bool_l ';'
;

id_int_l 
: ID {
	struct s_entry *ident = tos_newname($1);
	ident->type = elementary_type(T_INT);
}
| id_int_l ',' ID {
	struct s_entry *ident = tos_newname($3);
	ident->type = elementary_type(T_INT);
}
;

id_bool_l 
: ID {
	struct s_entry *ident = tos_newname($1);
	ident->type = elementary_type(T_BOOL);
}
| id_bool_l ',' ID {
	struct s_entry *ident = tos_newname($3);
	ident->type = elementary_type(T_BOOL);
}
;

statement_l_
: %empty {
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| statement_l {
	$$ = $1;
}
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
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "275: la variable %s n'existe pas", $1);
		exit(EXIT_FAILURE);
	}
	// YCHK(id == NULL, "la variable n'existe pas");
	quadop qid = quadop_name(id->ident);
	if (is_elementary_type(id->type, T_INT)) { // cas int
		YCHK($3.type != E_INT, "expression doit être int");
		gencode(quad_make(Q_MOVE, $3.u.result, quadop_empty(), qid));
	} else if (is_elementary_type(id->type, T_BOOL)) { // cas bool
		YCHK($3.type != E_BOOL, "expression doit être bool");
		complete($3.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qid));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($3.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qid));
	} else {
		raler("variable doit être de type int ou bool");
	}
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' '=' expr ';' {
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "301: la variable %s n'existe pas", $1);
		exit(EXIT_FAILURE);
	}
	// YCHK(id == NULL, "la variable n'existe pas");
	YCHK(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	YCHK($3.type != E_INT, "index de tableau doit être int");
	quadop qid = quadop_name(id->ident);
	if (is_array_type(id->type, E_INT)) { // cas int
		YCHK($6.type != E_INT, "expression doit être int");
		gencode(quad_make(Q_SETI, qid, $3.u.result, $6.u.result));	
	} else { // cas bool
		YCHK($6.type != E_BOOL, "expression doit être bool");
		complete($6.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_SETI, qid, $3.u.result, quadop_bool(1)));
		complete($6.u.boolexpr.false, nextquad);
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		gencode(quad_make(Q_SETI, qid, $3.u.result, quadop_bool(0)));
	}
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID ADD_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "327: la variable %s n'existe pas\n", $1);
		exit(EXIT_FAILURE);
	}
	// YCHK(id == NULL, "la variable n'existe pas");
	YCHK(!is_elementary_type(id->type, T_INT), "la variable n'est pas de type int");
	YCHK($3.type != E_INT, "l'expression doit être int");
	quadop qid = quadop_name(id->ident);
	gencode(quad_make(Q_ADD, qid, $3.u.result, qid));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' ADD_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "342 la variable %s n'existe pas\n", $1);
		exit(EXIT_FAILURE);
	}
	// YCHK(id == NULL, "la variable n'existe pas");
	YCHK(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	YCHK(!is_array_type(id->type, E_INT), "la variable n'est pas un tableau de int");
	YCHK($3.type != E_INT, "index de tableau doit être int");
	YCHK($6.type != E_INT, "l'expression doit être int");
	quadop qid = quadop_name(id->ident);
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, qid, $3.u.result, qo));
	gencode(quad_make(Q_ADD, qo, $6.u.result, qo));
	gencode(quad_make(Q_SETI, qid, $3.u.result, qo));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID SUB_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "364: la variable %s n'existe pas\n", $1);
		exit(EXIT_FAILURE);
	}
	// YCHK(id == NULL, "la variable n'existe pas");
	YCHK(!is_elementary_type(id->type, T_INT), "la variable n'est pas de type int");
	YCHK($3.type != E_INT, "l'expression doit être int");
	quadop qid = quadop_name(id->ident);
	gencode(quad_make(Q_SUB, qid, $3.u.result, qid));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' SUB_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "379: la variable %s n'existe pas", $1);
		exit(EXIT_FAILURE);
	}
	YCHK(id == NULL, "la variable n'existe pas");
	YCHK(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	YCHK(!is_array_type(id->type, T_INT), "la variable n'est pas un tableau de int");
	YCHK($3.type != E_INT, "index de tableau doit être int");
	YCHK($6.type != E_INT, "l'expression doit être int");
	quadop qid = quadop_name(id->ident);
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, qid, $3.u.result, qo));
	gencode(quad_make(Q_SUB, qo, $6.u.result, qo));
	gencode(quad_make(Q_SETI, qid, $3.u.result, qo));
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
	YCHK($3.type != E_BOOL, "l'expression doit être bool");
	complete($3.u.boolexpr.true, $5);
	$$.next = concat($3.u.boolexpr.false, $6.next);
	$$.next_break = $6.next_break;
	$$.next_continue = $6.next_continue;
}
| IF '(' expr ')' marker block goto ELSE marker block {
	YCHK($3.type != E_BOOL, "l'expression doit être bool");
	complete($3.u.boolexpr.true, $5);
	complete($3.u.boolexpr.false, $9);
	$$.next = concat(concat($6.next, $7.next), $10.next);
	$$.next_break = concat($6.next_break, $10.next_break);
	$$.next_continue = concat($6.next_continue, $10.next_continue);
}
| FOR pushctx ID '=' expr ',' expr {
	YCHK($5.type != E_INT, "l'expression doit être int");
	YCHK($7.type != E_INT, "l'expression doit être int");
	struct s_entry *id = tos_newname($3);
	id->type = elementary_type(T_INT);
	gencode(quad_make(Q_MOVE, $5.u.result, quadop_empty(), quadop_name(id->ident)));
} marker {
	struct s_entry *id = tos_lookup($3);
	gencode(quad_make(Q_BGT, quadop_name(id->ident), $7.u.result, quadop_empty()));
} block {
	struct s_entry *id = tos_lookup($3);
	complete($11.next, nextquad);
	complete($11.next_continue, nextquad);
	quadop qid = quadop_name(id->ident);
	gencode(quad_make(Q_ADD, qid, quadop_cst(1), qid));
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($9)));
	$$.next = concat(crelist($9), $11.next_break);
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| RETURN expr ';' {
	if ($2.type == E_INT) { // cas int
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), $2.u.result));
		$$.next = NULL;
	} else { // cas bool
		complete($2.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_bool(1)));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($2.u.boolexpr.false, nextquad);
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
	struct s_entry *id = tos_lookup($1);
	YCHK(id == NULL, "la fonction n'existe pas");
	YCHK(!is_elementary_type(id->type, T_FUNCTION), "la variable n'est pas une fonction");
	// TODO: checker la fonction
	quadop qo;
	if (is_function_type(id->type, R_VOID, NULL)) { // procédure
		qo = quadop_empty();
	} else if (is_function_type(id->type, R_INT, NULL)) { // fonction renvoyant int
		struct s_entry *temp = newtemp(); 
		temp->type = elementary_type(T_INT);
		qo = quadop_name(temp->ident);
	} else if (is_function_type(id->type, R_BOOL, NULL)) { // fonction renvoyant bool
		struct s_entry *temp = newtemp(); 
		temp->type = elementary_type(T_BOOL);
		qo = quadop_name(temp->ident);
	} else {
		raler("arguments incorrect");
	}
	gencode(quad_make(Q_CALL, quadop_name(id->ident), quadop_empty(), qo));
	$$ = qo;
}
| ID '(' expr_l ')' {
	struct s_entry *id = tos_lookup($1);
	YCHK(id == NULL, "la fonction n'existe pas");
	YCHK(!is_elementary_type(id->type, T_FUNCTION), "la variable n'est pas une fonction");
	// TODO: checker la fonction
	quadop qo;
	if (is_function_type(id->type, R_VOID, $3)) { // procédure
		qo = quadop_empty();
	} else if (is_function_type(id->type, R_INT, $3)) { // fonction renvoyant int
		struct s_entry *temp = newtemp(); 
		temp->type = elementary_type(T_INT);
		qo = quadop_name(temp->ident);
	} else if (is_function_type(id->type, R_BOOL, $3)) { // fonction renvoyant bool
		struct s_entry *temp = newtemp(); 
		temp->type = elementary_type(T_BOOL);
		qo = quadop_name(temp->ident);
	} else {
		raler("arguments incorrect");
	}
	gencode(quad_make(Q_CALL, quadop_name(id->ident), quadop_cst(arglist_size($3)), qo));
	$$ = qo;
}
;

expr_l 
: expr {
	// TODO: gérer cas int / bool
	$$ = arglist_addend(NULL, $1.type);
	if ($1.type == E_INT) {
		gencode(quad_make(Q_PARAM, $1.u.result, quadop_empty(), quadop_empty()));
	} else {
	}
}
| expr_l ',' expr {
	// TODO: gérer cas int / bool
	$$ = arglist_addend($1, $3.type);
	if ($3.type == E_INT) {
		gencode(quad_make(Q_PARAM, $3.u.result, quadop_empty(), quadop_empty()));
	} else {
	}
}
;

expr 
: ID {
	struct s_entry *id = tos_lookup($1);
	if (id == NULL) {
		fprintf(stderr, "544: la variable %s n'existe pas", $1);
		exit(EXIT_FAILURE);
	}
	// YCHK(id == NULL, "la variable n'existe pas");
	if (is_elementary_type(id->type, T_INT)) { // cas int
		$$.type = E_INT;
		$$.u.result = quadop_name(id->ident);
	} else if (is_elementary_type(id->type, T_BOOL)) { // cas bool
		$$.type = E_BOOL;
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, quadop_name(id->ident), quadop_bool(1), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else {
		raler("expression doit être int ou bool");
	}
} 
| ID '[' expr ']' {
	struct s_entry *id = tos_lookup($1);
	// YCHK(id == NULL, "la variable n'existe pas");
	if (id == NULL) {
		fprintf(stderr, "565: la variable %s n'existe pas", $1);
		exit(EXIT_FAILURE);
	}
	YCHK(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	YCHK($3.type != E_INT, "index de tableau doit être int");
	struct s_entry *temp = newtemp();
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, quadop_name(id->ident), $3.u.result, qo));
	if (is_array_type(id->type, T_INT)) {
		$$.type = E_INT;
		temp->type = elementary_type(T_INT);
	}
	else {
		$$.type = E_BOOL;
		temp->type = elementary_type(T_BOOL);
	}
	$$.u.result = qo;
}
| method_call {
	YCHK($1.type == QO_EMPTY, "appel de procédure dans expression");
	if ($1.type == QO_CST) {
		$$.type = E_INT;
		$$.u.result = $1;
	} else {
		$$.type = E_BOOL;
		if ($1.u.boolean) {
			$$.u.boolexpr.true = crelist(nextquad);
			$$.u.boolexpr.false = NULL;
		}
		else {
			$$.u.boolexpr.false = crelist(nextquad);
			$$.u.boolexpr.true = NULL;
		}
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| INT_LITERAL {
	$$.type = E_INT;
	$$.u.result = quadop_cst($1);
}
| CHAR_LITERAL {
	$$.type = E_INT;
	$$.u.result = quadop_cst((int) $1);
}
| BOOL_LITERAL {
	$$.type = E_BOOL;
	if ($1) {
		$$.u.boolexpr.true = crelist(nextquad);
		$$.u.boolexpr.false = NULL;
	}
	else {
		$$.u.boolexpr.false = crelist(nextquad);
		$$.u.boolexpr.true = NULL;
	}
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '+' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_ADD, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '-' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_SUB, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '*' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MUL, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '/' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_DIV, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '%' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MOD, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '<' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BLT, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '>' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BGT, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr LEQ expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BLE, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr BEQ expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BGE, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr EQ expr {
	YCHK($1.type != $3.type, "les opérandes doivent être de même type");
	$$.type = E_BOOL;
	if ($1.type == E_INT) {
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, $1.u.result, $3.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));

	} else {
		// TODO
	}
}
| expr NEQ expr {
	YCHK($1.type != $3.type, "les opérandes doivent être de même type");
	$$.type = E_BOOL;
	if ($1.type == E_INT) {
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BNE, $1.u.result, $3.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else {
		// TODO
	}
}
| expr AND marker expr {
	YCHK($1.type != E_BOOL, "opérande doit être bool");
	YCHK($4.type != E_BOOL, "opérande doit être bool");
	$$.type = E_BOOL;
	complete($1.u.boolexpr.true, $3);
	$$.u.boolexpr.false = concat($1.u.boolexpr.false, $4.u.boolexpr.false);
	$$.u.boolexpr.true = $4.u.boolexpr.true;
}
| expr OR marker expr {
	YCHK($1.type != E_BOOL, "opérande doit être bool");
	YCHK($4.type != E_BOOL, "opérande doit être bool");
	$$.type = E_BOOL;
	complete($1.u.boolexpr.false, $3);
	$$.u.boolexpr.true = concat($1.u.boolexpr.true, $4.u.boolexpr.true);
	$$.u.boolexpr.false = $4.u.boolexpr.false;
}
| '-' expr {
	YCHK($2.type != E_BOOL, "opérande doit être int");
	struct s_entry *temp = newtemp(); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MINUS, $2.u.result, quadop_empty(), qo));
	$$.type = E_INT;
	$$.u.result = qo;
} %prec UMINUS
| '!' expr %prec NOT {
	YCHK($2.type != E_BOOL, "opérande doit être bool");
	$$.type = E_BOOL;
	$$.u.boolexpr.true = $2.u.boolexpr.false;
	$$.u.boolexpr.false = $2.u.boolexpr.true;
}
| '(' expr ')' {
	$$ = $2;
}
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

void yyerror(char *msg) {
	printf("Error: %s\n", msg);
}

void raler(char *msg) {
	printf("%s\n", msg);
	exit(1);
}
