#include "quad.h"
#include "table.h"
#include "quad2mips.h"
#include <getopt.h>

extern int yyparse();
extern int yydebug;
extern int print_tos;

const char *inname = NULL;

int main(int argc, char *argv[])
{
    char *outname = "a.mips";

    int c;
    int option_index = 0;
    int print_global = 0;
    
    static struct option long_options[] = {
        {"o", required_argument, 0, 0},
        {"tos", no_argument, 0, 't'},
        {"version", no_argument, 0, 'v'},
        {"quad", no_argument, 0, 'q'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long_only(argc, argv, "o:tv", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'v':
            printf("Membres du projet :\n");
            printf("Rayan LAJARGE\nAhmet Sefa ULUCAN\n");
            printf("Massimo VENUTI\nAlexandre VOGEL\n");
            return EXIT_SUCCESS;

        case 't':
            print_tos  = 1;
            break;

        case 'o':
            outname = optarg;
            break;

        case 'q':
            print_global = 1;
            break;

        default:
            break;
        }
    }

    if (optind < argc)
    {
        while (optind < argc)
            inname = argv[optind++];
    }

    FILE *input = freopen(inname, "r", stdin);
    initcode();

    int res = yyparse();
    if (res == 1)
    {
        exit(EXIT_FAILURE);
    }

    if (print_global)
    {
        print_globalcode();
    }

    FILE *output = fopen(outname, "w");
    gen_mips(globalcode, nextquad, output);
    freecode();

    fclose(output);
    fclose(input);
    return res;
}
