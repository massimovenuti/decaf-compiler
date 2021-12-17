#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_HASH 100  // taille de la table de hachage

// liste d'arguments
struct s_arglist {
    enum elem_type { E_BOOL, E_INT } type;
    struct s_arglist *next;
};

// descripteur de type
struct s_typedesc {
    enum entry_type { T_BOOL, T_INT, T_ARRAY, T_FUNCTION } type;
    union {
        struct {
            enum elem_type type;
            int size;
        } array_info;
        struct {
            enum ret_type { R_BOOL, R_INT, R_VOID } ret_type;
            struct s_arglist *arglist;
            int arglist_size;
        } function_info;
    } u;
};

// description d'une entrée
struct s_entry {
    char *ident;
    struct s_typedesc *type;
    struct s_entry *next;
    unsigned int off;    
};

// pile de tables de symboles
struct s_context {
    struct s_entry *entry[N_HASH];
    struct s_context *next;
};

extern struct s_context *context; // pointeur sur la table de symboles
extern unsigned tempnum; // numéro du prochain identificateur temporaire

//--------------------------------------------------------------
// TABLE DE SYMBOLES
//--------------------------------------------------------------

/**
* @brief Calcule l'indice de hachage d'une chaine de caractères
* @param str Une chaîne de caractères
*/
unsigned int hash_idx(const char *str);

/**
* @brief Cherche une entrée dans la pile de table de symboles
* @param entry Une entrée
* @param ident L'identificateur de l'entrée dans la pile de table de symboles
*/
struct s_entry *lookup_entry(struct s_entry *entry, const char *ident);

/**
* @brief Libère l'espace mémoire occupé par une entrée
* @param entry Une entrée
*/
void free_entry(struct s_entry *entry);

/**
* @brief Empile une nouvelle table de symboles vide
* @param ctx Une pile de tables de symboles
*/
struct s_context *tos_pushctx(struct s_context *ctx);

/**
* @brief Dépile une table de symboles
* @param ctx Une pile de tables de symboles
*/
struct s_context *tos_popctx(struct s_context *ctx);

/**
* @brief Dépile une table de symboles et libère l'espace mémoire occupée par celle-ci
* @param ctx Une pile de tables de symboles
*/
void tos_freectx(struct s_context *ctx);

/**
* @brief Dépile une table de symboles et libère l'espace mémoire occupée par celle-ci
* @param ctx Une pile de tables de symboles
* @warning Utilisée uniquement pour simplifier les tests
*/
struct s_context *tos_popfreectx(struct s_context *ctx);

/**
* @brief Génère un nouvel identificateur temporaire et ajoute l'entrée correspondante dans la pile de tables de symboles
* @param ctx Une pile de tables de symboles
*/
struct s_entry *tos_newtemp(struct s_context *ctx);

/**
* @brief Ajoute une nouvelle entrée dans la table au sommet de la pile
* @param ctx Une pile de tables de symboles
* @param ident L'identificateur de l'entrée dans la pile de table de symboles
*/
struct s_entry *tos_newname(struct s_context *ctx, const char *ident);

/**
* @brief Teste si une entrée existe dans un niveau de la pile de tables de symboles
* @param ctx Une pile de tables de symboles
* @param ident L'identificateur de l'entrée dans la pile de table de symboles
*/
struct s_entry *tos_lookup(struct s_context *ctx, const char *ident);

//--------------------------------------------------------------
// TYPES
//--------------------------------------------------------------

/**
* @brief Initialise un type élémentaire
* @param type Un type élémentaire (T_INT ou T_BOOL)
* @warning Ne pas confondre avec le type éléméntaire d'un tableau ou d'un argument de fonction
*/
struct s_typedesc* elementary_type(enum entry_type type);

/**
* @brief Initialise un type 'tableau'
* @param type Le type de chaque élément du tableau
* @param size La taille du tableau
*/
struct s_typedesc* array_type(enum elem_type type, int size);

/**
* @brief Initialise un type 'fonction'
* @param type Le type de retour de la fonction
* @param arglist La liste des types d'argument de la fonction
*/
struct s_typedesc* function_type(enum ret_type type, struct s_arglist* arglist);

/**
* @brief Ajoute un nouvel argument en tête d'une liste d'arguments
* @param arglist Une liste d'arguments
* @param type Le type du nouvel argument dans la liste
*/
struct s_arglist* arglist_addbegin(struct s_arglist *arglist, enum elem_type type);

/**
* @brief Ajoute un nouvel argument à la fin d'une liste d'arguments
* @param arglist Une liste d'arguments
* @param type Le type du nouvel argument dans la liste
*/
struct s_arglist* arglist_addend(struct s_arglist *arglist, enum elem_type type);

/**
* @brief Calcule le nombre d'éléments d'une liste d'arguments
* @param arglist Une liste d'arguments
*/
int arglist_size(struct s_arglist *arglist);

/**
* @brief Libère l'espace mémoire occupé par une liste d'arguments
* @param arglist Une liste d'arguments
*/
void free_arglist(struct s_arglist *arglist);

/**
* @brief Teste si un descripteur de type correspond à un type élémentaire
* @param elem Un descripteur de type
* @param type Un type élémentaire
*/
int is_elementary_type(struct s_typedesc *elem, enum entry_type type);

/**
* @brief Teste si un descripteur de type correspond à la description d'un tableau
* @param arr Un descripteur de type
* @param type Un type 'tableau'
*/
int is_array_type(struct s_typedesc *arr, enum elem_type type);

/**
* @brief Teste si un descripteur de type correspond à la description d'une fonction
* @param fun Un descripteur de type
* @param type Le type de retour de la fonction
* @param arglist La liste des arguments de la fonction
*/
int is_function_type(struct s_typedesc *fun, enum ret_type type, struct s_arglist *arglist);

#endif
