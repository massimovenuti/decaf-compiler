#include "quad.h"

quad *globalcode; // code généré
size_t codesize;  // taille du tableau globalcode
size_t nextquad;  // numéro du prochain quad généré

void initcode() {
    MCHK(globalcode = (quad *)malloc(CODE_SIZE * sizeof(quad)));
    codesize = CODE_SIZE;
    nextquad = 0;
}

void freecode() {
    free(globalcode);
}

void gencode(quad q) {
    if (globalcode == NULL) {
        initcode();
    } else if (nextquad >= codesize) {
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

quadop quadop_cst(int cst) {
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

quadop quadop_label(int label) {
    quadop qo;
    qo.type = QO_LABEL;
    qo.u.label = label;
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

quad quad_make(enum quad_type type, quadop op1, quadop op2, quadop op3) {
    quad q;
    q.type = type;
    q.op1 = op1;
    q.op2 = op2;
    q.op3 = op3;
    return q;
}

ilist *crelist(int label) {
    ilist *l;
    MCHK(l = malloc(sizeof(ilist)));
    l->content = malloc(LIST_SIZE * sizeof(int));
    l->content[0] = label;
    l->max_size = LIST_SIZE;
    l->current_size = 1;
    return l;
}

ilist *concat(ilist *list1, ilist *list2) {
    ilist *l;
    MCHK(l = malloc(sizeof(ilist)));
    l->max_size = list1->max_size + list2->max_size;
    l->current_size = list1->current_size + list2->current_size;
    MCHK(l->content = malloc(l->max_size * sizeof(int)));
    MCHK(memcpy(l->content, list1->content, list1->current_size));
    MCHK(memcpy(l->content + list1->current_size, list2->content,
                list2->current_size));
    return l;
}

ilist *complete(ilist *list, int label) {
    for (size_t i = 0; i < list->current_size; i++) {
        quad *q = &globalcode[list->content[i]];
        q->op3 = quadop_label(label);
    }
    return list;
}

void freelist(ilist *list) {
    free(list->content);
    free(list);
}