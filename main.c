#include "quad.h"

extern int yyparse();
extern int yydebug;

int main(void)
{
    initcode();    
    yyparse();
    print_globalcode();
    freecode();
    return 0;
}
