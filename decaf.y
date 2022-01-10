%{
#include <stdio.h>
#include <errno.h>
#include "utils.h"
#include "quad.h"
#include "table.h"
#include "decaf.tab.h"

extern struct s_context *context;
extern struct s_stringtable *strings;

// int yydebug = 1; 
struct s_fifo *inloop = NULL;
unsigned infunction = 0;
enum ret_type infunction_type;
YYLTYPE token_yylloc;

int print_tos = 0;

extern int yylex();
extern int yylineno;

extern const char *inname;

void print_line(int lineNum);
void yyerror(char *msg);
struct s_statement new_statement();
struct s_expr new_expr();
void check_expr_bool(struct s_expr expr, YYLTYPE expr_loc);
void check_expr_int(struct s_expr expr, YYLTYPE expr_loc);
void check_op(struct s_expr expr1, struct s_expr expr2, YYLTYPE expr2_loc);
void check_int_op(struct s_expr expr1, YYLTYPE expr1_loc, struct s_expr expr2, YYLTYPE expr2_loc);
void check_bool_op(struct s_expr expr1, YYLTYPE expr1_loc, struct s_expr expr2, YYLTYPE expr2_loc);
void gen_err_fun_no_return();
void gen_bool_eval(struct s_entry *result, struct s_expr expr);
void gen_loop_exit();
%}

%locations

%code requires {
#include "table.h"
}

%union {
	char charval;
	char strval[2048];
	int intval;
	struct s_expr {
		enum elem_type type;
		union {
			quadop result;
			struct {
				struct s_fifo *true;
				struct s_fifo *false;
			} boolexpr;
		} u;
	} exprval;
	struct s_statement {
		struct s_fifo *next;
		struct s_fifo *next_break;
		struct s_fifo *next_continue;
	} stateval;
	struct s_array_access {
		struct s_entry *entry;
		struct s_expr index;
	} arrayval;
	struct s_entry *entryval;
	struct s_arglist *alistval;
	enum elem_type etypeval;
}

%type <exprval> expr
%type <intval> marker
%type <entryval> method_call id_use id_decl
%type <stateval> statement statement_l statement_l_ goto block
%type <alistval> arg_l arg_l_ expr_l expr_l_
%type <etypeval> arg
%type <arrayval> array_access

%token CLASS							        // class 
%token INT BOOL						            // type
%token VOID							            // void
%token <strval> ID							    // id
%token IF ELSE						            // if else
%token FOR							            // for
%token BREAK CONTINUE				            // break continue
%token RETURN						            // return
%token ADD_ASSIGN SUB_ASSIGN		            // '+=' '-='
%token LEQ BEQ EQ NEQ				            // '<=' '>=' '==' '!='
%token AND OR						            // '&&' '||'
%token <intval> INT_LITERAL BOOL_LITERAL		// int constant
%token <charval> CHAR_LITERAL                   // char constant
%token <strval> STRING_LITERAL	                // string constant

%left OR
%left AND
%left EQ NEQ
%left '<' '>' LEQ BEQ
%left '+' '-'
%left '*' '/' '%'
%nonassoc NOT
%nonassoc UMINUS

%start program

%%
program
: pushctx init CLASS ID '{' decl '}' final popctx
;

pushctx
: %empty {
	context = tos_pushctx(context);
	gencode(quad_make(Q_BCTX, quadop_empty(), quadop_empty(), quadop_context(context)));
	if (inloop != NULL)
		inloop->num++;
	if (infunction)
		infunction++;
}
;

popctx
: %empty {
    if (print_tos)
        tos_printctx(context);
	context = tos_popctx(context);
	gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	if (inloop != NULL)
		inloop->num--;
	if (infunction)
		infunction--;
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
	id->type = function_type(R_INT, NULL);
}
;

final
: %empty {
	fifo_free(inloop);
    struct s_entry *id = tos_lookup(context, "main");
    ERRORIF(id == NULL, "missing `main` function declaration in the program");
    ERRORIF(!is_elementary_type(id->type, T_FUNCTION), "missing `main` function declaration in the program");
    ERRORIF(!check_ret_type(id->type, R_VOID), "`main` function must have void returning type");
    ERRORIF(!check_arglist(id->type, NULL), "`main` function must not have arguments");
}

id_decl
: ID {
	$$ = tos_newname(context, $1);
	token_yylloc = @1;
	ERRORIF($$ == NULL, "identifier redefinition");
}
;

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
: id_decl { $1->type = elementary_type(T_INT); }
| id_decl '[' INT_LITERAL ']' { 
	token_yylloc = @3;
	ERRORIF($3 == 0, "array size must be greater than 0");
	$1->type = array_type(E_INT, $3); 
}
;

field_decl_bool_l 
: field_decl_bool
| field_decl_bool_l ',' field_decl_bool
;

field_decl_bool 
: id_decl { $1->type = elementary_type(T_BOOL); }
| id_decl '[' INT_LITERAL ']' { 
	token_yylloc = @3;
	ERRORIF($3 == 0, "array size must be greater than 0");
	$1->type = array_type(E_BOOL, $3);
}
;

method_decl_l 
: method_decl
| method_decl_l method_decl
;

method_decl
: INT id_decl {
	infunction = 1;
	infunction_type = R_INT;
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2->ident)));
} '(' arg_l_ ')' {
	$2->type = function_type(R_INT, $5);
} block {
	infunction = 0;
	if ($5 != NULL) {
		context = tos_popctx(context);
		gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	}
	gen_err_fun_no_return();
}
| BOOL id_decl {
	infunction = 1;
	infunction_type = R_BOOL;
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2->ident)));
} '(' arg_l_ ')' {
	$2->type = function_type(R_BOOL, $5);
} block {
	infunction = 0;
	if ($5 != NULL) {
		context = tos_popctx(context);
		gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	}
	gen_err_fun_no_return();
}
| VOID id_decl {
	infunction = 1;
	infunction_type = R_VOID;
	gencode(quad_make(Q_FUN, quadop_empty(), quadop_empty(), quadop_name($2->ident)));
} '(' arg_l_ ')' {
	$2->type = function_type(R_VOID, $5);
} block {
	infunction = 0;
	gencode(quad_make(Q_DRETURN, quadop_empty(), quadop_empty(), quadop_empty()));
	if ($5 != NULL) {
		context = tos_popctx(context);
		gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
;

arg_l_
: pushctx arg_l { $$ = $2; }
| %empty { $$ = NULL; }
;

arg_l 
: arg { $$ = arglist_addend(NULL, $1); }
| arg_l ',' arg { $$ = arglist_addend($1, $3); }
;

arg 
: INT id_decl {
	$2->type = elementary_type(T_INT);
	$$ = E_INT;
}
| BOOL id_decl {
	$2->type = elementary_type(T_BOOL);
	$$ = E_BOOL;
}
;

block 
: pushctx '{' var_decl_l_ statement_l_ '}' marker popctx {
  complete($4.next, $6);  
  $$ = new_statement();
  $$.next_break = $4.next_break;
  $$.next_continue = $4.next_continue;
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
: id_decl { $1->type = elementary_type(T_INT); }
| id_int_l ',' id_decl { $3->type = elementary_type(T_INT); }
;

id_bool_l 
: id_decl { $1->type = elementary_type(T_BOOL); }
| id_bool_l ',' id_decl { $3->type = elementary_type(T_BOOL); }
;

id_use
: ID {
	$$ = tos_lookup(context, $1);
	token_yylloc = @1;
	ERRORIF($$ == NULL, "identifier undefined");
}
;

array_access
: id_use '[' expr ']' {
	token_yylloc = @1;
	ERRORIF(!is_elementary_type($1->type, T_ARRAY), "object is not an array");
	token_yylloc = @3;
	ERRORIF($3.type != E_INT, "array index must be of type integer");
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst(0), quadop_label(nextquad + 2)));
	gencode(quad_make(Q_BLT, $3.u.result, quadop_cst($1->type->u.array_info.size), quadop_label(nextquad + 5)));
	static int string_idx = -1;
	if (string_idx < 0) {
		strings = new_string(strings, "\"**** array index out of bounds\\n\"");
		string_idx = strings->idx;
	} 
	gencode(quad_make(Q_SCALL, quadop_empty(), quadop_empty(), quadop_empty()));
	gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), quadop_str(string_idx)));
	gencode(quad_make(Q_CALL, quadop_name("WriteString"), quadop_cst(1), quadop_empty()));
	gencode(quad_make(Q_EXIT, quadop_empty(), quadop_empty(), quadop_empty()));
	$$.entry = $1;
	$$.index = $3;
}
;

statement_l_
: %empty { $$ = new_statement(); }
| statement_l { $$ = $1; }
;

statement_l 
: statement { $$ = $1; }
| statement_l marker statement {
	complete($1.next, $2);
	$$.next = $3.next;
	$$.next_break = concat($1.next_break, $3.next_break);
	$$.next_continue = concat($1.next_continue, $3.next_continue);
}
;

statement 
: id_use '=' expr ';' {
	$$ = new_statement();
	quadop qid = quadop_name($1->ident);
	if (is_elementary_type($1->type, T_INT)) { // cas int
		check_expr_int($3, @3);
		gencode(quad_make(Q_MOVE, $3.u.result, quadop_empty(), qid));
	} else if (is_elementary_type($1->type, T_BOOL)) { // cas bool
		check_expr_bool($3, @3);
		complete($3.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qid));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($3.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qid));
	} else {
		token_yylloc = @1;
		yyerror("operand must be of type integer or boolean");
		YYERROR;
	}
}
| array_access '=' expr ';' {
	$$ = new_statement();
	quadop qid = quadop_name($1.entry->ident);
	if (is_array_type($1.entry->type, E_INT)) { // cas int
		check_expr_int($3, @3);
		gencode(quad_make(Q_SETI, qid, $1.index.u.result, $3.u.result));	
	} else { // cas bool
		check_expr_bool($3, @3);
		complete($3.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_SETI, qid, $1.index.u.result, quadop_bool(1)));
		$$.next = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
		complete($3.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_SETI, qid, $1.index.u.result, quadop_bool(0)));
	}
}
| id_use ADD_ASSIGN expr ';' {
	token_yylloc = @1;
	ERRORIF(!is_elementary_type($1->type, T_INT), "operand must be of type integer");
	check_expr_int($3, @3);
	$$ = new_statement();
	quadop qid = quadop_name($1->ident);
	gencode(quad_make(Q_ADD, qid, $3.u.result, qid));
}
| array_access ADD_ASSIGN expr ';' {
	token_yylloc = @1;
	ERRORIF(!is_array_type($1.entry->type, E_INT), "operand must be of type integer");
	check_expr_int($3, @3);
	$$ = new_statement();
	quadop qid = quadop_name($1.entry->ident);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, qid, $1.index.u.result, qo));
	gencode(quad_make(Q_ADD, qo, $3.u.result, qo));
	gencode(quad_make(Q_SETI, qid, $1.index.u.result, qo));
}
| id_use SUB_ASSIGN expr ';' {
	token_yylloc = @1;
	ERRORIF(!is_elementary_type($1->type, T_INT), "operand must be of type integer");
	check_expr_int($3, @3);
	$$ = new_statement();
	quadop qid = quadop_name($1->ident);
	gencode(quad_make(Q_SUB, qid, $3.u.result, qid));
}
| array_access SUB_ASSIGN expr ';' {
	token_yylloc = @1;
	ERRORIF(!is_array_type($1.entry->type, E_INT), "operand must be of type integer");
	check_expr_int($3, @3);
	$$ = new_statement();
	quadop qid = quadop_name($1.entry->ident);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, qid, $1.index.u.result, qo));
	gencode(quad_make(Q_SUB, qo, $3.u.result, qo));
	gencode(quad_make(Q_SETI, qid, $1.index.u.result, qo));
}
| method_call ';' { $$ = new_statement(); }
| IF '(' expr ')' marker block {
	check_expr_bool($3, @3);
	$$ = new_statement();
	complete($3.u.boolexpr.true, $5);
	$$.next = concat($3.u.boolexpr.false, $6.next);
	$$.next_break = $6.next_break;
	$$.next_continue = $6.next_continue;
}
| IF '(' expr ')' marker block goto ELSE marker block {
	check_expr_bool($3, @3);
	$$ = new_statement();
	complete($3.u.boolexpr.true, $5);
	complete($3.u.boolexpr.false, $9);
	$$.next = concat(concat($6.next, $7.next), $10.next);
	$$.next_break = concat($6.next_break, $10.next_break);
	$$.next_continue = concat($6.next_continue, $10.next_continue);
}
| FOR pushctx ID '=' expr ',' expr {
	check_expr_int($5, @5);
	check_expr_int($7, @7);
	inloop = fifo_push(inloop, 0);
	struct s_entry *tmp = tos_newtemp(context);
	tmp->type = elementary_type(T_INT);
	gencode(quad_make(Q_MOVE, $7.u.result, quadop_empty(), quadop_name(tmp->ident)));
	$7.u.result = quadop_name(tmp->ident);
	struct s_entry *id = tos_newname(context, $3);
	id->type = elementary_type(T_INT);
	gencode(quad_make(Q_MOVE, $5.u.result, quadop_empty(), quadop_name(id->ident)));
	$<entryval>$ = id;
} marker {
	gencode(quad_make(Q_BGT, quadop_name($<entryval>8->ident), $7.u.result, quadop_empty()));
} block {
	complete($11.next, nextquad);
	complete($11.next_continue, nextquad);
	quadop qid = quadop_name($<entryval>8->ident);
	gencode(quad_make(Q_ADD, qid, quadop_cst(1), qid));
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($9)));
	$$ = new_statement();
	inloop = fifo_pop(inloop);
	complete(crelist($9), nextquad);
	complete($11.next_break, nextquad);
	context = tos_popctx(context);
	gencode(quad_make(Q_ECTX, quadop_empty(), quadop_empty(), quadop_empty()));
	if (infunction)
		infunction--;
	if (inloop != NULL)
		inloop->num--;
}
| RETURN expr ';' {
	token_yylloc = @1;
	ERRORIF(!infunction, "return statement must be called in a function");
	$$ = new_statement();
	token_yylloc = @2;
	if ($2.type == E_INT) { // cas int
		ERRORIF(infunction_type != R_INT, "wrong returning type");
		gencode(quad_make(Q_RETURN, quadop_context(context), quadop_cst(infunction - 2), $2.u.result));
	} else { // cas bool
		ERRORIF(infunction_type != R_BOOL, "wrong returning type");
		complete($2.u.boolexpr.true, nextquad);
		gencode(quad_make(Q_RETURN, quadop_context(context), quadop_cst(infunction - 2), quadop_bool(1)));
		complete($2.u.boolexpr.false, nextquad);
		gencode(quad_make(Q_RETURN, quadop_context(context), quadop_cst(infunction - 2), quadop_bool(0)));
	}
}
| RETURN ';' {
	token_yylloc = @1;
	ERRORIF(!infunction, "return statement must be called in a function");
	ERRORIF(infunction_type != R_VOID, "wrong returning type");
	$$ = new_statement();
	gencode(quad_make(Q_RETURN, quadop_context(context), quadop_cst(infunction - 2), quadop_empty()));
}
| BREAK ';' {
	token_yylloc = @1;
	ERRORIF(inloop == NULL, "break statement must be called in a function");
	gen_loop_exit();
	$$ = new_statement();
	$$.next_break = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| CONTINUE ';' {
	token_yylloc = @1;
	ERRORIF(inloop == NULL, "continue statement must be called in a function");
	gen_loop_exit();
	$$ = new_statement();
	$$.next_continue = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| block { $$ = $1; }
;


method_call
: id_use s_call '(' expr_l_ ')' {
	token_yylloc = @1;
	ERRORIF(!is_elementary_type($1->type, T_FUNCTION), "called object is not a function");
	token_yylloc = @4;
	ERRORIF(!check_arglist($1->type, $4), "wrong arguments in function call");
	quadop qo;
	if (check_ret_type($1->type, R_VOID)) { // procédure
		qo = quadop_empty();
		$$ = NULL;
	} else if (check_ret_type($1->type, R_INT)) { // fonction renvoyant int
		struct s_entry *temp = tos_newtemp(context); 
		temp->type = elementary_type(T_INT);
		qo = quadop_name(temp->ident);
		$$ = temp;
	} else { // fonction renvoyant bool
		struct s_entry *temp = tos_newtemp(context); 
		temp->type = elementary_type(T_BOOL);
		qo = quadop_name(temp->ident);
		$$ = temp;
	}
	gencode(quad_make(Q_CALL, quadop_name($1->ident), quadop_cst(arglist_size($4)), qo));
	free_arglist($4);
}
;

s_call
: %empty {
	gencode(quad_make(Q_SCALL, quadop_empty(), quadop_empty(), quadop_empty()));
}
;

expr_l_
: expr_l { $$ = $1; }
| %empty { $$ = NULL; }
;

expr_l 
: expr {
	$$ = arglist_addend(NULL, $1.type);
	if ($1.type == E_INT || $1.type == E_STR) { // cas int et string
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), $1.u.result));
	} else { // cas bool
		struct s_entry *temp = tos_newtemp(context);
		gen_bool_eval(temp, $1);
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), quadop_name(temp->ident)));
	}
}
| expr_l ',' expr {
	$$ = arglist_addend($1, $3.type);
	if ($3.type == E_INT || $3.type == E_STR) { // cas int et string
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), $3.u.result));
	} else { // cas bool
		struct s_entry *temp = tos_newtemp(context);
		gen_bool_eval(temp, $3);
		gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), quadop_name(temp->ident)));
	}
}
;

expr 
: id_use {
	if (is_elementary_type($1->type, T_INT)) { // cas int
		$$ = new_expr(E_INT);
		$$.u.result = quadop_name($1->ident);
	} else if (is_elementary_type($1->type, T_BOOL)) { // cas bool
		$$ = new_expr(E_BOOL);
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, quadop_name($1->ident), quadop_bool(1), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else {
		yyerror("expression must be of type integer or boolean");
		YYERROR;
	}
} 
| array_access {
	struct s_entry *temp = tos_newtemp(context);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_GETI, quadop_name($1.entry->ident), $1.index.u.result, qo));
	if (is_array_type($1.entry->type, E_INT)) {
		$$ = new_expr(E_INT);
		temp->type = elementary_type(T_INT);
		$$.u.result = qo;
	}
	else {
		$$ = new_expr(E_BOOL);
		temp->type = elementary_type(T_BOOL);
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, qo, quadop_bool(1), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, qo, quadop_empty(), quadop_empty()));
	}
}
| method_call {
	token_yylloc = @1;
	ERRORIF($1 == NULL, "using void return value");
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
	$$ = new_expr(E_INT);
	$$.u.result = quadop_cst($1);
}
| CHAR_LITERAL {
	$$ = new_expr(E_INT);
	$$.u.result = quadop_cst((int) $1);
}
| BOOL_LITERAL {
	$$ = new_expr(E_BOOL);
	if ($1)
		$$.u.boolexpr.true = crelist(nextquad);
	else
		$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| STRING_LITERAL {
    $$ = new_expr(E_STR);
    strings = new_string(strings, $1);
    $$.u.result = quadop_str(strings->idx);
}
| expr '+' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_INT);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_ADD, $1.u.result, $3.u.result, qo));
	$$.u.result = qo;
}
| expr '-' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_INT);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_SUB, $1.u.result, $3.u.result, qo));
	$$.u.result = qo;
}
| expr '*' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_INT);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MUL, $1.u.result, $3.u.result, qo));
	$$.u.result = qo;
}
| expr '/' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_INT);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_DIV, $1.u.result, $3.u.result, qo));
	$$.u.result = qo;
}
| expr '%' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_INT);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MOD, $1.u.result, $3.u.result, qo));
	$$.u.result = qo;
}
| expr '<' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_BOOL);
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BLT, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr '>' expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_BOOL);
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BGT, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr LEQ expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_BOOL);
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BLE, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr BEQ expr {
	check_int_op($1, @1, $3, @3);
	$$ = new_expr(E_BOOL);
	$$.u.boolexpr.true = crelist(nextquad);
	gencode(quad_make(Q_BGE, $1.u.result, $3.u.result, quadop_empty()));
	$$.u.boolexpr.false = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
| expr EQ marker expr {
	check_op($1, $4, @4);
	$$ = new_expr(E_BOOL);
	if ($1.type == E_INT) { // cas int
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, $1.u.result, $4.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else { // cas bool
		struct s_entry *temp_l = tos_newtemp(context);
		gen_bool_eval(temp_l, $1);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($3)));
		struct s_entry *temp_r = tos_newtemp(context);
		gen_bool_eval(temp_r, $4);
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BEQ, quadop_name(temp_l->ident), quadop_name(temp_r->ident), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| expr NEQ marker expr {
	check_op($1, $4, @4);
	$$ = new_expr(E_BOOL);
	if ($1.type == E_INT) { // cas int
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BNE, $1.u.result, $4.u.result, quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	} else { // cas bool
		struct s_entry *temp_l = tos_newtemp(context);
		gen_bool_eval(temp_l, $1);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label($3)));
		struct s_entry *temp_r = tos_newtemp(context);
		gen_bool_eval(temp_r, $4);
		$$.u.boolexpr.true = crelist(nextquad);
		gencode(quad_make(Q_BNE, quadop_name(temp_l->ident), quadop_name(temp_r->ident), quadop_empty()));
		$$.u.boolexpr.false = crelist(nextquad);
		gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
	}
}
| expr AND marker expr {
	check_bool_op($1, @1, $4, @4);
	$$ = new_expr(E_BOOL);
	complete($1.u.boolexpr.true, $3);
	$$.u.boolexpr.false = concat($1.u.boolexpr.false, $4.u.boolexpr.false);
	$$.u.boolexpr.true = $4.u.boolexpr.true;
}
| expr OR marker expr {
	check_bool_op($1, @1, $4, @4);
	$$ = new_expr(E_BOOL);
	complete($1.u.boolexpr.false, $3);
	$$.u.boolexpr.true = concat($1.u.boolexpr.true, $4.u.boolexpr.true);
	$$.u.boolexpr.false = $4.u.boolexpr.false;
}
| '-' expr {
	check_expr_int($2, @2);
	$$ = new_expr(E_INT);
	struct s_entry *temp = tos_newtemp(context); 
	temp->type = elementary_type(T_INT);
	quadop qo = quadop_name(temp->ident);
	gencode(quad_make(Q_MINUS, $2.u.result, quadop_empty(), qo));
	$$.u.result = qo;
} %prec UMINUS
| '!' expr %prec NOT {
	check_expr_bool($2, @2);
	$$ = new_expr(E_BOOL);
	$$.u.boolexpr.true = $2.u.boolexpr.false;
	$$.u.boolexpr.false = $2.u.boolexpr.true;
}
| '(' expr ')' { $$ = $2; }
;

marker
: %empty { $$ = nextquad; }
;

goto
: %empty {
	$$.next = crelist(nextquad);
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()));
}
;

%%

void print_line(int lineNum) {
	int count = 1;
	char lineStr[1000]; /* or other suitable maximum line size */
    FILE *input = freopen(inname, "r", stdin);
	while (!feof(stdin)) {
		fgets(lineStr, 1000, stdin);
		if (ferror(stdin)) {
            fprintf(stderr, "Reading error with code %d\n", errno);
            break;
        } else if (count == lineNum) { 
			for (int i = 0; i < snprintf(NULL, 0, "%s", lineStr); i++) {
				if (i == token_yylloc.first_column - 1)
					fprintf(stderr, RED);
				else if (i == token_yylloc.last_column - 1)
					fprintf(stderr, RESET);
				fprintf(stderr, "%c", lineStr[i]);
			}
			break;
		} else {   
			count++;
		}   
	}
	fclose(input);
}

void yyerror(char *msg) {
	fprintf(stderr,"%d:%d: " RED "error:" RESET " %s\n", token_yylloc.first_line, token_yylloc.first_column, msg);
	fprintf(stderr, "   %d | ", token_yylloc.first_line);
	print_line(token_yylloc.first_line);
	fprintf(stderr, "     |%*s " RED "^", token_yylloc.first_column - 1, "");
	for (int i = 0; i < token_yylloc.last_column - token_yylloc.first_column - 1; i++)
		fprintf(stderr, "~");
	fprintf(stderr, RESET "\n");
}

struct s_statement new_statement() {
	return (struct s_statement) {
		.next = NULL,
		.next_break = NULL,
		.next_continue = NULL
	};
}

struct s_expr new_expr(enum elem_type type) {
	return (struct s_expr) {
		.type = type,
		.u.boolexpr = {
			.true = NULL,
			.false = NULL
		}
	};
}

void check_expr_bool(struct s_expr expr, YYLTYPE expr_loc) {
	token_yylloc = expr_loc;
	ERRORIF(expr.type != E_BOOL, "expression must be of type boolean");
}

void check_expr_int(struct s_expr expr, YYLTYPE expr_loc) {
	token_yylloc = expr_loc;
	ERRORIF(expr.type != E_INT, "expression must be of type integer");
}

void check_op(struct s_expr expr1, struct s_expr expr2, YYLTYPE expr2_loc) {
	token_yylloc = expr2_loc;
	ERRORIF(expr1.type != expr2.type, "operands must be of the same type");
}

void check_int_op(struct s_expr expr1, YYLTYPE expr1_loc, struct s_expr expr2, YYLTYPE expr2_loc) {
	check_expr_int(expr1, expr1_loc);
	check_expr_int(expr2, expr2_loc);
}

void check_bool_op(struct s_expr expr1, YYLTYPE expr1_loc, struct s_expr expr2, YYLTYPE expr2_loc) {
	check_expr_bool(expr1, expr1_loc);
	check_expr_bool(expr2, expr2_loc);
}

void gen_err_fun_no_return() {
	static int string_idx = -1;
	if (string_idx < 0) {
		strings = new_string(strings, "\"**** function declared as returning a result returns nothing\\n\"");
		string_idx = strings->idx;
	} 
	gencode(quad_make(Q_SCALL, quadop_empty(), quadop_empty(), quadop_empty()));
	gencode(quad_make(Q_PARAM, quadop_empty(), quadop_empty(), quadop_str(string_idx)));
	gencode(quad_make(Q_CALL, quadop_name("WriteString"), quadop_cst(1), quadop_empty()));
	gencode(quad_make(Q_EXIT, quadop_empty(), quadop_empty(), quadop_empty()));
}

void gen_bool_eval(struct s_entry *result, struct s_expr expr) {
	result->type = elementary_type(T_BOOL);
	quadop qresult = quadop_name(result->ident);
	complete(expr.u.boolexpr.true, nextquad);
	gencode(quad_make(Q_MOVE, quadop_bool(1), quadop_empty(), qresult));
	gencode(quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_label(nextquad + 2)));
	complete(expr.u.boolexpr.false, nextquad);
	gencode(quad_make(Q_MOVE, quadop_bool(0), quadop_empty(), qresult));
}

void gen_loop_exit() {
	struct s_context *tmp = context;
	for (int i = inloop->num; i > 0; i--) {
		gencode(quad_make(Q_PECTX, quadop_empty(), quadop_empty(), quadop_context(tmp)));
		tmp = tmp->next;
	}
}
