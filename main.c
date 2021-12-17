#include "quad.h"
#include "table.h"
#include "quad2mips.h"

extern int yyparse();
extern int yydebug;

int main(int argc, char const *argv[]) {
    (void)argc;
    FILE*input = freopen(argv[1], "r", stdin);
    initcode();
    int res = yyparse();
    print_globalcode();
    gen_mips(globalcode, nextquad, stderr);
    freecode();
    fclose(input);
    return res;
}
