# include "../table.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct s_entry *e1, *e2, *e3;
    
    int errors = 0;
    freopen("/dev/null", "w", stderr); // disabling stderr
    
    // test 1 : 1 variable
    context = tos_pushctx();

    errors += (tos_newname("var1") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var1") == NULL) ? 1 : 0;

    context = tos_popctx();

    if (context == NULL && !errors)  
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
    context = tos_pushctx();

    errors += (tos_newname("var1") == NULL) ? 1 : 0;
    errors += (tos_newname("var2") == NULL) ? 1 : 0;
    errors += (tos_newname("var3") == NULL) ? 1 : 0;
    errors += (tos_newname("var4") == NULL) ? 1 : 0;
    errors += (tos_newname("var5") == NULL) ? 1 : 0;
    errors += (tos_newname("var6") == NULL) ? 1 : 0;
    errors += (tos_newname("var7") == NULL) ? 1 : 0;
    errors += (tos_newname("var8") == NULL) ? 1 : 0;
    errors += (tos_newname("var9") == NULL) ? 1 : 0;

    errors += (tos_lookup ("var1") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var2") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var3") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var4") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var5") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var6") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var7") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var8") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var9") == NULL) ? 1 : 0;

    context = tos_popctx();

    if (context == NULL && !errors)  
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
    context = tos_pushctx();

    errors += (tos_newname("var1") == NULL) ? 1 : 0;
    errors += (tos_lookup ("var1") == NULL) ? 1 : 0;    

    // error expected here ...
    errors += (tos_newname("var1") != NULL) ? 1 : 0;

    context = tos_popctx();
    
    if (context == NULL && !errors)  
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
    context = tos_pushctx();

    e1 = tos_newname("var1");
    e2 = tos_lookup ("var1");

    errors += (e1 != e2)? 1 : 0;

    context = tos_pushctx();
    
    e2 = tos_newname("var1");
    e3 = tos_lookup ("var1");

    errors += (e2 != e3)? 1 : 0;
    errors += (e1 == e3)? 1 : 0;

    context = tos_popctx();

    // error expected here ...
    errors += (tos_newname("var1") != NULL) ? 1 : 0;

    context = tos_popctx();
    
    if (context == NULL && !errors)  
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
