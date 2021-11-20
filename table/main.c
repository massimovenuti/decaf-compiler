# include "table.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    int test = atoi(argv[1]);
    table = new_tos();

    switch (test)
    {
    case 1: // 1 variable, 1 scope
        table = push_tos(table);
        table = tos_newname(table, "compteur", S_INT, 0);
        print_tos(table);
        table = pop_tos(table);
        break;
    case 2: // variable redefinition, 1 scope
        table = push_tos(table);
        table = tos_newname(table, "compteur", S_INT, 0);
        print_tos(table);
        table = tos_newname(table, "compteur", S_INT, 0);
        // error expected here ...
        break;
    case 3: // sereval variables, 2 scope
        table = push_tos(table);
        table = tos_newname(table, "compteur", S_INT, 0);
        table = tos_newname(table, "diviseur", S_INT, 0);
        table = tos_newname(table, "test", S_BOOL, 0);
        print_tos(table);
        printf("\n");
        table = push_tos(table);
        table = tos_newname(table, "test", S_BOOL, 0);
        table = tos_newname(table, "symbole", S_INT, 0);
        print_tos(table);
        table = pop_tos(table);
        table = pop_tos(table);
        break;
    case 4: // sereval variables, 2 scope with redefinition
        table = push_tos(table);
        table = tos_newname(table, "compteur", S_INT, 0);
        table = tos_newname(table, "diviseur", S_INT, 0);
        table = tos_newname(table, "test", S_BOOL, 0);
        print_tos(table);
        printf("\n");
        table = push_tos(table);
        table = tos_newname(table, "test", S_BOOL, 0);
        table = tos_newname(table, "nombre", S_INT, 0);
        print_tos(table);
        table = tos_newname(table, "test", S_BOOL, 0);
        // error expected here ...
        break;
    default:
        printf("unknown test\n");
        break;
    }

    return EXIT_SUCCESS;
}
