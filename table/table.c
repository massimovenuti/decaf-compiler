# include "table.h"

Symbol new_symbol()
{
    return NULL;
}

Symbol add_symbol(Symbol sym, const char *ident)
{
    Symbol new_sym = (Symbol)malloc(sizeof(struct s_symbol));
    new_sym->ident = strdup(ident);
    new_sym->next = sym;
    return new_sym;
}

Symbol search_symbol(Symbol sym, const char *ident)
{
    if (sym == NULL) return NULL;
    if (strcmp(ident, sym->ident) == 0) return sym;
    return search_symbol(sym->next, ident);
}

void destroy_symbol(Symbol sym)
{
    if (sym == NULL) return;
    destroy_symbol(sym->next);
    free(sym->ident);
    free(sym);
}

void print_symbol(Symbol sym)
{
    Symbol tmp = sym;

    while (tmp != NULL)
    {
        printf("%s\t", tmp->ident);
        tmp = tmp->next;
    }
    printf("\n");
}

Table new_table()
{
    return NULL;
}

Table add_table(Table prev_scope)
{
    Table new_scope = (Table)malloc(sizeof(struct s_table));
    new_scope->next = prev_scope;

    for (int i = 0; i < HASH_SIZE; i++)
        new_scope->entry[i] = new_symbol();

    return new_scope;
}

int hash_idx(const char *str)
{
	int i, res;
    i = 0;
    res = 0;

	while (str[i] != '\0')
    {
		res += ('a' - str[i] + 1) * (i + 1);
		i++;
	}
	return abs(res) % HASH_SIZE;
}

Table add_entry(Table curr_scope, const char *ident)
{
    int idx = hash_idx(ident);
    //printf("idx:%d\n", idx);
    curr_scope->entry[idx] = add_symbol(curr_scope->entry[idx], ident);
    return curr_scope;
}

Table pop_table(Table curr_scope)
{
    for (int i = 0; i < HASH_SIZE; i++)
        destroy_symbol(curr_scope->entry[i]);

    Table new_scope = curr_scope->next;
    free(curr_scope);
    return new_scope;
}

int get_table_scope(Table curr_scope)
{
    if (curr_scope == NULL) return -1;
    return 1 + get_table_scope(curr_scope->next);
}

int search_entry_scope(Table curr_scope, const char *ident)
{
    int idx = hash_idx(ident);
    
    Table scope = curr_scope;

    while (scope != NULL)
    {
        if (search_symbol(scope->entry[idx], ident) != NULL)
            return get_table_scope(scope);
        
        scope = scope->next;
    }
    return -1;
}

Symbol search_entry(Table curr_scope, const char *ident)
{
    int idx = hash_idx(ident);
    
    Table scope = curr_scope;
    Symbol ret = NULL;

    while (scope != NULL)
    {
        if ((ret = search_symbol(scope->entry[idx], ident)) != NULL)
            return ret;
        
        scope = scope->next;
    }
    return NULL;
}

void print_table(Table curr_scope)
{
    int scope_idx, nb_scope;
    scope_idx = 0;
    nb_scope = get_table_scope(curr_scope);

    Table scope = curr_scope;

    while (scope != NULL)
    {
        printf("Scope %d :\n", nb_scope - scope_idx);

        for (int i = 0; i < HASH_SIZE; i++)
        {
            if (scope->entry[i] != NULL)
            {
                printf("\t");
                print_symbol(scope->entry[i]);
            }
        }

        scope = scope->next;
        scope_idx++;
    }
}
