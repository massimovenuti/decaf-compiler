# include "table.h"

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

/* -------------------------------------------------------------------------------- */

Symbol new_symbol()
{
    return NULL;
}

Symbol add_symbol(Symbol sym, const char *ident, int type, int val)
{
    Symbol new_sym = (Symbol)malloc(sizeof(struct s_symbol));
    new_sym->ident = strdup(ident);
    new_sym->next = sym;
    new_sym->type = type;
    new_sym->val = val;
    return new_sym;
}

Symbol lookup_symbol(Symbol sym, const char *ident)
{
    if (sym == NULL) return NULL;
    if (strcmp(ident, sym->ident) == 0) return sym;
    return lookup_symbol(sym->next, ident);
}

void free_symbol(Symbol sym)
{
    if (sym == NULL) return;
    free_symbol(sym->next);
    free(sym->ident);
    free(sym);
}

void print_symbol(Symbol sym)
{
    Symbol tmp = sym;

    while (tmp != NULL)
    {
        switch (tmp->type)
        {
        case S_INT:
            printf("<INT>");
            break;
        case S_BOOL:
            printf("<BOOL>");
            break;
        default:
            printf("<UNKNOWN TYPE>");
            break;
        }
        printf("\t%d\t%s\n", tmp->val, tmp->ident);

        tmp = tmp->next;
    }
}

/* -------------------------------------------------------------------------------- */

Tos new_tos()
{
    return NULL;
}

Tos push_tos(Tos prev_table)
{
    Tos new_table = (Tos)malloc(sizeof(struct s_tos));
    new_table->next = prev_table;
    new_table->scope = (prev_table == NULL) ? 0 : prev_table->scope + 1;

    for (int i = 0; i < HASH_SIZE; i++)
        new_table->entry[i] = new_symbol();

    return new_table;
}

Tos tos_newname(Tos curr_table, const char *ident, int type, int val)
{
    int idx = hash_idx(ident);
    
    if (lookup_symbol(curr_table->entry[idx], ident) != NULL)
    {
        fprintf(stderr, "Error : '%s' redefinition\n", ident);
        free_tos(curr_table);
        return NULL;
    }

    curr_table->entry[idx] = add_symbol(curr_table->entry[idx], ident, type, val);
    return curr_table;
}

Tos pop_tos(Tos curr_table)
{
    for (int i = 0; i < HASH_SIZE; i++)
        free_symbol(curr_table->entry[i]);

    Tos new_table = curr_table->next;
    free(curr_table);
    return new_table;
}

Symbol tos_lookup(Tos curr_table, const char *ident)
{
    int idx = hash_idx(ident);
    
    Tos curr_scope = curr_table;
    Symbol ret = NULL;

    while (curr_scope != NULL)
    {
        if ((ret = lookup_symbol(curr_scope->entry[idx], ident)) != NULL)
            return ret;
        
        curr_scope = curr_scope->next;
    }
    return ret;
}

void free_tos(Tos curr_table)
{
    while (curr_table != NULL)
        curr_table = pop_tos(curr_table);
}

void print_tos(Tos curr_table)
{
    Tos curr_scope = curr_table;

    while (curr_scope != NULL)
    {
        printf("---------------\n> Scope %d\n---------------\n", 
            curr_scope->scope);

        for (int i = 0; i < HASH_SIZE; i++)
        {
            if (curr_scope->entry[i] != NULL)
            {
                print_symbol(curr_scope->entry[i]);
            }
        }
        curr_scope = curr_scope->next;
    }
}
