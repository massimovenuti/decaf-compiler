#include "quad.h"
#include "table.h"

extern int yyparse();
extern int yydebug;

int main(void) {
    initcode();
    printf("%d\n", snprintf(NULL, 0, "%d", 405));
    int res = yyparse();
    freecode();
    return res;
}
