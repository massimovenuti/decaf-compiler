#include "quad.h"
#include "table.h"

extern int yyparse();
extern int yydebug;

int main(void) {
    initcode();
    int res = yyparse();
    freecode();
    return res;
}
