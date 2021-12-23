%{
#include <stdio.h>
#include <string.h>
#include "quad.h"

#define YY_USER_ACTION                                       \
  yylloc.first_line = yylloc.last_line;                      \
  yylloc.first_column = yylloc.last_column;                  \
  if (yylloc.first_line == yylineno)                         \
     yylloc.last_column += yyleng;                           \
  else {                                                     \
     int col;                                                \
     for (col = 1; yytext[yyleng - col] != '\n'; ++col) {}   \
     yylloc.last_column = col;                               \
     yylloc.last_line = yylineno;                            \
  }

// exemple d'utilisation:
// {printf("col: %d\n", @1.first_column);}

#include "decaf.tab.h"
%}

%option nounput noyywrap yylineno

ID [[:alpha:]_][[:alnum:]_]*

SPECIAL_CHAR \\\"|\\'|\\|"\t"|"\n"
CHAR {SPECIAL_CHAR}|[ !#-&(-\[\]-~]

NUM [[:digit:]][[:digit:]]* 
HEX_NUM 0x[0-9a-fA-F]+ 

%%

"boolean"					{return BOOL;}
"break"						{return BREAK;}
"class"						{return CLASS;}
"continue"					{return CONTINUE;}
"else"						{return ELSE;}
"true"						{yylval.intval = 1; return BOOL_LITERAL;}
"false"						{yylval.intval = 0; return BOOL_LITERAL;}
"for"						{return FOR;}
"if"						{return IF;}
"int"						{return INT;}
"return"					{return RETURN;}
"void"						{return VOID;}

"<="						{return LEQ;}
">="						{return BEQ;}
"=="						{return EQ;}
"!="						{return NEQ;}
"&&"						{return AND;}
"||"						{return OR;}

"+="						{return ADD_ASSIGN;}
"-="                        {return SUB_ASSIGN;}

{ID}						{strncpy(yylval.strval, yytext, yyleng+1) ;  return ID;}

[\{\}\(\)\[\]\,\;\=\+\-\*\/\%\<\>\!]		{return yytext[0];}

"//"[^\n]*"\n"				{;}

'{CHAR}' 					{yylval.charval = yytext[1]; return CHAR_LITERAL;}
\"{CHAR}*\"					{return STRING_LITERAL;}

{NUM}						{
								if (sscanf(yytext, "%d", &yylval.intval) != 1) {
									fprintf(stderr, "%s not int\n", yytext);
									exit(EXIT_FAILURE);
								}
								return INT_LITERAL;
							}
{HEX_NUM}					{
								if (sscanf(yytext, "%x", &yylval.intval) != 1) {
									fprintf(stderr, "%s not hex\n", yytext);
									exit(EXIT_FAILURE);
								}
								return INT_LITERAL;
							}

[[:space:]]					{;}

.		{
	fprintf(stderr, "Caractère illégal (%c)\n", *yytext);
	return *yytext;
}
