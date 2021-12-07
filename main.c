#include "quad.h"
#include "table.h"

extern int yyparse();
extern int yydebug;

int main(int argc, char const *argv[]) {
    (void)argc;
    FILE*input = freopen(argv[1], "r", stdin);
    initcode();
    int res = yyparse();
    freecode();
    fclose(input);
    print_globalcode();
    return res;
}
