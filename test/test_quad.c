#include "../quad.h"

int check_quadop(quadop expected, quadop output) 
{
    if (expected.type != output.type) {
        return 0;
    }
    switch (expected.type) {
    case QO_CST:
        return expected.u.cst == output.u.cst;
        break;
    case QO_LABEL:
        return expected.u.label == output.u.label;
        break;
    case QO_BOOL:
        return expected.u.boolean == output.u.boolean;
        break;
    case QO_NAME:
        return strcmp(expected.u.name, output.u.name) == 0;
        break;
    case QO_EMPTY:
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

int check_quad(quad expected, quad output) 
{
    return expected.type == output.type &&
           check_quadop(expected.op1, output.op1) &&
           check_quadop(expected.op2, output.op2) &&
           check_quadop(expected.op3, output.op3);
}

int main(int argc, char **argv) 
{
    (void)argc; (void)argv;

    initcode();

    printf("check gencode\n"
           "gencode(GOTO,_,_,_);\n"
           "gencode(BEQ,_,_,_);\n"
           "gencode(BNE,_,_,_);\n");

    quad q1 = quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()),
         q2 = quad_make(Q_BEQ, quadop_empty(), quadop_empty(), quadop_empty()),
         q3 = quad_make(Q_BNE, quadop_empty(), quadop_empty(), quadop_empty());

    gencode(q1);
    gencode(q2);
    gencode(q3);

    if (globalcode == NULL) {
        printf("[ko]\n"
               "globalcode = NULL\n");
        exit(EXIT_FAILURE);
    }

    if (nextquad != 3) {
        printf("[ko]\n"
               "nextquad = %ld\n",
               nextquad);
        exit(EXIT_FAILURE);
    }

    if (!check_quad(q1, globalcode[0]) || !check_quad(q2, globalcode[1]) ||
        !check_quad(q3, globalcode[2])) {
        printf("[ko]\n");
        print_globalcode();
        exit(EXIT_FAILURE);
    }

    printf("[ok]\n\n");

    printf("check crelist\n"
           "crelist(0)\n");

    struct s_fifo *l0 = crelist(0);

    if (l0 && l0->num == 0) {
        printf("[ok]\n\n");
    } else {
        printf("[ko]\n");
        print_list(l0);
        exit(EXIT_FAILURE);
    }

    printf("check concat\n"
           "concat({0},{1})\n");

    struct s_fifo *l1 = crelist(1);
    struct s_fifo *l01 = concat(l0, l1);

    if (!l01 || !l01->next || l01->next->num != 0 || l01->num != 1) {
        printf("[ko]\n");
        print_list(l01);
        exit(EXIT_FAILURE);
    }

    printf("concat({0,1},{2})\n");

    struct s_fifo *l2 = crelist(2);
    struct s_fifo *l012 = concat(l01, l2);

    if (l012 && l012->next && l012->next->next && l012->next->next->num == 0 && l012->next->num == 1 &&
        l012->num == 2) {
        printf("[ok]\n\n");
    } else {
        printf("[ko]\n");
        print_list(l012);
        exit(EXIT_FAILURE);
    }

    printf("check complete\n"
           "complete({0,1},2)\n");

    complete(l01, 2);

    if (globalcode[0].op3.u.label == 2 && globalcode[1].op3.u.label == 2) {
        printf("[ok]\n");
    } else {
        printf("[ko]\n");
        print_globalcode();
        exit(EXIT_FAILURE);
    }
    
    freecode();
    return EXIT_SUCCESS;
}
