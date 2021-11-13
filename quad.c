#include "quad.h"

quad *globalcode; // code généré
size_t codesize;  // taille du tableau globalcode
size_t nextquad;  // numéro du prochain quad généré

void initcode() {
    MCHK(globalcode = (quad *)malloc(CODE_SIZE * sizeof(quad)));
    codesize = CODE_SIZE;
    nextquad = 0;
}

void gencode(quad q) {
    if (globalcode == NULL) {
        initcode();
    } else if (nextquad > (codesize - 1)) {
        codesize *= 2;
        globalcode = realloc(globalcode, codesize * sizeof(quad));
    }
    globalcode[nextquad] = q;
    nextquad++;
}

quadop quadop_empty() {
    quadop qo;
    qo.type = QO_EMPTY;
    return qo; 
}

quadop quadop_cst(int cst)  { 
    quadop qo;
    qo.type = QO_CST;
    qo.u.cst = cst;
    return qo; 
}

quadop quadop_bool(int boolean) { 
    quadop qo;
    qo.type = QO_BOOL;
    qo.u.boolean = boolean;
    return qo; 
}

quadop quadop_name(char *name) {
    quadop qo;
    qo.type = QO_NAME;
    qo.u.name = name; // ?
    // int len = strlen(name) + 1;
    // MCHK(qo.u.name = malloc(len * sizeof(char)));
    // snprintf(qo.u.name, len, "%s", name);
    return qo;
}

quadop newtemp() {
    // TODO
    return quadop_empty();
}

quad quad_make(enum quad_type type, quadop op1, quadop op2, quadop res) {
    quad q;
    q.type = type;
    q.op1 = op1;
    q.op2 = op2;
    q.res = res;
    return q;
}
