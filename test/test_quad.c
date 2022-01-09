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

    // === GENCODE ===

    quad q1 = quad_make(Q_GOTO, quadop_empty(), quadop_empty(), quadop_empty()),
         q2 = quad_make(Q_BEQ, quadop_empty(), quadop_empty(), quadop_empty()),
         q3 = quad_make(Q_BNE, quadop_empty(), quadop_empty(), quadop_empty());

    gencode(q1);

    // pour forcer les realloc
    codesize = 1;

    gencode(q2);
    gencode(q3);

    if (globalcode == NULL) {
        printf("[ko] gencode: globalcode = NULL\n");
        exit(EXIT_FAILURE);
    }

    if (nextquad != 3) {
        printf("[ko] gencode: nextquad = %ld\n",
               nextquad);
        exit(EXIT_FAILURE);
    }

    if (!check_quad(q1, globalcode[0]) || !check_quad(q2, globalcode[1]) ||
        !check_quad(q3, globalcode[2])) {
        printf("[ko] gencode:\n");
        print_globalcode();
        exit(EXIT_FAILURE);
    }

    printf("[ok] gencode\n");


    // === CRELIST ===

    struct s_fifo *l0 = crelist(0);

    if (l0 && l0->num == 0) {
        printf("[ok] crelist\n");
    } else {
        printf("[ko] crelist: ");
        fifo_print(l0);
        exit(EXIT_FAILURE);
    }


    // === CONCAT ===

    struct s_fifo *l1 = crelist(1);
    struct s_fifo *l01 = concat(l0, l1);

    l01 = concat(l01, NULL);
    l01 = concat(NULL, l01);
    l01 = concat(concat(NULL, NULL), l01);

    if (!l01 || !l01->next || l01->next->num != 0 || l01->num != 1) {
        printf("[ko] concat: ");
        fifo_print(l01);
        exit(EXIT_FAILURE);
    }

    struct s_fifo *l2 = crelist(2);
    struct s_fifo *l012 = concat(l01, l2);

    if (l012 && l012->next && l012->next->next && l012->next->next->num == 0 && l012->next->num == 1 &&
        l012->num == 2) {
        printf("[ok] concat\n");
    } else {
        printf("[ko] concat: ");
        fifo_print(l012);
        exit(EXIT_FAILURE);
    }

    // === COMPLETE ===

    complete(l01, 2);

    if (globalcode[0].op3.u.label == 2 && globalcode[1].op3.u.label == 2) {
        printf("[ok] complete\n");
    } else {
        printf("[ko] complete:\n");
        print_globalcode();
        exit(EXIT_FAILURE);
    }

    freecode();

    initcode();

    // === PRINT ===

    char a[] = "a", b[] = "b", c[] = "c", f[] = "f", t[] = "t";
    int label = 15, cst = 8;

    quadop qa = quadop_name(a), qb = quadop_name(b), qc = quadop_name(c), qf = quadop_name(f), qt = quadop_name(t);
    quadop qlabel = quadop_label(label);
    quadop qcst = quadop_cst(cst);
    quadop qtrue = quadop_bool(1);
    quadop qempty = quadop_empty();
    quadop qctx = quadop_context(NULL);

    strings = new_string(strings, "str");
    quadop qstr = quadop_str(strings->idx);

    gencode(quad_make(Q_ADD, qa, qb, qc));
    gencode(quad_make(Q_SUB, qa, qb, qc));
    gencode(quad_make(Q_MUL, qa, qb, qc));
    gencode(quad_make(Q_DIV, qa, qb, qc));
    gencode(quad_make(Q_MOD, qa, qb, qc));
    gencode(quad_make(Q_MINUS, qa, qempty, qc));
    gencode(quad_make(Q_MOVE, qa, qempty, qc));
    gencode(quad_make(Q_GOTO, qa, qempty, qempty));
    gencode(quad_make(Q_BLT, qa, qb, qlabel));
    gencode(quad_make(Q_BGT, qa, qb, qlabel));
    gencode(quad_make(Q_BLE, qa, qb, qlabel));
    gencode(quad_make(Q_BGE, qa, qb, qlabel));
    gencode(quad_make(Q_BEQ, qa, qb, qlabel));
    gencode(quad_make(Q_BNE, qa, qb, qlabel));
    gencode(quad_make(Q_PARAM, qempty, qempty, qstr));
    gencode(quad_make(Q_SCALL, qempty, qempty, qempty));
    gencode(quad_make(Q_CALL, qf, qcst, qempty));
    gencode(quad_make(Q_CALL, qf, qcst, qa));
    gencode(quad_make(Q_RETURN, qempty, qempty, qtrue));
    gencode(quad_make(Q_FUN, qempty, qempty, qf));
    gencode(quad_make(Q_SETI, qt, qcst, qa));
    gencode(quad_make(Q_GETI, qt, qcst, qa));
    gencode(quad_make(Q_BCTX, qempty, qempty, qctx));
    gencode(quad_make(Q_PECTX, qempty, qempty, qctx));
    gencode(quad_make(Q_ECTX, qempty, qempty, qempty));
    gencode(quad_make(Q_EXIT, qempty, qempty, qempty));

    // pour print un quadop incorrect    
    quadop qo;
    qo.type = -1;
    gencode(quad_make(-1, qo, qo, qo));

    printf("==== print\n");
    print_globalcode();

    freecode();

    globalcode = NULL;
    print_globalcode();
    
    return EXIT_SUCCESS;
}
