#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_HASH 100

struct s_arglist {
    enum expr_type { E_BOOL, E_INT } type;
    struct s_arglist *next;
};

struct s_typedesc {
    enum entry_type { T_BOOL, T_INT, T_ARRAY, T_FUNCTION } type;
    union {
        struct {
            enum expr_type type;
            int size;
        } array_info;
        struct {
            enum ret_type { R_BOOL, R_INT, R_VOID } ret_type;
            struct s_arglist *arglist;
            int arglist_size;
        } function_info;
    } u;
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

extern struct s_context *context;
extern unsigned tempnum;

unsigned int hash_idx(const char *str);

struct s_entry *newtemp(); // génère un nouveau temporaire
struct s_entry *lookup_entry(struct s_entry *entry, const char *ident);
void free_entry(struct s_entry *entry);

struct s_context *tos_pushctx();
struct s_context *tos_popctx();

struct s_entry *tos_newname(const char *ident);
struct s_entry *tos_lookup(const char *ident);

struct s_typedesc* elementary_type(enum entry_type type); // construit un type élémentaire (INT, BOOL)
struct s_typedesc* array_type(enum expr_type type, int size); // construit un type tableau
struct s_typedesc* function_type(enum ret_type type, struct s_arglist* arglist); // construit un type function

struct s_arglist* arglist_addbegin(struct s_arglist *arglist, enum expr_type type); // ajout argument en tête d'une liste d'arguments
struct s_arglist* arglist_addend(struct s_arglist *arglist, enum expr_type type); // ajout argument en fin d'une liste d'arguments
int arglist_size(struct s_arglist *arglist); // calcule le nombre d'éléments d'une liste d'arguments
void free_arglist(struct s_arglist *arglist); // libère l'espace mémoire occupé par une liste d'arguments

int is_elementary_type(struct s_typedesc *elem, enum entry_type type); // compare type élémentaire
int is_array_type(struct s_typedesc *arr, enum expr_type type, int index); // compare type et index
int is_function_type(struct s_typedesc *fun, enum ret_type type, struct s_arglist *arglist); // compare type de retour et liste d'arguments

#endif
