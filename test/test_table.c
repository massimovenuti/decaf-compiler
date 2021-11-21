# include "../table.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    Tos table = new_tos();
    int errors = 0;
    
    // test 1 : 1 variable, 1 scope
    table = push_tos(table);

    table = tos_newname(table, "compteur", S_INT, 0);
    errors += (tos_lookup(table, "compteur") == NULL) ? 1 : 0;

    table = pop_tos(table);

    printf("Test 1 : ");
    if (table == NULL && !errors)  
    {
        printf("[ok]\n");
    }
    else
    {
        printf("[ko]\n");
        exit(EXIT_FAILURE);
    }
    errors = 0;

    // test 2 : 1 variable with redefinition, 1 scope
    table = push_tos(table);
    
    table = tos_newname(table, "compteur", S_INT, 0);
    errors += (tos_lookup(table, "compteur") == NULL) ? 1 : 0;
    
    // error expected here ...
    printf("=> error expected here ...\n");
    table = tos_newname(table, "compteur", S_INT, 0);
    
    printf("Test 2 : ");
    if (table == NULL && !errors) 
    {
        printf("[ok]\n");
    }
    else
    {
        printf("[ko]\n");
        exit(EXIT_FAILURE);
    }
    errors = 0;

    // test 3 : several variables, 2 scopes
    table = push_tos(table);
    
    table = tos_newname(table, "compteur", S_INT, 0);
    table = tos_newname(table, "diviseur", S_INT, 0);
    table = tos_newname(table, "test", S_BOOL, 0);

    errors += (tos_lookup(table, "compteur") == NULL) ? 1 : 0;
    errors += (tos_lookup(table, "diviseur") == NULL) ? 1 : 0;
    errors += (tos_lookup(table, "test") == NULL) ? 1 : 0;
    
    table = push_tos(table);
    
    table = tos_newname(table, "test", S_BOOL, 0);
    table = tos_newname(table, "nombre", S_INT, 0);
    
    errors += (tos_lookup(table, "test") == NULL) ? 1 : 0;
    errors += (tos_lookup(table, "nombre") == NULL) ? 1 : 0;
    
    table = pop_tos(table);
    table = pop_tos(table);
    
    printf("Test 3 : ");
    if (table == NULL && !errors) 
    {
        printf("[ok]\n");
    }
    else
    {
        printf("[ko]\n");
        exit(EXIT_FAILURE);
    }
    errors = 0;

    // test 4 : several variables with redefinition, 2 scopes
    table = push_tos(table);
    
    table = tos_newname(table, "compteur", S_INT, 0);
    table = tos_newname(table, "diviseur", S_INT, 0);
    table = tos_newname(table, "test", S_BOOL, 0);

    errors += (tos_lookup(table, "compteur") == NULL) ? 1 : 0;
    errors += (tos_lookup(table, "diviseur") == NULL) ? 1 : 0;
    errors += (tos_lookup(table, "test") == NULL) ? 1 : 0;
    
    table = push_tos(table);
    
    table = tos_newname(table, "test", S_BOOL, 0);
    table = tos_newname(table, "nombre", S_INT, 0);

    errors += (tos_lookup(table, "test") == NULL) ? 1 : 0;
    errors += (tos_lookup(table, "nombre") == NULL) ? 1 : 0;
    
    // error expected here ...
    printf("=> error expected here ...\n");
    table = tos_newname(table, "test", S_BOOL, 0);
    
    printf("Test 4 : ");
    if (table == NULL && !errors) 
    {
        printf("[ok]\n");
    }
    else
    {
        printf("[ko]\n");
        exit(EXIT_FAILURE);
    }
    errors = 0;

    return EXIT_SUCCESS;
}
