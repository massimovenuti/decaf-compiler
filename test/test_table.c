# include "../table.h"

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    struct s_entry *e1, *e2, *e3;
    struct s_arglist *al1, *al2, *al3;

    char var[5] = "var";
    int i, j, test, errors;

    test = 0;
    errors = 0;

    // TEST 1 : SINGLE VARIABLE + LOOKUP ERROR

    context = tos_pushctx();

    errors += (tos_newname("var1") == NULL) ? 1 : 0;
    errors += (tos_lookup("var1") == NULL) ? 1 : 0;

    // error expected here ...
    errors += (tos_lookup("var2") != NULL) ? 1 : 0;

    context = tos_popctx();

    printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'single declaration + lookup error'\n");
    }
    else
    {
        printf("[ko]\t'single declaration + lookup error'\n");
        exit(EXIT_FAILURE);
    }

    // TEST 2 : HASHTABLE OVERFLOW

    context = tos_pushctx();

    for (i = 0; i < 10; i++)
    {
        var[3] = i + '0';
        for (j = 0; j < 10; j++)
        {
            var[4] = j + '0';
            e1 = tos_newname(var);
            e2 = tos_lookup(var);
            errors += (e1 == NULL || e1 != e2) ? 1 : 0;
        }
    }

    context = tos_popctx();

    printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'hashtable overflow'\n");
    }
    else
    {
        printf("[ko]\t'hashtable overflow'\n");
        exit(EXIT_FAILURE);
    }

    // TEST 3 : REDEFINITION ERROR

    context = tos_pushctx();

    errors += (tos_newname("var1") == NULL) ? 1 : 0;
    errors += (tos_lookup("var1") == NULL) ? 1 : 0;    

    // error expected here ...
    errors += (tos_newname("var1") != NULL) ? 1 : 0;

    context = tos_popctx();
    
        printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'redefinition error'\n");
    }
    else
    {
        printf("[ko]\t'redefinition error'\n");
        exit(EXIT_FAILURE);
    }

    // TEST 4 : SAME VARIABLE NAME, DIFFERENT CONTEXT

    context = tos_pushctx();

    e1 = tos_newname("var1");
    e2 = tos_lookup("var1");

    errors += (e1 == NULL || e1 != e2)? 1 : 0;

    context = tos_pushctx();
    
    e2 = tos_lookup("var1");

    errors += (e1 != e2)? 1 : 0;

    e2 = tos_newname("var1");
    e3 = tos_lookup("var1");

    errors += (e2 != e3)? 1 : 0;
    errors += (e1 == e3)? 1 : 0;

    context = tos_popctx();

    // error expected here ...
    errors += (tos_newname("var1") != NULL) ? 1 : 0;

    context = tos_popctx();
    
    printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'same variable name, different context'\n");
    }
    else
    {
        printf("[ko]\t'same variable name, different context'\n");
        exit(EXIT_FAILURE);
    }

    // TEST 5 : ELEMENTARY TYPE CONSTRUCTOR

    context = tos_pushctx();

    e1 = tos_newname("var1");
    
    e1->type = elementary_type(T_BOOL);
    
    errors += 1 - is_elementary_type(e1->type, T_BOOL);
    errors += is_elementary_type(e1->type, T_INT);

    e3 = tos_lookup("var1");
    errors += 1 - is_elementary_type(e3->type, T_BOOL);
    errors += is_elementary_type(e3->type, T_INT);

    e2 = tos_newname("var2");
    
    e2->type = elementary_type(T_INT);
    
    errors += is_elementary_type(e2->type, T_BOOL);
    errors += 1 - is_elementary_type(e2->type, T_INT);

    e3 = tos_lookup("var2");
    errors += is_elementary_type(e3->type, T_BOOL);
    errors += 1 - is_elementary_type(e3->type, T_INT);

    context = tos_popctx();
    
        printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'elementary type constructor'\n");
    }
    else
    {
        printf("[ko]\t'elementary type constructor'\n");
        exit(EXIT_FAILURE);
    }

    // TEST 6 : ARRAY TYPE CONSTRUCTOR
    
    context = tos_pushctx();

    e1 = tos_newname("var1");
    
    e1->type = array_type(L_INT, 10);

    for (i = 0; i < 10; i++)
        errors += 1 - is_array_type(e1->type, L_INT, i);
    
    errors += is_array_type(e1->type, L_BOOL, 0);
    errors += is_array_type(e1->type, L_INT, -1);
    errors += is_array_type(e1->type, L_INT, 10);

    context = tos_popctx();

    printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'array type constructor'\n");
    }
    else
    {
        printf("[ko]\t'array type constructor'\n");
        exit(EXIT_FAILURE);
    }

    // TEST 7 : FUNCTION TYPE CONSTRUCTOR
    
    al1 = NULL;
    al2 = NULL;
    al3 = NULL;

    context = tos_pushctx();

    e1 = tos_newname("var1");

    // function : ret = T_VOID, args = [L_BOOL, L_INT, L_BOOL, L_INT, L_INT] 
    al1 = arglist_addbegin(al1, L_INT);
    al1 = arglist_addbegin(al1, L_INT);
    al1 = arglist_addbegin(al1, L_BOOL);
    al1 = arglist_addbegin(al1, L_INT);
    al1 = arglist_addbegin(al1, L_BOOL);

    e1->type = function_type(R_VOID, al1);

    // al2 == al1
    al2 = arglist_addend(al2, L_BOOL);
    al2 = arglist_addend(al2, L_INT);
    al2 = arglist_addend(al2, L_BOOL);
    al2 = arglist_addend(al2, L_INT);
    al2 = arglist_addend(al2, L_INT);

    errors += 1 - is_function_type(e1->type, R_VOID, al2);

    // ret error
    errors +=  is_function_type(e1->type, R_INT, al2);

    // al3 != al1
    al3 = arglist_addbegin(al3, L_BOOL);
    al3 = arglist_addbegin(al3, L_INT);

    errors += is_function_type(e1->type, R_VOID, al3);

    context = tos_popctx();

    free_arglist(al2);
    free_arglist(al3);

    printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'function type constructor'\n");
    }
    else
    {
        printf("[ko]\t'function type constructor'\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
