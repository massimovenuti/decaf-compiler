# include "table.h"

struct s_context *context = NULL;
unsigned tempnum = 0;

unsigned int hash_idx(const char *str)
{
    int i = 0;
    unsigned long hash = 5381;

	while (str[i] != '\0')
        hash = ((hash << 5) + hash) + str[i++];
	
    return hash % N_HASH;
}

struct s_entry *newtemp() {
    int length = snprintf(NULL, 0, "%d", tempnum);
    char *temp = malloc((length + 2) * sizeof(char));
    snprintf(temp, length, "$%d", tempnum);
    struct s_entry *entry = tos_newname(temp);
    free(temp);
    return entry;
}

struct s_entry *lookup_entry(struct s_entry *entry, const char *ident)
{
    if (entry == NULL) return NULL;
    if (strcmp(ident, entry->ident) == 0) return entry;
    return lookup_entry(entry->next, ident);
}

void free_entry(struct s_entry *entry)
{
    if (entry == NULL) return;
    free_entry(entry->next);
    free(entry->ident);
    free(entry);
}

struct s_context *tos_pushctx()
{
    struct s_context *new_ctx = (struct s_context *)malloc(sizeof(struct s_context));

    for (int i = 0; i < N_HASH; i++)
        new_ctx->entry[i] = NULL;

    new_ctx->next = context;
    return new_ctx;
}

struct s_context *tos_popctx()
{
    for (int i = 0; i < N_HASH; i++)
        free_entry(context->entry[i]);

    struct s_context *prev = context->next;
    free(context);
    return prev;
}

struct s_entry *tos_newname(const char *ident)
{
    unsigned int idx = hash_idx(ident);

    if (lookup_entry(context->entry[idx], ident) != NULL)
    {
        fprintf(stderr, "error : redefinition of '%s'\n", ident);
        return NULL;
    }

    struct s_entry *entry = (struct s_entry *)malloc(sizeof(struct s_entry));
    entry->ident = strdup(ident); 
    entry->type = NULL;
    entry->next = context->entry[idx];

    context->entry[idx] = entry;
    return entry;
}

struct s_entry *tos_lookup(const char *ident)
{
    unsigned int idx = hash_idx(ident);
    
    struct s_context *tmp = context;
    struct s_entry *look = NULL;

    while (tmp != NULL)
    {
        if ((look = lookup_entry(tmp->entry[idx], ident)) != NULL)
            return look;
        
        tmp = tmp->next;
    }
    return NULL;
}
