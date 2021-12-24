#include "quad2mips.h"

void init_string(struct s_stringtable *st, FILE *output) {
	for (struct s_stringtable * tmp = st; tmp != NULL; tmp = tmp->next)
	{
		fprintf(output, "_S%d: .asciiz \"%s\"\n", tmp->idx, tmp->content);
	}
}

void alloc_tab(struct s_context *t, FILE *output)
{
	fprintf(output, "subi $sp, $sp, %d\n", t->count * 4);
}

void free_tab(struct s_context *t, FILE *output)
{
	fprintf(output, "addi $sp, $sp, %d\n", t->count * 4);
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
		fprintf(output, "lw %s, %d($sp)\n", registre, tos_getoff(t, qo.u.name) * 4);
		break;

	case QO_STRING:
		fprintf(output, "la %s, _S%d\n", registre, qo.u.string);
		break;

	default:
		break;
	}
}

void save(quadop qo, const char *registre, struct s_context *t, FILE *output)
{
	fprintf(output, "sw %s, %d($sp)\n", registre, tos_getoff(t, qo.u.name) * 4);
}

void quad2mips(quad q, struct s_context **t, int *is_def, FILE *output)
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
		fprintf(output, "j Q%d\n", q.op3.u.label);
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
		if (strcmp(q.op3.u.name, "main"))
		{
			*is_def = 1;
		}
		break;

	case Q_PARAM:
		fprintf(output, "subi $sp, $sp, 4\n");
		load_quadop(q.op3, "$t0", *t, output);
		fprintf(output, "sw $t0, 0($sp)\n");
		break;

	case Q_CALL:
		fprintf(output, "jal %s\n", q.op1.u.name);
		fprintf(output, "addi $sp, $sp, %d\n", q.op2.u.cst * 4);
		if (q.op3.type != QO_EMPTY)
		{
			save(q.op3, "$v0", *t, output);
		}
		// free_tab(*t, output);
		break;
	
	case Q_RETURN:
		if (q.op3.type != QO_EMPTY)
		{
			load_quadop(q.op3, "$v0", *t, output);
		}
		free_tab(*t, output);
		fprintf(output, "jr $ra\n");
		break;

	case Q_SETI:
		//TODO
		break;

	case Q_GETI:
		//TODO
		break;

	case Q_BCTX:
		*t = q.op3.u.context;
		if (!(*is_def))
		{
			alloc_tab(*t, output);
		}
		else
		{
			*is_def = 0;
		}
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
	fprintf(output,".data\n");
	init_string(strings, output);

	char * mips_WriteInt = "WriteInt:\nli $v0 1\nlw $a0 0($sp)\nsyscall\njr $ra\n";
	char * mips_WriteString = "WriteString:\nli $v0 4\nlw $a0 0($sp)\nsyscall\njr $ra\n";
	char * mips_ReadInt = "ReadInt:\nli $v0 5\nsyscall\njr $ra\n";

	fprintf(output, ".text\nj main\n%s\n%s\n%s\n", mips_WriteInt, mips_WriteString, mips_ReadInt);

	struct s_context *t = NULL;
	int is_def = 0;
	for (size_t i = 0; i < len; i++)
	{
		if (quadcode[i].type != Q_BCTX && quadcode[i].type != Q_ECTX)
		{
			fprintf(output, "Q%ld:\n", i);
		}
		quad2mips(quadcode[i], &t, &is_def, output);
	}
}
