#ifndef __QUAD2MIPS_H__
#define __QUAD2MIPS_H__

/**
* @file quad2mips.h
* @author LAJARGE, ULUCAN, VENUTI, VOGEL
* @brief Ensemble de fonctions permettant la génération de code mips (en-tête).
* @date 2021-12-28
* 
*/

#include <stdio.h>

#include "quad.h"
#include "table.h"


/**
* \fn void compute_funoff(struct s_context *t)
* \brief Calcule l'offset des éléments d'une table d'arguments de fonction
* \param t Une structure s_context
*/
void compute_funoff(struct s_context *t);

/**
* \fn void init_string(struct s_stringtable *st, FILE *output)
* \brief Génère les instructions mips pour une chaine de caractères
* \param st Une structure s_stringtable
* \param output Une structure FILE
*/
void init_string(struct s_stringtable *st, FILE *output);

/**
* \fn void init_glob(struct s_context *c, FILE *output);
* \brief Génère les instructions mips pour une variable globale
* \param c Une structure s_context
* \param output Une structure FILE
*/
void init_glob(struct s_context *c, FILE *output);

/**
* \fn void alloc_tab(struct s_context *t, FILE *output)
* \brief Alloue l'espace nécessaire à une table dans la mémoire 
* \param t Une structure s_context
* \param output Une structure FILE
*/
void alloc_tab(struct s_context *t, FILE *output);

/**
* \fn void free_tab(struct s_context *t, FILE *output)
* \brief Libère la mémoire allouée
* \param t Une structure s_context
* \param output Une structure FILE
*/
void free_tab(struct s_context *t, FILE *output);

/**
* \fn void load_quadop(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output)
* \brief Charqye un quadop dans un registre
* \param qo Un quadop
* \param registre Une chaîne de caractères
* \param my_off Un unsigned int
* \param t Une structure s_context
* \param output Une structure FILE
*/
void load_quadop(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output);

/**
* \fn void save(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output)
* \brief 
* \param qo Un quadop
* \param registre Une chaîne de caractères
* \param my_off Un unsigned int
* \param t Une structure s_context
* \param output Une structure FILE
*/
void save(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output);

/**
* \fn void quad2mips(quad q, struct s_context **t, int *is_def, unsigned int *my_off, FILE *output)
* \brief Génère une instruction mips en fonction du quad
* \param q Un quad
* \param registre Une chaîne de caractères
* \param my_off Un unsigned int
* \param t Une structure s_context
* \param output Une structure FILE
*/
void quad2mips(quad q, struct s_context **t, int *is_def, unsigned int *my_off, FILE *output);

/**
* \fn void gen_mips(quad *quadcode, size_t len, FILE *output)
* \brief Génère un fichier .mips
* \param qo Un quadop
* \param t Un pointeur sur une structure s_context
* \param is_def Un entier
* \param my_off Un unsigned int
* \param output Une structure FILE
*/
void gen_mips(quad *quadcode, size_t len, FILE *output);

#endif
