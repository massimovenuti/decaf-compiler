/**
* @file quad.c
* @author LAJARGE, ULUCAN, VENUTI, VOGEL
* @brief Génération du code 3 adresses.
* @date 2021-12-28
* 
*/

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
    qo.u.name = name;
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
    qo.u.string = string;
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

struct s_fifo *crelist(int label) {
    return fifo_push(NULL, label);
}

struct s_fifo *concat(struct s_fifo *list1, struct s_fifo *list2) {
    if (list1 == NULL && list2 == NULL) {
        return NULL;
    } else if (list1 == NULL) {
        return list2;
    } else if (list2 == NULL) {
        return list1;
    } else {
        struct s_fifo *l;
        for (l = list2; l->next != NULL; l = l->next);
        l->next = list1;
        return list2;
    }
}

void complete(struct s_fifo *list, int label) {
    for (struct s_fifo *l = list; l != NULL; l = l->next)
        globalcode[l->num].op3 = quadop_label(label);
    fifo_free(list);
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
    case QO_STRING:
        printf("%s", get_content(strings, qo.u.string));
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

    case Q_SCALL:
        printf("SCALL");
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
    
    case Q_PECTX:
        printf("Pseudo Fin Context =");
        print_quadop(q.op3);
		break;

    case Q_ECTX:
        printf("Fin Context");
		break;

    case Q_EXIT:
        printf("exit");
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

void print_list(struct s_fifo *list) {
    printf("{ ");
    for (struct s_fifo *l = list; l != NULL; l = l->next)
        printf("%d ", l->num);
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
