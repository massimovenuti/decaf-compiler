# include "../table.h"

#define NB_NEWTEMP 100

extern struct s_context *context;

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    int i, test, errors;
    struct s_entry *e1, *e2, *e3;
    struct s_arglist *al1, *al2, *al3;

    test = 0;
    errors = 0;

    e1 = NULL;
    e2 = NULL;
    e3 = NULL;

    al1 = NULL;
    al2 = NULL;
    al3 = NULL;

    //--------------------------------------------------------------
    // TEST 1 : SINGLE VARIABLE + LOOKUP ERROR
    //--------------------------------------------------------------
    context = tos_pushctx(context);

    errors += (tos_newname(context, "var1") == NULL) ? 1 : 0;
    errors += (tos_lookup(context, "var1") == NULL) ? 1 : 0;

    // lookup error
    errors += (tos_lookup(context, "var2") != NULL) ? 1 : 0;

    context = tos_popfreectx(context);

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

    //--------------------------------------------------------------
    // TEST 2 : HASHTABLE OVERFLOW USING NEWTEMP
    //--------------------------------------------------------------
    context = tos_pushctx(context);

    for (i = 0; i < NB_NEWTEMP; i++)
    {
        e1 = tos_newtemp(context);
        // printf("%s\t", e1->ident);
        e2 = tos_lookup(context, e1->ident);
        errors += (e1 == NULL || e1 != e2) ? 1 : 0;
    }
    // printf("\n");

    context = tos_popfreectx(context);

    printf("test %d\t", ++test);
    if (context == NULL && !errors)  
    {
        printf("[ok]\t'hashtable overflow using newtemp'\n");
    }
    else
    {
        printf("[ko]\t'hashtable overflow using newtemp'\n");
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------
    // TEST 3 : REDEFINITION ERROR
    //--------------------------------------------------------------
    context = tos_pushctx(context);

    errors += (tos_newname(context, "var1") == NULL) ? 1 : 0;
    errors += (tos_lookup(context, "var1") == NULL) ? 1 : 0;    

    // newname error
    errors += (tos_newname(context, "var1") != NULL) ? 1 : 0;

    context = tos_popfreectx(context);
    
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

    //--------------------------------------------------------------
    // TEST 4 : SAME VARIABLE NAME, DIFFERENT CONTEXT
    //--------------------------------------------------------------
    context = tos_pushctx(context);

    e1 = tos_newname(context, "var1");
    e2 = tos_lookup(context, "var1");

    errors += (e1 == NULL || e1 != e2)? 1 : 0;

    context = tos_pushctx(context);
    
    e2 = tos_lookup(context, "var1");

    errors += (e1 != e2)? 1 : 0;

    e2 = tos_newname(context, "var1");
    e3 = tos_lookup(context, "var1");

    errors += (e2 != e3)? 1 : 0;
    errors += (e1 == e3)? 1 : 0;

    context = tos_popfreectx(context);

    // newname error
    errors += (tos_newname(context, "var1") != NULL) ? 1 : 0;

    context = tos_popfreectx(context);
    
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

    //--------------------------------------------------------------
    // TEST 5 : ELEMENTARY TYPE CONSTRUCTOR
    //--------------------------------------------------------------
    context = tos_pushctx(context);

    e1 = tos_newname(context, "var1");
    e1->type = elementary_type(T_BOOL);
    
    errors += 1 - is_elementary_type(e1->type, T_BOOL);
    errors += is_elementary_type(e1->type, T_INT);

    e3 = tos_lookup(context, "var1");
    errors += 1 - is_elementary_type(e3->type, T_BOOL);
    errors += is_elementary_type(e3->type, T_INT);

    e2 = tos_newname(context, "var2");
    e2->type = elementary_type(T_INT);
    
    errors += is_elementary_type(e2->type, T_BOOL);
    errors += 1 - is_elementary_type(e2->type, T_INT);

    e3 = tos_lookup(context, "var2");
    errors += is_elementary_type(e3->type, T_BOOL);
    errors += 1 - is_elementary_type(e3->type, T_INT);

    context = tos_popfreectx(context);
    
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

    //--------------------------------------------------------------
    // TEST 6 : ARRAY TYPE CONSTRUCTOR
    //--------------------------------------------------------------
    context = tos_pushctx(context);

    e1 = tos_newname(context, "var1");
    e1->type = array_type(E_INT, 10);
    
    errors += is_array_type(e1->type, E_BOOL);

    context = tos_popfreectx(context);

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

    //--------------------------------------------------------------
    // TEST 7 : FUNCTION TYPE CONSTRUCTOR
    //--------------------------------------------------------------

    context = tos_pushctx(context);

    e1 = tos_newname(context, "var1");

    // function : ret = T_VOID, args = [E_BOOL, E_INT, E_BOOL, E_INT, E_INT] 
    al1 = arglist_addbegin(al1, E_INT);
    al1 = arglist_addbegin(al1, E_INT);
    al1 = arglist_addbegin(al1, E_BOOL);
    al1 = arglist_addbegin(al1, E_INT);
    al1 = arglist_addbegin(al1, E_BOOL);

    e1->type = function_type(R_VOID, al1);

    // al2 == al1
    al2 = arglist_addend(al2, E_BOOL);
    al2 = arglist_addend(al2, E_INT);
    al2 = arglist_addend(al2, E_BOOL);
    al2 = arglist_addend(al2, E_INT);
    al2 = arglist_addend(al2, E_INT);

    errors += 1 - is_function_type(e1->type, R_VOID, al2);

    // ret error
    errors +=  is_function_type(e1->type, R_INT, al2);

    // al3 != al1
    al3 = arglist_addbegin(al3, E_BOOL);
    al3 = arglist_addbegin(al3, E_INT);

    errors += is_function_type(e1->type, R_VOID, al3);

    context = tos_popfreectx(context);

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
