# include "table.h"

unsigned int hash_idx(const char *str)
{
    int i = 0;
    unsigned long hash = 5381;

	while (str[i] != '\0')
        hash = ((hash << 5) + hash) + str[i++];
	
    return hash % N_HASH;
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

struct s_context *tos_pushctx(struct s_context *ctx)
{
    struct s_context *new_ctx = (struct s_context *)malloc(sizeof(struct s_context));

    for (int i = 0; i < N_HASH; i++)
        new_ctx->entry[i] = NULL;

    new_ctx->next = ctx;
    return new_ctx;
}

struct s_context *tos_popctx(struct s_context *ctx)
{
    for (int i = 0; i < N_HASH; i++)
        free_entry(ctx->entry[i]);

    struct s_context *prev = ctx->next;
    free(ctx);
    return prev;
}

struct s_entry *tos_newname(struct s_context *ctx, const char *ident)
{
    unsigned int idx = hash_idx(ident);

    if (lookup_entry(ctx->entry[idx], ident) != NULL)
    {
        fprintf(stderr, "error : redefinition of '%s'\n", ident);
        return NULL;
    }

    struct s_entry *entry = (struct s_entry *)malloc(sizeof(struct s_entry));
    entry->ident = strdup(ident); 
    entry->type = NULL;
    entry->next = ctx->entry[idx];

    ctx->entry[idx] = entry;
    return entry;
}

struct s_entry *tos_lookup(struct s_context *ctx, const char *ident)
{
    unsigned int idx = hash_idx(ident);
    
    struct s_context *tmp = ctx;
    struct s_entry *look = NULL;

    while (tmp != NULL)
    {
        if ((look = lookup_entry(tmp->entry[idx], ident)) != NULL)
            return look;
        
        tmp = tmp->next;
    }
    
    fprintf(stderr, "error : '%s' undeclared variable\n", ident);
    return NULL;
}
