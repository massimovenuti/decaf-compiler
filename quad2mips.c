#include "quad2mips.h"

void comput_entry_off(struct s_entry *e, unsigned int *next_off, FILE *output)
{
	if (e == NULL)
	{
		return;
	}

	if (e->type->type != T_FUNCTION)
	{
		e->off = *next_off;
		*next_off += 4;
	}
	comput_entry_off(e->next, next_off, output);
}

unsigned int max_entry_off(struct s_entry *e)
{
	if (e == NULL)
	{
		return 0;
	}

	if (e->type->type == T_FUNCTION)
	{
		return max_entry_off(e->next);
	}
	

	unsigned int res = max_entry_off(e->next);
	return (e->off < res) ? res : e->off;
}

void alloc_tab(struct s_context *t, FILE *output)
{
	unsigned int off = 0;
	for (int i = 0; i < N_HASH; i++) {
		comput_entry_off(t->entry[i], &off, output);
	}
	fprintf(output, "subi $sp, $sp, %d\n", off);
}

void free_tab(struct s_context *t, FILE *output)
{
	unsigned int max_off = 0;
	unsigned int res = 0;
	for (int i = 0; i < N_HASH; i++) {
		res = max_entry_off(t->entry[i]);
		max_off = (max_off < res)? res: max_off;
	}
	fprintf(output, "addi $sp, $sp, %d\n", (max_off)? max_off + 4 : 0);
}

void load_quadop(quadop qo, const char *registre, struct s_context *t, FILE *output)
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
		fprintf(output, "lw %s, %d($sp)\n", registre, tos_lookup(t, qo.u.name)->off);
		break;

	default:
		break;
	}
}

void save(quadop qo, const char *registre, struct s_context *t, FILE *output)
{
	fprintf(output, "sw %s, %d($sp)\n", registre, tos_lookup(t, qo.u.name)->off);
}

void quad2mips(quad q, struct s_context **t, FILE *output)
{
	switch (q.type)
	{
	case Q_ADD:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "add $t2, $t0, $t1\n");
		save(q.op3, "$t2", *t, output);
		break;

	case Q_SUB:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "sub $t2, $t0, $t1\n");
		save(q.op3, "$t2", *t, output);
		break;

	case Q_MUL:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "mul $t2, $t0, $t1\n");
		save(q.op3, "$t2", *t, output);
		break;

	case Q_DIV:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "div $t2, $t0, $t1\n");
		save(q.op3, "$t2", *t, output);
		break;

	case Q_MOD:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "div $t0, $t1\n");
		fprintf(output, "mfhi $t2\n");
		save(q.op3, "$t2", *t, output);
		break;

	case Q_MINUS:
		load_quadop(q.op1, "$t0", *t, output);
		fprintf(output, "mul $t1, $t0, -1\n");
		save(q.op3, "$t1", *t, output);
		break;

	case Q_MOVE:
		load_quadop(q.op1, "$t0", *t, output);
		save(q.op3, "$t0", *t, output);
		break;

	case Q_GOTO:
		fprintf(output, "f Q%d\n", q.op3.u.label);
		break;

	case Q_BLT:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "blt $t0, $t1, Q%d\n", q.op3.u.label);
		break;

	case Q_BGT:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "bgt $t0, $t1, Q%d\n", q.op3.u.label);
		break;

	case Q_BLE:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "ble $t0, $t1, Q%d\n", q.op3.u.label);
		break;

	case Q_BGE:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "bge $t0, $t1, Q%d\n", q.op3.u.label);
		break;

	case Q_BEQ:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "beq $t0, $t1, Q%d\n", q.op3.u.label);
		break;

	case Q_BNE:
		load_quadop(q.op1, "$t0", *t, output);
		load_quadop(q.op2, "$t1", *t, output);
		fprintf(output, "bne $t0, $t1, Q%d\n", q.op3.u.label);
		break;
	
	case Q_FUN:
		fprintf(output, "%s:\n", q.op3.u.name);
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

	case Q_BCTX:
		*t = q.op3.u.context;
		alloc_tab(*t, output);
		break;
	
	case Q_ECTX:
		free_tab(*t, output);
		*t = (*t)->next;
		break;

	default:
		break;
	}
}

void gen_mips(quad *quadcode, size_t len, FILE *output)
{
	struct s_context *t = NULL;
	for (size_t i = 0; i < len; i++)
	{
		if (quadcode[i].type != Q_BCTX && quadcode[i].type != Q_ECTX)
		{
			fprintf(output, "Q%d:\n", i);
		}
		quad2mips(quadcode[i], &t, output);
	}
}