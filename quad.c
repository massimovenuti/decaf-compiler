#include "quad.h"

quad *globalcode; // code généré
size_t codesize;  // taille du tableau globalcode
size_t nextquad;  // numéro du prochain quad généré

void initcode() {
    MCHK(globalcode = (quad *)malloc(CODE_SIZE * sizeof(quad)));
    codesize = CODE_SIZE;
    nextquad = 0;
}

void freecode() { free(globalcode); }

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
    MCHK(l->content = malloc(sizeof(int)));
    l->content[0] = label;
    l->size = 1;
    return l;
}

ilist *concat(ilist *list1, ilist *list2) {
    ilist *l;
    if (list1 == NULL && list2 == NULL) {
        l = NULL;
    } else if (list1 == NULL) {
        l = list2;
    } else if (list2 == NULL) {
        l = list1;
    } else {
        MCHK(l = malloc(sizeof(ilist)));
        l->size = list1->size + list2->size;
        MCHK(l->content = malloc(l->size * sizeof(int)));
        MCHK(memcpy(l->content, list1->content, list1->size * sizeof(int)));
        MCHK(memcpy(l->content + list1->size, list2->content,
                    list2->size * sizeof(int)));
    }
    return l;
}

void complete(ilist *list, int label) {
    if (list == NULL)
        return;
    for (size_t i = 0; i < list->size; i++) {
        quad *q = &globalcode[list->content[i]];
        q->op3 = quadop_label(label);
    }
    freelist(list);
}

void freelist(ilist *list) {
    free(list->content);
    free(list);
}

void print_quadop(quadop qo) {
    switch (qo.type) {
    case QO_CST:
        printf("(cst:%d)", qo.u.cst);
        break;
    case QO_LABEL:
        printf("(label:%d)", qo.u.label);
        break;
    case QO_BOOL:
        printf("(bool:%s)", qo.u.boolean ? "true" : "false");
        break;
    case QO_NAME:
        printf("(name,%s)", qo.u.name);
        break;
    case QO_EMPTY:
        printf("_");
        break;
    default:
        printf("(?%d)", qo.type);
        break;
    }
}

void print_quad(quad q) {
    char quad_type_str[][10] = {
        "add",  "sub",   "mul",  "div",    "mod", "minus", "not",
        "move", "goto",  "blt",  "bgt",    "ble", "bge",   "beq",
        "bne",  "param", "call", "return", "fun", "seti",  "geti"};
    if (q.type >= Q_ADD && q.type <= Q_GETI) {
        printf("(%s,", quad_type_str[q.type]);
    } else {
        printf("(?%d,", q.type);
    }
    print_quadop(q.op1);
    printf(",");
    print_quadop(q.op2);
    printf(",");
    print_quadop(q.op3);
    printf(")");
}

void print_ilist(ilist *l) {
    if (l == NULL)
        return;
    printf("{ ");
    for (int i = 0; i < l->size; i++) {
        printf("%d ", l->content[i]);
    }
    printf("}\n");
}

void print_globalcode() {
    if (globalcode == NULL)
        return;
    for (int i = 0; i < nextquad; i++) {
        printf("%d: ", i);
        print_quad(globalcode[i]);
        printf("\n");
    }
}