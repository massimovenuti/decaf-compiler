%{
#include <stdio.h>
#include "utils.h"
#include "quad.h"
#include "table.h"
#include "decaf.tab.h"

extern int yylex();
void yyerror(char *msg);
void raler(char *msg);

// int yydebug = 1; 
%}

%code requires {
#include "table.h"
}

%union {
	char charval;
	char *strval;
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
	struct {
		unsigned length;
	} listval;
	struct s_arglist *alistval;
	enum elem_type etypeval;
}

%type <exprval> expr
%type <intval> marker
%type <listval> expr_l
%type <qoval> method_call
%type <stateval> statement statement_l statement_l_ goto block
%type <alistval> arg_l arg_l_
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
	tos_pushctx();
}
;

popctx
: %empty {
	tos_popctx();
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
	struct s_entry *ident = tos_newname($1);
	ident->type = array_type(T_BOOL, $3);
}
;

method_decl_l 
: method_decl
| method_decl_l method_decl
;

method_decl
: INT ID {
	tos_newname($2);
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} '(' arg_l_ ')' {
	struct s_entry *ident = tos_lookup($2);
	ident->type = function_type(R_INT, $5);
} block popctx {
	YCHK($8.next_break != NULL, "break doit être dans une boucle");
	YCHK($8.next_continue != NULL, "continue doit être dans une boucle");
}
| BOOL ID {
	tos_newname($2);
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} '(' arg_l_ ')' {
	struct s_entry *ident = tos_lookup($2);
	ident->type = function_type(R_BOOL, $5);
} block popctx {
	YCHK($8.next_break != NULL, "break doit être dans une boucle");
	YCHK($8.next_continue != NULL, "continue doit être dans une boucle");
}
| VOID ID {
	tos_newname($2);
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2)));
} '(' arg_l_ ')' {
	struct s_entry *ident = tos_lookup($2);
	ident->type = function_type(R_VOID, $5);
} block popctx {
	YCHK($8.next_break != NULL, "break doit être dans une boucle");
	YCHK($8.next_continue != NULL, "continue doit être dans une boucle");
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
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la variable n'existe pas");
	quadop id = quadop_name($1);
	if (ident->type->type == T_INT) { // cas int
		YCHK($3.type != E_INT, "l'expression doit être int");
		gencode(quad_make(Q_MOVE, $3.u.result, quadop_empty(), id));
	} else { // cas bool
		YCHK($3.type != E_BOOL, "l'expression doit être bool");
		complete($3.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), id));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($3.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), id));
	}
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' '=' expr ';' {
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la variable n'existe pas");
	// TODO: checker le tableau
	quadop id = quadop_name($1);
	if (1) { // cas int
		gencode(quad_make(Q_SETI, id, $3.u.result, $6.u.result));	
	} else { // cas bool
		complete($6.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_SETI, id, $3.u.result, quadop_bool(1)));
		complete($6.u.boolexpr.false, nextquad);
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		gencode(quad_make(Q_SETI, id, $3.u.result, quadop_bool(0)));
	}
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID ADD_ASSIGN expr ';' {
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la variable n'existe pas");
	YCHK(ident->type->type != T_INT, "la variable n'est pas de type int");
	YCHK($3.type != E_INT, "l'expression doit être int");
	quadop id = quadop_name($1);
	gencode(quad_make(Q_ADD, id, $3.u.result, id));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' ADD_ASSIGN expr ';' {
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la variable n'existe pas");
	// TODO: checker le tableau
	quadop id = quadop_name($1);
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop tmp = quadop_name(entry->ident);
	gencode(quad_make(Q_GETI, id, $3.u.result, tmp));
	gencode(quad_make(Q_ADD, tmp, $6.u.result, tmp));
	gencode(quad_make(Q_SETI, id, $3.u.result, tmp));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID SUB_ASSIGN expr ';' {
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la variable n'existe pas");
	YCHK(ident->type->type != T_INT, "la variable n'est pas de type int");
	YCHK($3.type != E_INT, "l'expression doit être int");
	quadop id = quadop_name($1);
	gencode(quad_make(Q_SUB, id, $3.u.result, id));
	$$.next = NULL;
	$$.next_break = NULL;
	$$.next_continue = NULL;
}
| ID '[' expr ']' SUB_ASSIGN expr ';' {
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la variable n'existe pas");
	// TODO: checker le tableau
	quadop id = quadop_name($1);
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop tmp = quadop_name(entry->ident);
	gencode(quad_make(Q_GETI, id, $3.u.result, tmp));
	gencode(quad_make(Q_SUB, tmp, $6.u.result, tmp));
	gencode(quad_make(Q_SETI, id, $3.u.result, tmp));
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
| FOR ID '=' expr ',' expr {
	// ! créer un nouvel identificateur !
	// ! push une nouvelle table des symboles !
	YCHK($4.type != E_INT, "l'expression doit être int");
	YCHK($6.type != E_INT, "l'expression doit être int");
	gencode(quad_make(Q_MOVE, $4.u.result, quadop_empty(), quadop_name($2)));
} marker {
	gencode(quad_make(Q_BGT, quadop_name($2), $4.u.result, quadop_empty()));
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
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la fonction n'existe pas");
	// TODO: checker la fonction
	quadop qo;
	if (1) { // procédure
		qo = quadop_empty();
	} else { // fonction
		struct s_entry *entry = newtemp(); 
		// entry->type->type = T_INT; // gérer type (int ou bool)
		qo = quadop_name(entry->ident);
	}
	gencode(quad_make(Q_CALL, quadop_name($1), quadop_empty(), qo));
	$$ = qo;
}
| ID '(' expr_l ')' {
	struct s_entry *ident = tos_lookup($1);
	YCHK(ident == NULL, "la fonction n'existe pas");
	// TODO: checker la fonction
	quadop qo;
	if (1) { // procédure
		qo = quadop_empty();
	} else { // fonction
		struct s_entry *entry = newtemp(); 
		// entry->type->type = T_INT; // gérer type (int ou bool)
		qo = quadop_name(entry->ident);
	}
	gencode(quad_make(Q_CALL, quadop_name($1), quadop_cst($3.length), qo));
	$$ = qo;
}
;

expr_l 
: expr {
	$$.length = 1;
	gencode(quad_make(Q_PARAM, $1.u.result, quadop_empty(), quadop_empty()));
}
| expr_l ',' expr {
	$$.length = $1.length + 1;
	gencode(quad_make(Q_PARAM, $3.u.result, quadop_empty(), quadop_empty()));
}
;

expr 
: ID {
	struct s_entry *entry = tos_lookup($1);
	YCHK(entry == NULL, "la variable n'existe pas");
	if (entry->type->type == T_INT) { // cas int
		$$.type = E_INT;
		$$.u.result = quadop_name($1);
	} else if (entry->type->type == T_BOOL) { // cas bool
		$$.type = E_BOOL;
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, quadop_name($1), quadop_bool(1), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else {
		raler("expression doit être int ou bool");
	}
} 
| ID '[' expr ']' {
	struct s_entry *entry = tos_lookup($1);
	YCHK(entry == NULL, "la variable n'existe pas");
	YCHK(entry->type->type != T_ARRAY, "la variable n'est pas un tableau");
	YCHK($3.type != E_INT, "index de tableau doit être int");
	struct s_entry *temp = newtemp();
	// temp->type->type = entry->type->type; TODO: gerer type (int ou bool)
	quadop qo = quadop_name(entry->ident);
	gencode(quad_make(Q_GETI, quadop_name($1), $3.u.result, qo));
	if (entry->type->type == T_INT)
		$$.type = E_INT;
	else
		$$.type = E_BOOL;
	$$.u.result = qo;
}
| method_call {
	YCHK($1.type == QO_EMPTY, "appel de procédure dans expression");
	if ($1.type == QO_CST)
		$$.type = E_INT;
	else
		$$.type = E_BOOL;
	$$.u.result = $1;
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
	if ($1)
		$$.u.boolexpr.true = crelist(nextquad);
	else
		$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '+' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop qo = quadop_name(entry->ident);
	gencode(quad_make(Q_ADD, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '-' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop qo = quadop_name(entry->ident);
	gencode(quad_make(Q_SUB, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '*' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop qo = quadop_name(entry->ident);
	gencode(quad_make(Q_MUL, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '/' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop qo = quadop_name(entry->ident);
	gencode(quad_make(Q_DIV, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '%' expr {
	YCHK($1.type != E_INT, "opérande doit être int");
	YCHK($3.type != E_INT, "opérande doit être int");
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop qo = quadop_name(entry->ident);
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
	$$.type = E_BOOL;
	if ($1.type == E_INT) {
		YCHK($3.type != E_INT, "opérande doit être int");
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, $1.u.result, $3.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));

	} else {
		YCHK($3.type != E_BOOL, "opérande doit être bool");
		// TODO
	}
}
| expr NEQ expr {
	$$.type = E_BOOL;
	if ($1.type == E_INT) {
		YCHK($3.type != E_INT, "opérande doit être int");
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BNE, $1.u.result, $3.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else {
		YCHK($3.type != E_BOOL, "opérande doit être bool");
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
	struct s_entry *entry = newtemp(); 
	entry->type->type = T_INT;
	quadop qo = quadop_name(entry->ident);
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
