/**
* @file table.c
* @author LAJARGE, ULUCAN, VENUTI, VOGEL
* @brief Tables de symboles, types et table de chaine de caractères.
* @date 2021-12-28
* 
*/

#include "table.h"

struct s_context *context = NULL;
unsigned int tempnum = 0;
struct s_stringtable *strings = NULL;

//--------------------------------------------------------------
// TABLE DE SYMBOLES
//--------------------------------------------------------------

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
    if (entry == NULL)
        return NULL;
    if (strcmp(ident, entry->ident) == 0)
        return entry;
    return lookup_entry(entry->next, ident);
}

void free_entry(struct s_entry *entry)
{
    if (entry == NULL)
        return;
    free_entry(entry->next);
    free(entry->ident);

    if (entry->type != NULL)
    {
        if (entry->type->type == T_FUNCTION)
            free_arglist(entry->type->u.function_info.arglist);

        free(entry->type);
    }
    free(entry);
}

struct s_context *tos_pushctx(struct s_context *ctx)
{
    struct s_context *new_ctx = (struct s_context *)malloc(sizeof(struct s_context));

    for (int i = 0; i < N_HASH; i++)
        new_ctx->entry[i] = NULL;

    new_ctx->count = 0;
    new_ctx->next = ctx;
    return new_ctx;
}

struct s_context *tos_popctx(struct s_context *ctx)
{
    return ctx->next;
}

void tos_freectx(struct s_context *ctx)
{
    for (int i = 0; i < N_HASH; i++)
        free_entry(ctx->entry[i]);

    free(context);
}

struct s_context *tos_popfreectx(struct s_context *ctx)
{
    struct s_context *next = ctx->next;
    tos_freectx(ctx);
    return next;
}

struct s_entry *tos_newname(struct s_context *ctx, const char *ident)
{
    unsigned int idx = hash_idx(ident);

    if (lookup_entry(ctx->entry[idx], ident) != NULL)
        return NULL;

    struct s_entry *entry = (struct s_entry *)malloc(sizeof(struct s_entry));
    entry->ident = strdup(ident);
    entry->type = NULL;
    entry->offset = ctx->count++;
    entry->next = ctx->entry[idx];

    ctx->entry[idx] = entry;
    return entry;
}

struct s_entry *tos_newtemp(struct s_context *ctx)
{
    int length = snprintf(NULL, 0, "%d", tempnum);
    char *temp = malloc((length + 2) * sizeof(char));
    snprintf(temp, length + 2, "$%d", tempnum);

    struct s_entry *entry = tos_newname(ctx, temp);
    free(temp);
    tempnum++;
    return entry;
}

struct s_entry *tos_lookup(struct s_context *ctx, const char *ident)
{
    unsigned int idx = hash_idx(ident);

    struct s_entry *look = NULL;

    for (struct s_context *tmp = ctx; tmp != NULL; tmp = tmp->next)
    {
        if ((look = lookup_entry(tmp->entry[idx], ident)) != NULL)
            return look;
    }
    return NULL;
}

int tos_getoff(struct s_context *ctx, const char *ident)
{
    unsigned int idx = hash_idx(ident);

    struct s_entry *look = NULL;
    int offset = 0;

    for (struct s_context *tmp = ctx; tmp != NULL; tmp = tmp->next)
    {
        if (tmp->next == NULL)
            return -1;

        if ((look = lookup_entry(tmp->entry[idx], ident)) != NULL)
            return look->offset + offset;
        
        offset += tmp->count;
    }
    return -1;
}

//--------------------------------------------------------------
// TYPES
//--------------------------------------------------------------

struct s_typedesc *elementary_type(enum entry_type type)
{
    struct s_typedesc *desc = (struct s_typedesc *)malloc(sizeof(struct s_typedesc));
    desc->type = type;
    return desc;
}

struct s_typedesc *array_type(enum elem_type type, int size)
{
    struct s_typedesc *desc = (struct s_typedesc *)malloc(sizeof(struct s_typedesc));
    desc->type = T_ARRAY;
    desc->u.array_info.type = type;
    desc->u.array_info.size = size;
    return desc;
}

struct s_typedesc *function_type(enum ret_type type, struct s_arglist *arglist)
{
    struct s_typedesc *desc = (struct s_typedesc *)malloc(sizeof(struct s_typedesc));
    desc->type = T_FUNCTION;
    desc->u.function_info.ret_type = type;
    desc->u.function_info.arglist = arglist;
    desc->u.function_info.arglist_size = arglist_size(arglist);
    return desc;
}

struct s_arglist *arglist_addbegin(struct s_arglist *arglist, enum elem_type type)
{
    struct s_arglist *new_arg = (struct s_arglist *)malloc(sizeof(struct s_arglist));
    new_arg->type = type;
    new_arg->next = arglist;
    return new_arg;
}

struct s_arglist *arglist_addend(struct s_arglist *arglist, enum elem_type type)
{
    if (arglist == NULL)
        return arglist_addbegin(arglist, type);

    struct s_arglist *new_arg = (struct s_arglist *)malloc(sizeof(struct s_arglist));
    new_arg->type = type;
    new_arg->next = NULL;

    struct s_arglist *tmp = arglist;
    for (; tmp->next != NULL; tmp = tmp->next);
    tmp->next = new_arg;

    return arglist;
}

unsigned int arglist_size(struct s_arglist *arglist)
{
    return (arglist == NULL) ? 0 : 1 + arglist_size(arglist->next);
}

void free_arglist(struct s_arglist *arglist)
{
    if (arglist == NULL)
        return;
    free_arglist(arglist->next);
    free(arglist);
}

int is_elementary_type(struct s_typedesc *elem, enum entry_type type)
{
    return elem->type == type;
}

int is_array_type(struct s_typedesc *arr, enum elem_type type)
{
    return arr->type == T_ARRAY && arr->u.array_info.type == type;
}

int is_function_type(struct s_typedesc *fun, enum ret_type type, struct s_arglist *arglist)
{
    if (fun->type != T_FUNCTION)
        return 0;
    if (fun->u.function_info.ret_type != type)
        return 0;
    if (fun->u.function_info.arglist_size != arglist_size(arglist))
        return 0;

    struct s_arglist *tmp = fun->u.function_info.arglist;

    while (tmp != NULL)
    {
        if (tmp->type != arglist->type)
            return 0;

        tmp = tmp->next;
        arglist = arglist->next;
    }
    return 1;
}

//--------------------------------------------------------------
// TABLE DE CHAINES DE CARACTERES
//--------------------------------------------------------------

struct s_stringtable *new_string(struct s_stringtable *st, const char *content)
{
    struct s_stringtable *new_st = (struct s_stringtable *)malloc(sizeof(struct s_stringtable));
    new_st->content = strdup(content);
    new_st->idx = (st == NULL) ? 0 : 1 + st->idx;
    new_st->next = st;
    return new_st;
}

char *get_content(struct s_stringtable *st, unsigned int idx)
{
    for (struct s_stringtable *tmp = st; tmp != NULL; tmp = tmp->next)
    {
        if (tmp->idx == idx)
            return tmp->content;
    }
    return NULL;
}

unsigned int count_stringtable(struct s_stringtable *st)
{
    return (st == NULL) ? 0 : 1 + count_stringtable(st->next);
}

void free_stringtable(struct s_stringtable *st)
{
    if (st == NULL)
        return;
    free_stringtable(st->next);
    free(st->content);
    free(st);
}
