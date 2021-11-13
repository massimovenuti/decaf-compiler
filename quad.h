#ifndef __QUAD_H__
#define __QUAD_H__

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_SIZE 255

struct quadop {
    enum quadop_type { QO_CST, QO_BOOL, QO_NAME, QO_EMPTY } type;
    union {
        int cst;
        int boolean;
        char *name;
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
        Q_ASSIGN, // copie
        Q_GOTO,   // branchement inconditionnel
        Q_BLT,    // branchement conditionnel
        Q_BGT,
        Q_BLE,
        Q_BGE,
        Q_BEQ,
        Q_BNE,
        Q_PARAM, // appel de procédure
        Q_CALL,
        Q_SETI, // affectation et indice de tableau
        Q_GETI
    } type;
    quadop op1, op2, res;
};

typedef struct quad quad;

extern quad *globalcode; // code généré
extern size_t codesize;  // taille du tableau globalcode
extern size_t nextquad;  // numéro du prochain quad généré

void initcode(); // initialise les variables globales
void gencode(quad q); // écrit dans globalcode[nextquad] et incrémente nextquad

quadop newtemp(); // génère un temporaire frais

quadop quadop_empty();           // crée un quadop vide
quadop quadop_cst(int cst);      // crée un quadop de type constante
quadop quadop_bool(int boolean); // crée un quadop de type booléen
quadop quadop_name(char *name);  // crée un quadop de type nom

quad quad_make(enum quad_type type, quadop op1, quadop op2, quadop res); // crée un quad

#endif