#ifndef __TABLE_H__
#define __TABLE_H__

/**
* @file table.h
* @author LAJARGE, ULUCAN, VENUTI, VOGEL
* @brief Table de symboles, types, table de chaine de caractères et pile d'entiers (en-tête).
* @date 2021-12-28
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_HASH 100  // taille de la table de hachage

/**
* \struct s_fifo
* \brief Pile d'entiers
*/
struct s_fifo {
    int num;
    struct s_fifo *next;
};

/**
* \struct s_stringtable
* \brief Table de chaine de caractères
*/
struct s_stringtable {
    char *content;
    unsigned int idx;
    struct s_stringtable *next;
};

/**
* \struct s_arglist
* \brief Liste d'arguments
*/
struct s_arglist {
    enum elem_type { E_BOOL, E_INT, E_STR } type;
    struct s_arglist *next;
};

/**
* \struct s_typedesc
* \brief Descripteur de type
*/
struct s_typedesc {
    enum entry_type { T_BOOL, T_INT, T_ARRAY, T_FUNCTION } type;
    union {
        struct {
            enum elem_type type;
            unsigned int size;
        } array_info;
        struct {
            enum ret_type { R_BOOL, R_INT, R_VOID } ret_type;
            struct s_arglist *arglist;
            unsigned int arglist_size;
        } function_info;
    } u;
};

/**
* \struct s_entry
* \brief Description d'une entrée
*/
struct s_entry {
    char *ident;
    struct s_typedesc *type;
    unsigned int offset;
    struct s_entry *next;
};

/**
* \struct s_context
* \brief Pile de tables de symboles
*/
struct s_context {
    struct s_entry *entry[N_HASH];
    unsigned int count;
    unsigned int idx;
    struct s_context *next;
};

extern struct s_context *context; // pointeur sur la table de symboles
extern unsigned tempnum; // numéro du prochain identificateur temporaire
extern struct s_stringtable *strings; // pointeur sur la table de chaînes de caractères

//--------------------------------------------------------------
// TABLE DE SYMBOLES
//--------------------------------------------------------------

/**
* \fn unsigned int hash_idx(const char *str)
* \brief Calcule l'indice de hachage d'une chaîne de caractères
* \param str Une chaîne de caractères
*/
unsigned int hash_idx(const char *str);

/**
* \fn struct s_entry *lookup_entry(struct s_entry *entry, const char *ident)
* \brief Cherche une entrée dans la pile de table de symboles
* \param entry Une entrée
* \param ident L'identificateur de l'entrée dans la pile de tables de symboles
*/
struct s_entry *lookup_entry(struct s_entry *entry, const char *ident);

/**
* \fn void free_entry(struct s_entry *entry)
* \brief Libère l'espace mémoire occupé par une entrée
* \param entry Une entrée
*/
void free_entry(struct s_entry *entry);

/**
* \fn struct s_context *tos_pushctx(struct s_context *ctx)
* \brief Empile une nouvelle table de symboles vide
* \param ctx Une pile de tables de symboles
*/
struct s_context *tos_pushctx(struct s_context *ctx);

/**
* \fn struct s_context *tos_popctx(struct s_context *ctx)
* \brief Dépile une table de symboles
* \param ctx Une pile de tables de symboles
*/
struct s_context *tos_popctx(struct s_context *ctx);

/**
* \fn void tos_freectx(struct s_context *ctx)
* \brief Libère l'espace mémoire occupée par celle-ci
* \param ctx Une pile de tables de symboles
*/
void tos_freectx(struct s_context *ctx);

/**
* \fn struct s_context *tos_popfreectx(struct s_context *ctx)
* \brief Dépile une table de symboles et libère l'espace mémoire occupée par celle-ci
* \param ctx Une pile de tables de symboles
* \warning Utilisée uniquement pour simplifier les tests
*/
struct s_context *tos_popfreectx(struct s_context *ctx);

/**
* \fn struct s_entry *tos_newtemp(struct s_context *ctx)
* \brief Génère un nouvel identificateur temporaire et ajoute l'entrée correspondante dans la pile de tables de symboles
* \param ctx Une pile de tables de symboles
*/
struct s_entry *tos_newtemp(struct s_context *ctx);

/**
* \fn struct s_entry *tos_newname(struct s_context *ctx, const char *ident)
* \brief Ajoute une nouvelle entrée dans la table au sommet de la pile
* \param ctx Une pile de tables de symboles
* \param ident L'identificateur de l'entrée dans la pile de tables de symboles
*/
struct s_entry *tos_newname(struct s_context *ctx, const char *ident);

/**
* \fn struct s_entry *tos_lookup(struct s_context *ctx, const char *ident)
* \brief Teste si une entrée existe dans un niveau de la pile de tables de symboles
* \param ctx Une pile de tables de symboles
* \param ident L'identificateur de l'entrée dans la pile de tables de symboles
*/
struct s_entry *tos_lookup(struct s_context *ctx, const char *ident);

/**
* \fn int tos_getoff(struct s_context *ctx, const char *ident)
* \brief Calcule l'offset d'une entrée à partir du sommet de la pile de tables de symboles
* \param ctx Une pile de tables de symboles
* \param ident L'identificateur de l'entrée dans la pile de tables de symboles
*/
int tos_getoff(struct s_context *ctx, const char *ident);

//--------------------------------------------------------------
// TYPES
//--------------------------------------------------------------

/**
* \fn struct s_typedesc* elementary_type(enum entry_type type)
* \brief Initialise un type élémentaire
* \param type Un type élémentaire (T_INT ou T_BOOL)
*/
struct s_typedesc* elementary_type(enum entry_type type);

/**
* \fn struct s_typedesc* array_type(enum elem_type type, int size)
* \brief Initialise un type 'tableau'
* \param type Le type de chaque élément du tableau
* \param size La taille du tableau
*/
struct s_typedesc* array_type(enum elem_type type, int size);

/**
* \fn struct s_typedesc* function_type(enum ret_type type, struct s_arglist* arglist)
* \brief Initialise un type 'fonction'
* \param type Le type de retour de la fonction
* \param arglist La liste des types d'argument de la fonction
*/
struct s_typedesc* function_type(enum ret_type type, struct s_arglist* arglist);

/**
* \fn struct s_arglist* arglist_addbegin(struct s_arglist *arglist, enum elem_type type)
* \brief Ajoute un nouvel argument en tête d'une liste d'arguments
* \param arglist Une liste d'arguments
* \param type Le type du nouvel argument dans la liste
*/
struct s_arglist* arglist_addbegin(struct s_arglist *arglist, enum elem_type type);

/**
* \fn struct s_arglist* arglist_addend(struct s_arglist *arglist, enum elem_type type)
* \brief Ajoute un nouvel argument à la fin d'une liste d'arguments
* \param arglist Une liste d'arguments
* \param type Le type du nouvel argument dans la liste
*/
struct s_arglist* arglist_addend(struct s_arglist *arglist, enum elem_type type);

/**
* \fn unsigned int arglist_size(struct s_arglist *arglist)
* \brief Calcule le nombre d'éléments d'une liste d'arguments
* \param arglist Une liste d'arguments
*/
unsigned int arglist_size(struct s_arglist *arglist);

/**
* \fn void free_arglist(struct s_arglist *arglist)
* \brief Libère l'espace mémoire occupé par une liste d'arguments
* \param arglist Une liste d'arguments
*/
void free_arglist(struct s_arglist *arglist);

/**
* \fn int is_elementary_type(struct s_typedesc *elem, enum entry_type type)
* \brief Teste si un descripteur de type correspond à un type élémentaire
* \param elem Un descripteur de type
* \param type Un type élémentaire
*/
int is_elementary_type(struct s_typedesc *elem, enum entry_type type);

/**
* \fn int is_array_type(struct s_typedesc *arr, enum elem_type type)
* \brief Teste si un descripteur de type correspond à la description d'un tableau
* \param arr Un descripteur de type
* \param type Un type 'tableau'
*/
int is_array_type(struct s_typedesc *arr, enum elem_type type);

/**
* \fn int check_ret_type(struct s_typedesc *fun, enum ret_type type)
* \brief Vérifie si un type de retour concorde avec celui d'un descripteur de fonction
* \param fun Un descripteur de type
* \param type Le type de retour de la fonction
*/
int check_ret_type(struct s_typedesc *fun, enum ret_type type);

/**
* \fn int check_arglist(struct s_typedesc *fun, struct s_arglist *arglist)
* \brief Vérifie si une liste d'arguments concorde avec celle d'un descripteur de fonction
* \param fun Un descripteur de type
* \param arglist La liste des arguments de la fonction
*/
int check_arglist(struct s_typedesc *fun, struct s_arglist *arglist);

/**
* \fn int is_function_type(struct s_typedesc *fun, enum ret_type type, struct s_arglist *arglist)
* \brief Teste si un descripteur de type correspond à la description d'une fonction
* \param fun Un descripteur de type
* \param type Le type de retour de la fonction
* \param arglist La liste des arguments de la fonction
*/
int is_function_type(struct s_typedesc *fun, enum ret_type type, struct s_arglist *arglist);

//--------------------------------------------------------------
// TABLE DE CHAINES DE CARACTERES
//--------------------------------------------------------------

/**
* \fn struct s_stringtable *new_string(struct s_stringtable *st, const char *content)
* \brief Ajoute une nouvelle de chaîne de caractères en fin d'une table de chaînes de caractères
* \param st Une table de chaînes de caractères
* \param content Le contenu de la chaîne de caractères
*/
struct s_stringtable *new_string(struct s_stringtable *st, const char *content);

/**
* \fn char *get_content(struct s_stringtable *st, unsigned int idx)
* \brief Récupère le contenu d'une chaîne de caractères par son indice dans une table de chaînes de caractères
* \param st Une table de chaînes de caractères
* \param idx L'indice correspondant à l'adresse de la chaîne de caractères dans la table
*/
char *get_content(struct s_stringtable *st, unsigned int idx);

/**
* \fn unsigned int count_stringtable(struct s_stringtable *st)
* \brief Récupère le nombre d'éléments dans une table de chaînes de caractères
* \param st Une table de chaînes de caractères
*/
unsigned int count_stringtable(struct s_stringtable *st);

/**
* \fn void free_stringtable(struct s_stringtable *st)
* \brief Libère l'espace mémoire occupé par une table de chaînes de caractères
* \param st Une table de chaînes de caractères
*/
void free_stringtable(struct s_stringtable *st);

//--------------------------------------------------------------
// PILE D'ENTIERS
//--------------------------------------------------------------

/**
* \fn struct s_fifo *fifo_push(struct s_fifo *fifo, int num)
* \brief Empile un nouvel entier dans la pile
* \param fifo Une pile d'entiers
* \param num Un entier
*/
struct s_fifo *fifo_push(struct s_fifo *fifo, int num);

/**
* \fn struct s_fifo *fifo_pop(struct s_fifo *fifo)
* \brief Dépile et libère l'espace mémoire occupé par le maillon au sommet de la pile
* \param fifo Une pile d'entiers
*/
struct s_fifo *fifo_pop(struct s_fifo *fifo);

/**
* \fn void fifo_free(struct s_fifo *fifo)
* \brief Libère l'espace mémoire occupé par une pile d'entiers
* \param fifo Une pile d'entiers
*/
void fifo_free(struct s_fifo *fifo);

/**
* \fn void fifo_print(struct s_fifo *list)
* \brief Affiche une pile d'entiers
* \param fifo Une pile d'entiers
*/
void fifo_print(struct s_fifo *fifo);

//--------------------------------------------------------------
// AFFICHAGE
//--------------------------------------------------------------

/**
* \fn void print_elem_type(elem elem_type type)
* \brief Affiche le nom d'un type élémentaire
* \param type Un type de fonction
*/
void print_elem_type(enum elem_type type);

/**
* \fn void print_arglist(struct s_arglist *arglist)
* \brief Affiche le type des arguments dans une liste d'arguments
* \param arglist Une liste d'arguments
*/
void print_arglist(struct s_arglist *arglist);

/**
* \fn void print_function(struct s_typedesc *fun, const char* ident)
* \brief Affiche le prototype d'une fonction
* \param fun Descripteur du type d'une fonction
* \param ident Une chaine de caractère
*/
void print_function(struct s_typedesc *fun, const char* ident);

/**
* \fn void print_entry(struct s_entry *entry)
* \brief Affiche une structure d'entrée
* \param entry Une structure descripteur d'entrée
*/
void print_entry(struct s_entry *entry);

/**
* \fn void tos_printctx(struct s_context *ctx)
* \brief Affiche les contextes d'une table des symboles
* \param ctx Une pile de table des symboles
*/
void tos_printctx(struct s_context *ctx);

#endif
