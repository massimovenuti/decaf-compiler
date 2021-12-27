%{
#include <stdio.h>
#include "utils.h"
#include "quad.h"
#include "table.h"
#include "decaf.tab.h"

extern struct s_context *context;
extern struct s_stringtable *strings;

unsigned inloop = 0;
unsigned infunction = 0;
enum ret_type infunction_type;

extern int yylex();
extern int yylineno;

void yyerror(char *msg);
struct s_statement new_statement();
struct s_expr new_expr();

// int yydebug = 1; 
%}

%locations

%code requires {
#include "table.h"
}

%union {
	char charval;
	// TODO: rendre dynamique
	char strval[255];
	int intval;
	struct s_expr {
		enum elem_type type;
		union {
			quadop result;
			struct {
				ilist *true;
				ilist *false;
			} boolexpr;
		} u;
	} exprval;
	struct s_statement {
		ilist *next;
		ilist *next_break;
		ilist *next_continue;
	} stateval;
	struct s_entry *entryval;
	struct s_arglist *alistval;
	enum elem_type etypeval;
}

%type <exprval> expr
%type <intval> marker
%type <entryval> method_call
%type <stateval> statement statement_l statement_l_ goto block
%type <alistval> arg_l arg_l_ expr_l
%type <etypeval> arg

%token CLASS							// class 
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
%token <strval> STRING_LITERAL	// ...


%left OR
%left AND
%left '<' '>' LEQ BEQ EQ NEQ
%left '+' '-'
%left '*' '/' '%'
%nonassoc NOT UMINUS

%start program

%%
program
: CLASS ID '{' pushctx init decl check_main popctx '}'
;

pushctx
: %empty {
	context = tos_pushctx(context);
	gencode(quad_make(Q_BCTX, quadop_empty(), quadop_empty(), quadop_context(context)));
}
;

popctx
: %empty {
	context = tos_popctx(context);
	gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
}
;

init
: %empty {
	struct s_entry *id;
	id = tos_newname(context, "WriteInt");
	id->type = function_type(R_VOID, arglist_addbegin(NULL, E_INT));
	id = tos_newname(context, "WriteBool");
	id->type = function_type(R_VOID, arglist_addbegin(NULL, E_BOOL));
	id = tos_newname(context, "WriteString");
	id->type = function_type(R_VOID, arglist_addbegin(NULL, E_STR));
	id = tos_newname(context, "ReadInt");
	id->type = function_type(R_VOID, arglist_addbegin(NULL, E_INT));
}
;

check_main
: %empty {
    struct s_entry *id = tos_lookup(context, "main");
    ERRORIF(id == NULL, "la fonction main n'a pas été définie dans le programme");
    // vérifier si id->type == NULL ? (ne devrait pas arriver)
    ERRORIF(id->type->type != T_FUNCTION, "l'identificateur main déclaré n'est pas une fonction");
    ERRORIF(id->type->u.function_info.ret_type != R_VOID, "la fonction main doit avoir le type de retour void");
    ERRORIF(id->type->u.function_info.arglist != NULL, "la fonction main ne doit pas prendre d'arguments en paramètre");
}

decl
: field_decl_l method_decl_l
| field_decl_l
| method_decl_l
| %empty
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
	struct s_entry *ident = tos_newname(context, $1);
	ERRORIF(ident == NULL, "la variable existe déjà");
	ident->type = elementary_type(T_INT);
}
| ID '[' INT_LITERAL ']' {
	struct s_entry *ident = tos_newname(context, $1);
	ERRORIF(ident == NULL, "le tableau existe déjà");
	// INT_LITERAL est forcément un entier positif
	// ERRORIF($3 <= 0, "taille de tableau doit être supérieure à 0");
	ident->type = array_type(E_INT, $3);
}
;

field_decl_bool_l 
: field_decl_bool
| field_decl_bool_l ',' field_decl_bool
;

field_decl_bool 
: ID {
	struct s_entry *ident = tos_newname(context, $1);
	ERRORIF(ident == NULL, "la variable existe déjà");
	ident->type = elementary_type(T_BOOL);
}
| ID '[' INT_LITERAL ']' {
	struct s_entry *id = tos_newname(context, $1);
	ERRORIF(id == NULL, "le tableau existe déjà");
	// INT_LITERAL est forcément un entier positif
	// ERRORIF($3 <= 0, "taille de tableau doit être supérieure à 0");
	id->type = array_type(E_BOOL, $3);
}
;

method_decl_l 
: method_decl
| method_decl_l method_decl
;

method_decl
: INT ID {
	struct s_entry *id = tos_newname(context, $2);
	ERRORIF(id == NULL, "la fonction existe déjà");
	infunction = 1;
	infunction_type = R_INT;
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name(id->ident)));
} '(' arg_l_ ')' {
	struct s_entry *ident = tos_lookup(context, $2);
	ident->type = function_type(R_INT, $5);
} block {
	infunction = 0;
	if ($5 != NULL) {
		context = tos_popctx(context);
		gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| BOOL ID {
	struct s_entry *id = tos_newname(context, $2);
	ERRORIF(id == NULL, "la fonction existe déjà");
	infunction = 1;
	infunction_type = R_BOOL;
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name(id->ident)));
} '(' arg_l_ ')' {
	struct s_entry *id = tos_lookup(context, $2);
	id->type = function_type(R_BOOL, $5);
} block {
	infunction = 0;
	if ($5 != NULL) {
		context = tos_popctx(context);
		gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| VOID ID {
	struct s_entry *id = tos_newname(context, $2);
	ERRORIF(id == NULL, "la fonction existe déjà");
	infunction = 1;
	infunction_type = R_VOID;
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name(id->ident)));
} '(' arg_l_ ')' {
	struct s_entry *id = tos_lookup(context, $2);
	id->type = function_type(R_VOID, $5);
} block {
	infunction = 0;
	if ($5 != NULL) {
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_empty()));
		context = tos_popctx(context);
		gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
;

arg_l_
: pushctx set_ra arg_l {$$ = $3;}
| %empty {$$ = NULL;}
;

set_ra
: %empty {
	struct s_entry *ra = tos_newname(context, "_ra");
}
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
	struct s_entry *ident = tos_newname(context, $2);
	ERRORIF(ident == NULL, "argument déjà utilisé");
	ident->type = elementary_type(T_INT);
	$$ = E_INT;
}
| BOOL ID {
	struct s_entry *ident = tos_newname(context, $2);
	ERRORIF(ident == NULL, "argument déjà utilisé");
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
	struct s_entry *ident = tos_newname(context, $1);
	ERRORIF(ident == NULL, "la variable existe déjà");
	ident->type = elementary_type(T_INT);
}
| id_int_l ',' ID {
	struct s_entry *ident = tos_newname(context, $3);
	ERRORIF(ident == NULL, "la variable existe déjà");
	ident->type = elementary_type(T_INT);
}
;

id_bool_l 
: ID {
	struct s_entry *ident = tos_newname(context, $1);
	ERRORIF(ident == NULL, "la variable existe déjà");
	ident->type = elementary_type(T_BOOL);
}
| id_bool_l ',' ID {
	struct s_entry *ident = tos_newname(context, $3);
	ERRORIF(ident == NULL, "la variable existe déjà");
	ident->type = elementary_type(T_BOOL);
}
;

statement_l_
: %empty {
	$$ = new_statement();
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
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	$$ = new_statement();
	quadop qid = quadop_name(id->ident);
	if (is_elementary_type(id->type, T_INT)) { // cas int
		ERRORIF($3.type != E_INT, "expression doit être int");
		gencode(quad_make(Q_MOVE, $3.u.result, quadop_empty(), qid));
	} else if (is_elementary_type(id->type, T_BOOL)) { // cas bool
		ERRORIF($3.type != E_BOOL, "expression doit être bool");
		complete($3.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qid));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($3.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qid));
	} else {
		yyerror("variable doit être de type int ou bool");
		YYERROR;
	}
}
| ID '[' expr ']' '=' expr ';' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	ERRORIF($3.type != E_INT, "index de tableau doit être int");
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(0), quadop_label(nextquad + 2)));
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(id->type->u.array_info.size), quadop_label(nextquad + 2)));
	// TODO: faire afficher un message d'erreur
	gencode(quad_make(Q_EXIT, quadop_empty(), quadop_empty(), quadop_empty()));
	$$ = new_statement();
	quadop qid = quadop_name(id->ident);
	if (is_array_type(id->type, E_INT)) { // cas int
		ERRORIF($6.type != E_INT, "expression doit être int");
		gencode(quad_make(Q_SETI, qid, $3.u.result, $6.u.result));	
	} else { // cas bool
		ERRORIF($6.type != E_BOOL, "expression doit être bool");
		complete($6.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_SETI, qid, $3.u.result, quadop_bool(1)));
		complete($6.u.boolexpr.false, nextquad);
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		gencode(quad_make(Q_SETI, qid, $3.u.result, quadop_bool(0)));
	}
}
| ID ADD_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_INT), "la variable n'est pas de type int");
	ERRORIF($3.type != E_INT, "l'expression doit être int");
	$$ = new_statement();
	quadop qid = quadop_name(id->ident);
	gencode(quad_make(Q_ADD, qid, $3.u.result, qid));
}
| ID '[' expr ']' ADD_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	ERRORIF(!is_array_type(id->type, E_INT), "la variable n'est pas un tableau de int");
	ERRORIF($3.type != E_INT, "index de tableau doit être int");
	ERRORIF($6.type != E_INT, "l'expression doit être int");
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(0), quadop_label(nextquad + 2)));
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(id->type->u.array_info.size), quadop_label(nextquad + 2)));
	// TODO: faire afficher un message d'erreur
	gencode(quad_make(Q_EXIT, quadop_empty(), quadop_empty(), quadop_empty()));
	$$ = new_statement();
	quadop qid = quadop_name(id->ident);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, qid, $3.u.result, qo));
	gencode(quad_make(Q_ADD, qo, $6.u.result, qo));
	gencode(quad_make(Q_SETI, qid, $3.u.result, qo));
}
| ID SUB_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_INT), "la variable n'est pas de type int");
	ERRORIF($3.type != E_INT, "l'expression doit être int");
	$$ = new_statement();
	quadop qid = quadop_name(id->ident);
	gencode(quad_make(Q_SUB, qid, $3.u.result, qid));
}
| ID '[' expr ']' SUB_ASSIGN expr ';' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	ERRORIF(!is_array_type(id->type, E_INT), "la variable n'est pas un tableau de int");
	ERRORIF($3.type != E_INT, "index de tableau doit être int");
	ERRORIF($6.type != E_INT, "l'expression doit être int");
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(0), quadop_label(nextquad + 2)));
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(id->type->u.array_info.size), quadop_label(nextquad + 2)));
	// TODO: faire afficher un message d'erreur
	gencode(quad_make(Q_EXIT, quadop_empty(), quadop_empty(), quadop_empty()));
	$$ = new_statement();
	quadop qid = quadop_name(id->ident);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, qid, $3.u.result, qo));
	gencode(quad_make(Q_SUB, qo, $6.u.result, qo));
	gencode(quad_make(Q_SETI, qid, $3.u.result, qo));
}
| method_call ';' {
	$$ = new_statement();
}
| IF '(' expr ')' marker block {
	ERRORIF($3.type != E_BOOL, "l'expression doit être bool");
	$$ = new_statement();
	complete($3.u.boolexpr.true, $5);
	$$.next = concat($3.u.boolexpr.false, $6.next);
	$$.next_break = $6.next_break;
	$$.next_continue = $6.next_continue;
}
| IF '(' expr ')' marker block goto ELSE marker block {
	ERRORIF($3.type != E_BOOL, "l'expression doit être bool");
	$$ = new_statement();
	complete($3.u.boolexpr.true, $5);
	complete($3.u.boolexpr.false, $9);
	$$.next = concat(concat($6.next, $7.next), $10.next);
	$$.next_break = concat($6.next_break, $10.next_break);
	$$.next_continue = concat($6.next_continue, $10.next_continue);
}
| FOR {
	inloop = 1;
} pushctx ID '=' expr ',' expr {
	ERRORIF($6.type != E_INT, "l'expression doit être int");
	ERRORIF($8.type != E_INT, "l'expression doit être int");
	struct s_entry *tmp = tos_newtemp(context);
	tmp->type = elementary_type(T_INT);
	gencode(quad_make(Q_MOVE, $8.u.result, quadop_empty(), quadop_name(tmp->ident)));
	$8.u.result = quadop_name(tmp->ident);
	struct s_entry *id = tos_newname(context, $4);
	id->type = elementary_type(T_INT);
	gencode(quad_make(Q_MOVE, $6.u.result, quadop_empty(), quadop_name(id->ident)));
} marker {
	struct s_entry *id = tos_lookup(context, $4);
	gencode(quad_make(Q_BGT, quadop_name(id->ident), $8.u.result, quadop_empty()));
} block {
	struct s_entry *id = tos_lookup(context, $4);
	complete($12.next, nextquad);
	complete($12.next_continue, nextquad);
	quadop qid = quadop_name(id->ident);
	gencode(quad_make(Q_ADD, qid, quadop_cst(1), qid));
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($10)));
	$$ = new_statement();
	$$.next = concat(crelist($10), $12.next_break);
	inloop = 0;
}
| RETURN expr ';' {
	ERRORIF(!infunction, "return doit être appelé dans une fonction");
	$$ = new_statement();
	if ($2.type == E_INT) { // cas int
		ERRORIF(infunction_type != R_INT, "mauvais type de retour");
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), $2.u.result));
	} else { // cas bool
		ERRORIF(infunction_type != R_BOOL, "mauvais type de retour");
		complete($2.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_bool(1)));
		complete($2.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_bool(0)));
	}
}
| RETURN ';' {
	ERRORIF(!infunction, "return doit être appelé dans une fonction");
	ERRORIF(infunction_type != R_VOID, "mauvais type de retour");
	$$ = new_statement();
	gencode(quad_make(Q_RETURN, quadop_empty(), quadop_empty(), quadop_empty()));
}
| BREAK ';' {
	ERRORIF(!inloop, "break doit être appelé dans une boucle");
	$$ = new_statement();
	$$.next_break = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| CONTINUE ';' {
	ERRORIF(!inloop, "continue doit être appelé dans une boucle");
	$$ = new_statement();
	$$.next_continue = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| block {
	$$ = $1;
}
;

method_call
: ID '(' ')' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la fonction n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_FUNCTION), "la variable n'est pas une fonction");
	// TODO: checker la fonction
	quadop qo;
	if (is_function_type(id->type, R_VOID, NULL)) { // procédure
		qo = quadop_empty();
		$$ = NULL;
	} else if (is_function_type(id->type, R_INT, NULL)) { // fonction renvoyant int
		struct s_entry *temp = tos_newtemp(context); 
		temp->type = elementary_type(T_INT);
		qo = quadop_name(temp->ident);
		$$ = temp;
	} else if (is_function_type(id->type, R_BOOL, NULL)) { // fonction renvoyant bool
		struct s_entry *temp = tos_newtemp(context); 
		temp->type = elementary_type(T_BOOL);
		qo = quadop_name(temp->ident);
		$$ = temp;
	} else {
		yyerror("arguments incorrect");
		YYERROR;
	}
	gencode(quad_make(Q_CALL, quadop_name(id->ident), quadop_cst(0), qo));
	// $$ = qo;
}
| ID '(' expr_l ')' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la fonction n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_FUNCTION), "la variable n'est pas une fonction");
	// TODO: checker la fonction
	quadop qo;
	if (is_function_type(id->type, R_VOID, $3)) { // procédure
		// TODO: gérer WriteInt, WriteBool, etc...
		// TODO: checker si on appelle main ?
		qo = quadop_empty();
		$$ = NULL;
	} else if (is_function_type(id->type, R_INT, $3)) { // fonction renvoyant int
		struct s_entry *temp = tos_newtemp(context); 
		temp->type = elementary_type(T_INT);
		qo = quadop_name(temp->ident);
		$$ = temp;
	} else if (is_function_type(id->type, R_BOOL, $3)) { // fonction renvoyant bool
		struct s_entry *temp = tos_newtemp(context); 
		temp->type = elementary_type(T_BOOL);
		qo = quadop_name(temp->ident);
		$$ = temp;
	} else {
		yyerror("arguments incorrect");
		YYERROR;
	}
	gencode(quad_make(Q_CALL, quadop_name(id->ident), quadop_cst(arglist_size($3)), qo));
	// $$ = qo;
}
;

expr_l 
: expr {
	$$ = arglist_addend(NULL, $1.type);
	if ($1.type == E_INT || $1.type == E_STR) { // cas int et string
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), $1.u.result));
	} else { // cas bool
		struct s_entry *temp = tos_newtemp(context);
		temp->type = elementary_type(E_BOOL);
		quadop qtemp = quadop_name(temp->ident);
		complete($1.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qtemp));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label(nextquad + 2)));
		complete($1.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qtemp));
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), qtemp));
	}
}
| expr ',' expr_l {
	$$ = arglist_addend($3, $1.type);
	if ($1.type == E_INT || $1.type == E_STR) { // cas int et string
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), $1.u.result));
	} else { // cas bool
		struct s_entry *temp = tos_newtemp(context);
		temp->type = elementary_type(E_BOOL);
		quadop qtemp = quadop_name(temp->ident);
		complete($1.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qtemp));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label(nextquad + 2)));
		complete($1.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qtemp));
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), qtemp));
	}
}
;

expr 
: ID {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	$$ = new_expr();
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
		yyerror("expression doit être int ou bool");
		YYERROR;
	}
} 
| ID '[' expr ']' {
	struct s_entry *id = tos_lookup(context, $1);
	ERRORIF(id == NULL, "la variable n'existe pas");
	ERRORIF(!is_elementary_type(id->type, T_ARRAY), "la variable n'est pas un tableau");
	ERRORIF($3.type != E_INT, "index de tableau doit être int");
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(0), quadop_label(nextquad + 2)));
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(id->type->u.array_info.size), quadop_label(nextquad + 2)));
	// TODO: faire afficher un message d'erreur
	gencode(quad_make(Q_EXIT, quadop_empty(), quadop_empty(), quadop_empty()));
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, quadop_name(id->ident), $3.u.result, qo));
	if (is_array_type(id->type, E_INT)) {
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
	ERRORIF($1 == NULL, "appel de procédure dans expression");
	$$ = new_expr();
	if (is_elementary_type($1->type, T_INT)) { // cas int
		$$.type = E_INT;
		$$.u.result = quadop_name($1->ident);
	} else if (is_elementary_type($1->type, T_BOOL)) { // cas bool
		$$.type = E_BOOL;
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, quadop_name($1->ident), quadop_bool(1), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| INT_LITERAL {
	$$ = new_expr();
	$$.type = E_INT;
	$$.u.result = quadop_cst($1);
}
| CHAR_LITERAL {
	$$ = new_expr();
	$$.type = E_INT;
	$$.u.result = quadop_cst((int) $1);
}
| BOOL_LITERAL {
	$$ = new_expr();
	$$.type = E_BOOL;
	if ($1)
		$$.u.boolexpr.true = crelist(nextquad);
	else
		$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| STRING_LITERAL {
    $$ = new_expr();
    strings = new_string(strings, $1);
    $$.type = E_STR;
    $$.u.result = quadop_str(strings->idx);
}
| expr '+' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_ADD, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '-' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_SUB, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '*' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MUL, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '/' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_DIV, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '%' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MOD, $1.u.result, $3.u.result, qo));
	$$.type = E_INT;
	$$.u.result = qo;
}
| expr '<' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BLT, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '>' expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BGT, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr LEQ expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BLE, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr BEQ expr {
	ERRORIF($1.type != E_INT, "opérande doit être int");
	ERRORIF($3.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	$$.type = E_BOOL;
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BGE, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr EQ marker expr {
	ERRORIF($1.type != $4.type, "les opérandes doivent être de même type");
	$$ = new_expr();
	$$.type = E_BOOL;
	if ($1.type == E_INT) { // cas int
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, $1.u.result, $4.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else { // cas bool
		// left
		struct s_entry *temp_l = tos_newtemp(context);
		temp_l->type = elementary_type(E_BOOL);
		quadop qtemp_l = quadop_name(temp_l->ident);
		complete($1.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qtemp_l));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($3)));
		complete($1.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qtemp_l));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($3)));
		// right
		struct s_entry *temp_r = tos_newtemp(context);
		temp_r->type = elementary_type(E_BOOL);
		quadop qtemp_r = quadop_name(temp_r->ident);
		complete($4.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qtemp_r));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label(nextquad + 2)));
		complete($4.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qtemp_r));
		// test equality
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, qtemp_l, qtemp_r, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| expr NEQ marker expr {
	ERRORIF($1.type != $4.type, "les opérandes doivent être de même type");
	$$ = new_expr();
	$$.type = E_BOOL;
	if ($1.type == E_INT) { // cas int
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BNE, $1.u.result, $4.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else { // cas bool
		// left
		struct s_entry *temp_l = tos_newtemp(context);
		temp_l->type = elementary_type(E_BOOL);
		quadop qtemp_l = quadop_name(temp_l->ident);
		complete($1.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qtemp_l));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($3)));
		complete($1.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qtemp_l));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($3)));
		// right
		struct s_entry *temp_r = tos_newtemp(context);
		temp_r->type = elementary_type(E_BOOL);
		quadop qtemp_r = quadop_name(temp_r->ident);
		complete($4.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qtemp_r));
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label(nextquad + 2)));
		complete($4.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qtemp_r));
		// test equality
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BNE, qtemp_l, qtemp_r, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| expr AND marker expr {
	ERRORIF($1.type != E_BOOL, "opérande doit être bool");
	ERRORIF($4.type != E_BOOL, "opérande doit être bool");
	$$ = new_expr();
	$$.type = E_BOOL;
	complete($1.u.boolexpr.true, $3);
	$$.u.boolexpr.false = concat($1.u.boolexpr.false, $4.u.boolexpr.false);
	$$.u.boolexpr.true = $4.u.boolexpr.true;
}
| expr OR marker expr {
	ERRORIF($1.type != E_BOOL, "opérande doit être bool");
	ERRORIF($4.type != E_BOOL, "opérande doit être bool");
	$$ = new_expr();
	$$.type = E_BOOL;
	complete($1.u.boolexpr.false, $3);
	$$.u.boolexpr.true = concat($1.u.boolexpr.true, $4.u.boolexpr.true);
	$$.u.boolexpr.false = $4.u.boolexpr.false;
}
| '-' expr {
	ERRORIF($2.type != E_INT, "opérande doit être int");
	$$ = new_expr();
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MINUS, $2.u.result, quadop_empty(), qo));
	$$.type = E_INT;
	$$.u.result = qo;
} %prec UMINUS
| '!' expr %prec NOT {
	ERRORIF($2.type != E_BOOL, "opérande doit être bool");
	$$ = new_expr();
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
	fprintf(stderr,"Line %d: error: %s\n", yylineno, msg);
}

struct s_statement new_statement() {
	return (struct s_statement) {
		.next = NULL,
		.next_break = NULL,
		.next_continue = NULL
	};
}

struct s_expr new_expr() {
	return (struct s_expr) {
		.u.boolexpr = {
			.true = NULL,
			.false = NULL
		}
	};
}
