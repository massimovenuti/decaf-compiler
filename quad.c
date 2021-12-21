#include "quad.h"

quad *globalcode = NULL; // code généré
size_t codesize = 0;  // taille du tableau globalcode
size_t nextquad = 0;  // numéro du prochain quad généré

void initcode() {
    MEMCHECK(globalcode = (quad *)malloc(CODE_SIZE * sizeof(quad)));
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
    // debug
    // printf("%ld: ", nextquad);
    // print_quad(q);
    // printf("\n");
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
    // MEMCHECK(qo.u.name = malloc(len * sizeof(char)));
    // snprintf(qo.u.name, len, "%s", name);
    return qo;
}

quadop quadop_context(struct s_context *context) {
    quadop qo;
    qo.type = QO_CONTEXT;
    qo.u.context = context;
    return qo;
}

quadop quadop_str(int string) {
    quadop qo;
    qo.type = QO_STRING;
    qo.u.string = index;
    return qo;
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
    MEMCHECK(l = malloc(sizeof(ilist)));
    MEMCHECK(l->content = malloc(sizeof(int)));
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
        MEMCHECK(l = malloc(sizeof(ilist)));
        l->size = list1->size + list2->size;
        MEMCHECK(l->content = malloc(l->size * sizeof(int)));
        MEMCHECK(memcpy(l->content, list1->content, list1->size * sizeof(int)));
        MEMCHECK(memcpy(l->content + list1->size, list2->content,
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
        printf("%d", qo.u.cst);
        break;
    case QO_LABEL:
        printf("%d", qo.u.label);
        break;
    case QO_BOOL:
        printf("%d", qo.u.boolean);
        break;
    case QO_NAME:
        printf("%s", qo.u.name);
        break;
    case QO_EMPTY:
        printf("_");
        break;
    case QO_CONTEXT:
        printf("%p", qo.u.context);
        break;
    default:
        printf("?:%d", qo.type);
        break;
    }
}

void print_quad(quad q) {

    switch (q.type)
	{
	case Q_ADD:
        print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
		printf(" + ");
        print_quadop(q.op2);
		break;

	case Q_SUB:
        print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
		printf(" - ");
        print_quadop(q.op2);
		break;

	case Q_MUL:
        print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
		printf(" * ");
        print_quadop(q.op2);
		break;

	case Q_DIV:
        print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
		printf(" / ");
        print_quadop(q.op2);
		break;

	case Q_MOD:
        print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
		printf(" %% ");
        print_quadop(q.op2);
		break;

	case Q_MINUS:
        print_quadop(q.op3);
        printf(" = ");
		printf(" - ");
        print_quadop(q.op1);
		break;

    // case Q_NOT:
    //     print_quadop(q.op3);
    //     printf(" = ");
	// 	printf(" ! ");
    //     print_quadop(q.op1);
	// 	break;

	case Q_MOVE:
		print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
		break;

	case Q_GOTO:
        printf("goto ");
		print_quadop(q.op3);
		break;

	case Q_BLT:
        printf("if ");
		print_quadop(q.op1);
        printf(" < ");
		print_quadop(q.op2);
        printf(" goto ");
		print_quadop(q.op3);
		break;

	case Q_BGT:
        printf("if ");
		print_quadop(q.op1);
        printf(" > ");
		print_quadop(q.op2);
        printf(" goto ");
		print_quadop(q.op3);
		break;

	case Q_BLE:
        printf("if ");
		print_quadop(q.op1);
        printf(" <= ");
		print_quadop(q.op2);
        printf(" goto ");
		print_quadop(q.op3);
		break;

	case Q_BGE:
        printf("if ");
		print_quadop(q.op1);
        printf(" >= ");
		print_quadop(q.op2);
        printf(" goto ");
		print_quadop(q.op3);
		break;

	case Q_BEQ:
        printf("if ");
		print_quadop(q.op1);
        printf(" == ");
		print_quadop(q.op2);
        printf(" goto ");
		print_quadop(q.op3);
		break;

	case Q_BNE:
        printf("if ");
		print_quadop(q.op1);
        printf(" > ");
		print_quadop(q.op2);
        printf(" != ");
		print_quadop(q.op3);
		break;

	case Q_PARAM:
		printf("param ");
		print_quadop(q.op3);
		break;

	case Q_CALL:
        if (q.op3.type != QO_EMPTY) {
		    print_quadop(q.op3);
    		printf(" = ");
        }
		printf("call ");
		print_quadop(q.op2);
		printf(" ");
        print_quadop(q.op1);
		break;

    case Q_RETURN:
		printf("return ");
		print_quadop(q.op3);
		break;
    
    case Q_FUN:
		printf("def ");
		print_quadop(q.op3);
		break;

	case Q_SETI:
		print_quadop(q.op1);
        printf("[");
		print_quadop(q.op2);
        printf("] = ");
		print_quadop(q.op3);
		break;

	case Q_GETI:
		print_quadop(q.op3);
        printf(" = ");
        print_quadop(q.op1);
        printf("[");
		print_quadop(q.op2);
		printf("]");
		break;
    
    case Q_BCTX:
        printf("Context = ");
		print_quadop(q.op3);
		break;
    
    case Q_ECTX:
        printf("Fin Context");
		break;

	default:
        printf("(?:%d,", q.type);
        print_quadop(q.op1);
        printf(",");
        print_quadop(q.op2);
        printf(",");
        print_quadop(q.op3);
        printf(")");
		break;
	}
}

void print_ilist(ilist *l) {
    if (l == NULL)
        return;
    printf("{ ");
    for (size_t i = 0; i < l->size; i++) {
        printf("%d ", l->content[i]);
    }
    printf("}\n");
}

void print_globalcode() {
    if (globalcode == NULL)
        return;
    for (size_t i = 0; i < nextquad; i++) {
        printf("%ld:\t", i);
        print_quad(globalcode[i]);
        printf("\n");
    }
}
