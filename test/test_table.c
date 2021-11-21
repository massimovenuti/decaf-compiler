# include "../table.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    Tos table = new_tos();
    
    int test = atoi(argv[1]);
    switch (test)
    {
    case 1: // 1 variable, 1 scope
        table = push_tos(table);
        
        table = tos_newname(table, "compteur", S_INT, 0);
        print_tos(table);
        
        table = pop_tos(table);
        printf("\n");

        if (table == NULL) 
        {
            printf("[ok]\n");
        }
        else
        {
            printf("[ko]\n");
            exit(EXIT_FAILURE);
        }
        break;
    case 2: // variable redefinition, 1 scope
        table = push_tos(table);
        
        table = tos_newname(table, "compteur", S_INT, 0);
        print_tos(table);
        printf("\n");
        
        table = tos_newname(table, "compteur", S_INT, 0);
        // error expected here ...
        
        if (table == NULL) 
        {
            printf("[ok]\n");
        }
        else
        {
            printf("[ko]\n");
            exit(EXIT_FAILURE);
        }
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
        printf("\n");
        
        table = pop_tos(table);
        table = pop_tos(table);

        if (table == NULL) 
        {
            printf("[ok]\n");
        }
        else
        {
            printf("[ko]\n");
            exit(EXIT_FAILURE);
        }
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
        printf("\n");
        
        table = tos_newname(table, "test", S_BOOL, 0);
        // error expected here ...

        if (table == NULL) 
        {
            printf("[ok]\n");
        }
        else
        {
            printf("[ko]\n");
            exit(EXIT_FAILURE);
        }
        break;
    default:
        printf("unknown test\n");
        break;
    }

    return EXIT_SUCCESS;
}
