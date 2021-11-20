%{
#include <stdio.h>
#include "decaf.tab.h"
%}

%option nounput noyywrap

ID [[:alpha:]_][[:alnum:]_]*

SPECIAL_CHAR \\\"|\\'|\\|"\t"|"\n"
CHAR {SPECIAL_CHAR}|[ !#-&(-\[\]-~]

NUM [[:digit:]][[:digit:]]* 
HEX_NUM 0x[0-9a-fA-F]+ 

%%

"boolean"					{return BOOL;}
"break"						{return BREAK;}
"class Program"				{return CLPR;}
"continue"					{return CONTINUE;}
"else"						{return ELSE;}
"true"						{yylval=1; return BOOL_LITERAL;}
"false"						{yylval=0; return BOOL_LITERAL;}
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

{ID}						{return ID;}

[\{\}\(\)\[\]\,\;\=\+\-\*\/\%\<\>\!]		{return yytext[0];}

"//"[^\n]*"\n"				{;}

'{CHAR}' 					{yylval = yytext[1]; return CHAR_LITERAL;}
\"{CHAR}*\"					{return STRING_LITERAL;}

{NUM}						{
								if (sscanf(yytext, "%d", &yylval) != 1) {
									fprintf(stderr, "%s not int\n", yytext);
									exit(EXIT_FAILURE);
								}
								return INT_LITERAL;
							}
{HEX_NUM}					{
								if (sscanf(yytext, "%x", &yylval) != 1) {
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
