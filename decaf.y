%{
#include <stdio.h>
extern int yylex();
void yyerror(char *msg);

// int yydebug = 1; 
%}

%token CLPR							// class Program
%token INT BOOL						// type
%token VOID							// void
%token ID							// id
%token IF ELSE						// if else
%token FOR							// for
%token BREAK CONTINUE				// break continue
%token RETURN						// return
%token ADD_ASSIGN SUB_ASSIGN		// '+=' '-='
%token LEQ BEQ EQ NEQ				// '<=' '>=' '==' '!='
%token AND OR						// '&&' '||'
%token INT_LITERAL BOOL_LITERAL		// ...
%token CHAR_LITERAL STRING_LITERAL	// ...

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
| CLPR '{' field_decl_l '}'
| CLPR '{' method_decl_l '}'
| CLPR '{' '}'
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
: INT ID '(' arg_l ')' block
| BOOL ID '(' arg_l ')' block
| VOID ID '(' arg_l ')' block
| INT ID '(' ')' block
| BOOL ID '(' ')' block
| VOID ID '(' ')' block
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
: '{' var_decl_l statement_l '}'
| '{' var_decl_l '}'
| '{' statement_l '}'
| '{' '}'
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
: statement
| statement_l statement
;

statement 
: location '=' expr ';'
| location ADD_ASSIGN expr ';'
| location SUB_ASSIGN expr ';'
| method_call ';'
| IF '(' expr ')' block
| IF '(' expr ')' block ELSE block;
| FOR ID '=' expr ',' expr block
| RETURN expr ';'
| RETURN ';'
| BREAK ';'
| CONTINUE ';'
| block
;

location 
: ID 
| ID '[' expr ']'
;

method_call 
: ID '(' ')'
| ID '(' expr_l ')'
;

expr_l 
: expr
| expr_l ',' expr
;

expr 
: location
| method_call
| INT_LITERAL		//{printf("INT %d\n", $1);}
| CHAR_LITERAL		//{printf("CHAR %c\n", $1);}
| BOOL_LITERAL		//{printf("BOOL %d\n", $1);}
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| expr '%' expr
| expr '<' expr
| expr '>' expr
| expr LEQ expr
| expr BEQ expr
| expr EQ expr
| expr NEQ expr
| expr AND expr
| expr OR expr
| '-' expr			%prec UMINUS
| '!' expr			%prec NOT
| '(' expr ')'
;

%%

void yyerror(char *msg)
{
	printf("Error: %s %d\n", msg);
}
