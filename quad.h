#ifndef __QUAD_H__
#define __QUAD_H__

#include "utils.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_SIZE 255
#define LIST_SIZE 255

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

struct quad {
    enum quad_type {
        Q_ADD, // opération binaire et affectation
        Q_SUB,
        Q_MUL,
        Q_DIV,
        Q_MOD,
        Q_MINUS, // opération unaire et affectation
        Q_NOT,
        Q_MOVE, // copie
        Q_GOTO,   // branchement inconditionnel
        Q_BLT,    // branchement conditionnel
        Q_BGT,
        Q_BLE,
        Q_BGE,
        Q_BEQ,
        Q_BNE,
        Q_PARAM, // appel de procédure
        Q_CALL,
        Q_RETURN, // retour de fonction
        Q_FUN, // début de fonction
        Q_SETI, // affectation et indice de tableau
        Q_GETI,
        Q_BCTX,
        Q_ECTX,
        Q_WINT,
        Q_WBOOL,
        Q_WSTR,
        Q_RINT,
        Q_EXIT
    } type;
    quadop op1, op2, op3;
};

typedef struct quad quad;

extern quad *globalcode; // code généré
extern size_t codesize;  // taille du tableau globalcode
extern size_t nextquad;  // numéro du prochain quad généré

void initcode();      // initialise les variables globales
void freecode();      // libère la mémoire du tableau global
void gencode(quad q); // écrit dans globalcode[nextquad] et incrémente nextquad

quadop quadop_empty();           // crée un quadop vide
quadop quadop_cst(int cst);      // crée un quadop de type constante
quadop quadop_bool(int boolean); // crée un quadop de type booléen
quadop quadop_label(int label);  // crée un quadop de type label
quadop quadop_name(char *name);  // crée un quadop de type nom
quadop quadop_context(struct s_context *context); // crée un quadop de type context
quadop quadop_str(int string);    // crée un quadop de type string

quad quad_make(enum quad_type type, quadop op1, quadop op2,
               quadop op3); // crée un quad

struct ilist {
    int *content;
    size_t size;
};

typedef struct ilist ilist;

ilist *crelist(int label); // crée une liste d’adresses de quadruplets,
// initialisée à un seul élément "label", et qui retourne un pointeur sur la
// liste créée

ilist *concat(ilist *list1, ilist *list2); // concatène deux listes à partir de
// leurs pointeurs respectifs "list1" et "list2", et retourne un pointeur sur la
// liste résultat

void complete(ilist *list, int label); // complète tous les quadruplets de la
// liste "list" avec le label spécifié

void freelist(ilist *list);

void print_quadop(quadop qo);

void print_quad(quad q);

void print_ilist(ilist *l);

void print_globalcode();

#endif
