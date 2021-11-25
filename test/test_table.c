# include "../table.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct s_context *table = NULL;
    struct s_entry *e1, *e2, *e3;
    
    int errors = 0;
    freopen("/dev/null", "w", stderr); // disabling stderr
    
    // test 1 : 1 variable
    table = tos_pushctx(table);

    errors += (tos_newname(table, "var1") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var1") == NULL) ? 1 : 0;

    table = tos_popctx(table);

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

    // test 2 : 10 variables
    table = tos_pushctx(table);

    errors += (tos_newname(table, "var1") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var2") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var3") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var4") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var5") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var6") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var7") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var8") == NULL) ? 1 : 0;
    errors += (tos_newname(table, "var9") == NULL) ? 1 : 0;

    errors += (tos_lookup (table, "var1") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var2") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var3") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var4") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var5") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var6") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var7") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var8") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var9") == NULL) ? 1 : 0;

    table = tos_popctx(table);

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

    // test 3 : 1 variable + redefinition error
    table = tos_pushctx(table);

    errors += (tos_newname(table, "var1") == NULL) ? 1 : 0;
    errors += (tos_lookup (table, "var1") == NULL) ? 1 : 0;    

    // error expected here ...
    errors += (tos_newname(table, "var1") != NULL) ? 1 : 0;

    table = tos_popctx(table);
    
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

    // test 4 : 2 variables of the same name in 2 different contexts + redefinition error after pop
    table = tos_pushctx(table);

    e1 = tos_newname(table, "var1");
    e2 = tos_lookup (table, "var1");

    errors += (e1 != e2)? 1 : 0;

    table = tos_pushctx(table);
    
    e2 = tos_newname(table, "var1");
    e3 = tos_lookup (table, "var1");

    errors += (e2 != e3)? 1 : 0;
    errors += (e1 == e3)? 1 : 0;

    table = tos_popctx(table);

    // error expected here ...
    errors += (tos_newname(table, "var1") != NULL) ? 1 : 0;

    table = tos_popctx(table);
    
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
