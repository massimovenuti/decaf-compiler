#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define S_INT   1
#define S_BOOL  2

#define HASH_SIZE 100

/**
* @brief Récupère un indice dans la table des symboles à partir du hachage d'un symbole
*/
int hash_idx(const char *str);

// liste de symboles
typedef struct s_symbol {
    char *ident;
    enum { INT = S_INT, BOOL = S_BOOL } type;
    int val;
    struct s_symbol *next;
} *Symbol;

// pile de table des symboles
typedef struct s_tos
{
    Symbol entry[HASH_SIZE];
    int scope;
    struct s_tos *next;
} *Tos;

/**
* @brief Crée une nouvelle liste de symboles
*/
Symbol new_symbol();

/**
* @brief Ajoute un nouveau symbole dans une liste de symboles
*/
Symbol add_symbol(Symbol sym, const char *ident, int type, int val);

/**
* @brief Recherche un symbole dans une liste de symboles
*/
Symbol lookup_symbol(Symbol sym, const char *ident);

/**
* @brief Libère l'espace mémoire occupé par une liste de symboles
*/
void free_symbol(Symbol sym);

/**
* @brief Affiche une liste de symboles
*/
void print_symbol(Symbol sym);

/**
* @brief Crée une nouvelle pile de table de symboles vide
*/
Tos new_tos();

/**
* @brief Empile une nouvelle table de symboles vide
*/
Tos push_tos(Tos prev_table);

/**
* @brief Ajoute un nouveau symbole dans la table des symboles au sommet de la pile
*/
Tos tos_newname(Tos curr_table, const char *ident, int type, int val);

/**
* @brief Dépile la table des symboles au sommet de la pile
*/
Tos pop_tos(Tos curr_table);

/**
* @brief Recherche un symbole dans une pile de table de symboles
*/
Symbol tos_lookup(Tos curr_table, const char *ident);

/**
* @brief Affiche le contenu d'un pile de table de symboles
*/
void print_tos(Tos curr_table);

// variable globale de la pile de tables de symboles
Tos table;

# endif
