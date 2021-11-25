# include "table.h"

int hash_idx(const char *str)
{
	int i = 0;
    int res = 0;

	while (str[i] != '\0')
    {
		res += ('a' - str[i] + 1) * (i + 1);
		i++;
	}
	return abs(res) % N_HASH;
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

void pushctx(struct s_context *ctx)
{
    struct s_context *new_ctx = (struct s_context *)malloc(sizeof(struct s_context));
    new_ctx->next = ctx;
    new_ctx->scope = (ctx == NULL) ? 0 : ctx->scope + 1;

    for (int i = 0; i < N_HASH; i++)
        new_ctx->entry[i] = NULL;

    ctx = new_ctx;
}

void popctx(struct s_context *ctx)
{
    for (int i = 0; i < N_HASH; i++)
        free_entry(ctx->entry[i]);

    struct s_context *tmp = ctx->next;
    free(ctx);
    ctx = tmp;
}

struct s_entry *newname(struct s_context *ctx, const char *ident)
{
    int idx = hash_idx(ident);

    // ... lookup if dup

    struct s_entry *entry = (struct s_entry *)malloc(sizeof(struct s_entry));
    entry->ident = strdup(ident); 
    entry->type = NULL;
    entry->next = ctx->entry[idx];
    ctx->entry[idx] = entry;
    
    return entry;
}

struct s_entry *lookup(struct s_context *ctx, const char *ident)
{
    int idx = hash_idx(ident);
    
    struct s_entry *look;
    struct s_context *tmp = ctx;

    while (tmp != NULL)
    {
        if ((look = lookup_entry(tmp->entry[idx], ident)) != NULL)
            return look;
        
        tmp = tmp->next;
    }
    return NULL;
}
