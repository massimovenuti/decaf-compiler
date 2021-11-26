#include "quad2mips.h"

void load_quadop(quadop qo, const char *registre, FILE *output)
{
	switch (qo.type)
	{
	case QO_CST:
		fprintf(output, "li %s, %d\n", registre, qo.u.cst);
		break;

	case QO_BOOL:
		fprintf(output, "li %s, %d\n", registre, qo.u.boolean);
		break;

	case QO_NAME:
		// TODO
		break;

	default:
		break;
	}
}

void save(quadop qo, const char *registre, FILE *output)
{
	// TODO
}

void quad2mips(quad q, FILE *output)
{
	switch (q.type)
	{
	case Q_ADD:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "add $t2, $t0, $t1");
		save(q.op3, "$t2", output);
		break;

	case Q_SUB:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "sub $t2, $t0, $t1");
		save(q.op3, "$t2", output);
		break;

	case Q_MUL:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "mul $t2, $t0, $t1");
		save(q.op3, "$t2", output);
		break;

	case Q_DIV:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "div $t2, $t0, $t1");
		save(q.op3, "$t2", output);
		break;

	case Q_MOD:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "div $t0, $t1\n");
		fprintf(output, "mfhi $t2\n");
		save(q.op3, "$t2", output);
		break;

	case Q_MINUS:
		load_quadop(q.op1, "$t0", output);
		fprintf(output, "mul $t1, $t0, -1\n");
		save(q.op3, "$t1", output);
		break;

	case Q_MOVE:
		load_quadop(q.op1, "$t0", output);
		save(q.op3, "$t0", output);
		break;

	case Q_GOTO:
		fprintf(output, "f Q%d\n", q.op3.u.label);
		break;

	case Q_BLT:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "blt $t0, $t1, Q%d", q.op3.u.label);
		break;

	case Q_BGT:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "bgt $t0, $t1, Q%d", q.op3.u.label);
		break;

	case Q_BLE:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "ble $t0, $t1, Q%d", q.op3.u.label);
		break;

	case Q_BGE:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "bge $t0, $t1, Q%d", q.op3.u.label);
		break;

	case Q_BEQ:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "beq $t0, $t1, Q%d", q.op3.u.label);
		break;

	case Q_BNE:
		load_quadop(q.op1, "$t0", output);
		load_quadop(q.op2, "$t1", output);
		fprintf(output, "bne $t0, $t1, Q%d", q.op3.u.label);
		break;

	case Q_PARAM:
		//TODO
		break;

	case Q_CALL:
		//TODO
		break;

	case Q_SETI:
		//TODO
		break;

	case Q_GETI:
		//TODO
		break;

	default:
		break;
	}
}

void gen_mips(quad *quadcode, size_t len, FILE *output)
{
	for (size_t i = 0; i < len; i++)
	{
		fprintf(output, "Q%d:\n", i);
		quad2mips(quadcode[i], output);
	}
}