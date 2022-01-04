#ifndef __QUAD_H__
#define __QUAD_H__

/**
* @file quad.h
* @author LAJARGE, ULUCAN, VENUTI, VOGEL
* @brief Génération du code 3 adresses (en-tête).
* @date 2021-12-28
* 
*/

#include "utils.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_SIZE 255
#define LIST_SIZE 255

//--------------------------------------------------------------
// QUADRUPLETS
//--------------------------------------------------------------

/**
* \struct quadop
* 
*/
struct quadop {
    enum quadop_type { QO_EMPTY, QO_CST, QO_BOOL, QO_LABEL, QO_NAME, QO_CONTEXT, QO_STRING } type;
    union {
        int cst;
        int boolean;
        int label;
        char *name;
        struct s_context *context;
        int string;
    } u;
};

typedef struct quadop quadop;

/**
* \struct quad 
* 
*/
struct quad {
    enum quad_type {
        Q_ADD,      /*!< Addition */
        Q_SUB,      /*!< Soustraction */
        Q_MUL,      /*!< Multiplication */
        Q_DIV,      /*!< Division*/
        Q_MOD,      /*!< Modulo */
        Q_MINUS,    /*!< Opposé */
        Q_MOVE,     /*!< Copie */
        Q_GOTO,     /*!< Branchement inconditionnel */
        Q_BLT,      /*!< Inférieur strict */
        Q_BGT,      /*!< Supérieur strict */
        Q_BLE,      /*!< Inférieur ou égal */
        Q_BGE,      /*!< Supérieur ou égal */
        Q_BEQ,      /*!< Egalité */
        Q_BNE,      /*!< Non égalité */
        Q_PARAM,    /*!< Appel de procédure */
        Q_SCALL,    /*!< Début d'un appel */
        Q_CALL,     /*!< Appel de fonction */
        Q_RETURN,   /*!< Retour de fonction */
        Q_FUN,      /*!< Début de fonction */
        Q_SETI,     /*!< Création d'un élément de tableau */
        Q_GETI,     /*!< Récupération d'un élément de tableau */
        Q_BCTX,     /*!< Début de contexte */
        Q_PECTX,    /*!< Pseudo fin de contexte */
        Q_ECTX,     /*!< Fin de contexte */
        Q_EXIT,     /*!< Exit */
        Q_DRETURN   /*!< Retour par défaut des procédures */
    } type;
    quadop op1, op2, op3;
};

typedef struct quad quad;

extern quad *globalcode; // code généré
extern size_t codesize;  // taille du tableau globalcode
extern size_t nextquad;  // numéro du prochain quad généré
// extern struct s_stringtable *strings;

/**
* \fn void initcode()
* \brief Initialise les variables globales
*/
void initcode();

/**
* \fn void freecode()
* \brief Libère la mémoire du tableau global
*/
void freecode();

/**
* \fn void gencode(quad q)
* \brief Ecrit dans globalcode[nextquad] et incrémente nextquad
*/
void gencode(quad q);

/**
* \fn quadop quadop_empty()
* \brief Crée un quadop vide
* \return Un quadop
*/
quadop quadop_empty();

/**
* \fn quadop quadop_cst(int cst)
* \brief Crée un quadop de type constante
* \param cst Un entier
* \return Un quadop
*/
quadop quadop_cst(int cst);

/**
* \fn quadop quadop_bool(int boolean)
* \brief Crée un quadop de type booléen
* \param boolean Un entier
* \return Un quadop
*/
quadop quadop_bool(int boolean);

/**
* \fn quadop quadop_label(int label)
* \brief Crée un quadop de type label
* \param label Un entier
* \return Un quadop
*/
quadop quadop_label(int label);

/**
* \fn quadop quadop_name(char *name)
* \brief Crée un quadop de type nom
* \param name Une chaine de caractères
* \return Un quadop
*/
quadop quadop_name(char *name);

/**
* \fn quadop quadop_context(struct s_context *context)
* \brief Crée un quadop de type context
* \param context Une structure s_context
* \return Un quadop
*/
quadop quadop_context(struct s_context *context);

/**
* \fn quadop quadop_str(int string)
* \brief Crée un quadop de type string
* \param string Un entier
* \return Un quadop
*/
quadop quadop_str(int string);

/**
* \fn quad quad_make(enum quad_type type, quadop op1, quadop op2, quadop op3)
* \brief Crée un quad
* \param type Un quad_type
* \param op1 Un quadop
* \param op2 Un quadop
* \param op3 Un quadop
* \return Un quad
*/
quad quad_make(enum quad_type type, quadop op1, quadop op2, quadop op3);

//--------------------------------------------------------------
// LISTES
//--------------------------------------------------------------

/**
* \struct ilist 
* 
*/
struct ilist {
    int *content;
    size_t size;
};

typedef struct ilist ilist;

/**
* \fn ilist *crelist(int label)
* \brief Crée une liste d'adresses de quadruplets
* \param label Un entier
* \return Un pointeur sur une liste
*/
ilist *crelist(int label);

/**
* \fn ilist *concat(ilist *list1, ilist *list2)
* \brief Concatène deux listes
* \param list1 Une liste
* \param list2 Une liste
* \return Un pointeur sur une liste
*/
ilist *concat(ilist *list1, ilist *list2);

/**
* \fn void complete(ilist *list, int label)
* \brief Complète tous les quadruplets d'une liste
* \param list Une liste
* \param label Un entier
*/
void complete(ilist *list, int label);

/**
* \fn void freelist(ilist *list)
* \brief Libère la mémoire allouée pour une liste
* \param list Une liste
*/
void freelist(ilist *list);

/**
* \fn void print_quadop(quadop qo)
* \brief Affiche un quadop
* \param qo Un quadop
*/
void print_quadop(quadop qo);

/**
* \fn void print_quad(quad q)
* \brief Affiche un quad
* \param q Un quad
*/
void print_quad(quad q);

/**
* \fn void print_ilist(ilist *l)
* \brief Affiche une liste
* \param l Une liste
*/
void print_ilist(ilist *l);

/**
* \fn void print_globalcode()
* \brief Affiche le code généré
*/
void print_globalcode();

#endif
