#include "quad.h"

extern int yyparse();
extern int yydebug;

int main(void)
{
    return yyparse();
}
