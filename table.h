#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_HASH 100

struct s_typedesc {
    enum { T_VOID, T_INT, T_BOOL, T_ARRAY, T_FUNCTION } type;
    // union ...
};

struct s_entry {
    char *ident;
    struct s_typedesc *type;
    struct s_entry *next;
};

struct s_context {
    struct s_entry *entry[N_HASH];
    struct s_context *next;
};

unsigned int hash_idx(const char *str);

struct s_entry *lookup_entry(struct s_entry *entry, const char *ident);
void free_entry(struct s_entry *entry);

struct s_context *tos_pushctx(struct s_context *ctx);
struct s_context *tos_popctx (struct s_context *ctx);

struct s_entry *tos_newname(struct s_context *ctx, const char *ident);
struct s_entry *tos_lookup (struct s_context *ctx, const char *ident);

# endif
