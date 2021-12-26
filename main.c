#include "quad.h"
#include "table.h"
#include "quad2mips.h"
#include <getopt.h>

extern int yyparse();
extern int yydebug;

int main(int argc, char const *argv[])
{
    char *outname = "a.mips";
    const char *inname = NULL;

    int c;
    while (1)
    {
        c = getopt(argc, (char *const *)argv, "o:");
        if (c == -1)
            break;

        switch (c)
        {

        case 'o':
            outname = optarg;
            break;

        default:
            fprintf(stderr, "Error option\n");
        }
    }

    for (int i = optind; i < argc; i++)
    {
        if (inname == NULL)
        {
            inname = argv[i];
        }
        else
        {
            fprintf(stderr, "Error input\n");
        }
    }

    FILE *input = freopen(inname, "r", stdin);
    initcode();
    int res = yyparse();
    // print_globalcode();
    FILE *output = fopen(outname, "w");
    gen_mips(globalcode, nextquad, output);
    freecode();
    fclose(output);
    fclose(input);
    return res;
}
