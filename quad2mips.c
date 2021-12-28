#include "quad2mips.h"

void compute_funoff(struct s_context *t)
{
	for (int i = 0; i < N_HASH; i++)
	{
		for (struct s_entry *e = t->entry[i]; e != NULL; e = e->next)
		{
			e->offset = t->count - e->offset - 1;
		}
	}
}

void init_string(struct s_stringtable *st, FILE *output)
{
	for (struct s_stringtable *tmp = st; tmp != NULL; tmp = tmp->next)
	{
		fprintf(output, "_S%d: .asciiz %s\n", tmp->idx, tmp->content);
	}
}

void init_glob(struct s_context *c, FILE *output)
{
	for (int i = 0; i < N_HASH; i++)
	{
		for (struct s_entry *e = c->entry[i]; e != NULL; e = e->next)
		{
			if (e->ident != NULL && (e->type->type == T_INT || e->type->type == T_BOOL))
			{
				fprintf(output, "_G%s: .word 0\n", e->ident);
			}
			else if (e->ident != NULL && e->type->type == T_ARRAY)
			{
				fprintf(output, "_A%s: .space %d\n", e->ident, e->type->u.array_info.size * 4);
			}
		}
	}
}

void alloc_tab(struct s_context *t, FILE *output)
{
	if (t->next != NULL)
	{
		fprintf(output, "addi $sp, $sp, -%d\n", t->count * 4);
	}
	else
	{
		init_glob(t, output);
	}
}

void free_tab(struct s_context *t, FILE *output)
{
	if (t->next != NULL)
	{
		for (size_t i = 0; i < t->count; i++)
		{
			fprintf(output, "sw $zero, %d($sp)\n", i * 4);
		}
		fprintf(output, "addi $sp, $sp, %d\n", t->count * 4);
	}
}

void load_quadop(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output)
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
		int off = tos_getoff(t, qo.u.name);
		if (off < 0)
		{
			fprintf(output, "lw %s, _G%s\n", registre, qo.u.name);
		}
		else
		{
			fprintf(output, "lw %s, %d($sp)\n", registre, off * 4 + my_off);
		}
		break;

	case QO_STRING:
		fprintf(output, "la %s, _S%d\n", registre, qo.u.string);
		break;

	default:
		break;
	}
}

void save(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output)
{
	int off = tos_getoff(t, qo.u.name);
	if (off < 0)
	{
		fprintf(output, "sw %s, _G%s\n", registre, qo.u.name);
	}
	else
	{
		fprintf(output, "sw %s, %d($sp)\n", registre, off * 4 + my_off);
	}
}

void quad2mips(quad q, struct s_context **t, int *is_def, unsigned int *my_off, FILE *output)
{
	switch (q.type)
	{
	case Q_ADD:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "add $t2, $t0, $t1\n");
		save(q.op3, "$t2", *my_off, *t, output);
		break;

	case Q_SUB:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "sub $t2, $t0, $t1\n");
		save(q.op3, "$t2", *my_off, *t, output);
		break;

	case Q_MUL:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "mul $t2, $t0, $t1\n");
		save(q.op3, "$t2", *my_off, *t, output);
		break;

	case Q_DIV:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "div $t2, $t0, $t1\n");
		save(q.op3, "$t2", *my_off, *t, output);
		break;

	case Q_MOD:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "div $t0, $t1\n");
		fprintf(output, "mfhi $t2\n");
		save(q.op3, "$t2", *my_off, *t, output);
		break;

	case Q_MINUS:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		fprintf(output, "mul $t1, $t0, -1\n");
		save(q.op3, "$t1", *my_off, *t, output);
		break;

	case Q_MOVE:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		save(q.op3, "$t0", *my_off, *t, output);
		break;

	case Q_GOTO:
		fprintf(output, "j _Q%d\n", q.op3.u.label);
		break;

	case Q_BLT:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "blt $t0, $t1, _Q%d\n", q.op3.u.label);
		break;

	case Q_BGT:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "bgt $t0, $t1, _Q%d\n", q.op3.u.label);
		break;

	case Q_BLE:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "ble $t0, $t1, _Q%d\n", q.op3.u.label);
		break;

	case Q_BGE:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "bge $t0, $t1, _Q%d\n", q.op3.u.label);
		break;

	case Q_BEQ:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "beq $t0, $t1, _Q%d\n", q.op3.u.label);
		break;

	case Q_BNE:
		load_quadop(q.op1, "$t0", *my_off, *t, output);
		load_quadop(q.op2, "$t1", *my_off, *t, output);
		fprintf(output, "bne $t0, $t1, _Q%d\n", q.op3.u.label);
		break;

	case Q_FUN:
		fprintf(output, "%s:\n", q.op3.u.name);
		if (tos_lookup(*t, q.op3.u.name)->type->u.function_info.arglist != NULL)
		{
			*is_def = 1;
		}
		break;

	case Q_SCALL:
		fprintf(output, "addi $sp, $sp, -4\n");
		fprintf(output, "sw $ra, 0($sp)\n");
		*my_off += 4;
		break;

	case Q_PARAM:
		fprintf(output, "addi $sp, $sp, -4\n");
		*my_off += 4;
		load_quadop(q.op3, "$t0", *my_off, *t, output);
		fprintf(output, "sw $t0, 0($sp)\n");
		break;

	case Q_CALL:
		*my_off -= 4 + q.op2.u.cst * 4;
		fprintf(output, "jal %s\n", q.op1.u.name);
		for (size_t i = 0; i < (q.op2.u.cst); i++)
		{
			fprintf(output, "sw $zero, %d($sp)\n", i * 4);
		}
		fprintf(output, "addi $sp, $sp, %d\n", (q.op2.u.cst) * 4);
		fprintf(output, "lw $ra, 0($sp)\n");
		fprintf(output, "sw $zero, 0($sp)\n");
		fprintf(output, "addi $sp, $sp, 4\n");
		if (q.op3.type != QO_EMPTY)
		{
			save(q.op3, "$v0", *my_off, *t, output);
		}
		break;

	case Q_RETURN:
		if (q.op3.type != QO_EMPTY)
		{
			load_quadop(q.op3, "$v0", *my_off, *t, output);
		}
		free_tab(*t, output);
		fprintf(output, "jr $ra\n");
		break;

	case Q_DRETURN:
		fprintf(output, "jr $ra\n");
		break;

	case Q_SETI:
		load_quadop(q.op2, "$t0", *my_off, *t, output);
		fprintf(output, "mul $t0, $t0, 4\n");
		load_quadop(q.op3, "$t1", *my_off, *t, output);
		fprintf(output, "sw $t1, _A%s($t0)\n", q.op1.u.name);
		break;

	case Q_GETI:
		load_quadop(q.op2, "$t0", *my_off, *t, output);
		fprintf(output, "mul $t0, $t0, 4\n");
		fprintf(output, "lw $t1, _A%s($t0)\n", q.op1.u.name);
		save(q.op3, "$t1", *my_off, *t, output);
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
			compute_funoff(*t);
		}
		break;

	case Q_PECTX:
		struct s_context *tp = q.op3.u.context;
		free_tab(tp, output);
		break;

	case Q_ECTX:
		free_tab(*t, output);
		struct s_context *tmp = *t;
		*t = (*t)->next;
		tos_freectx(tmp);
		break;

	case Q_EXIT:
		fprintf(output, "li $v0 17\nli $a0 0\nsyscall\n");
		break;

	default:
		break;
	}
}

void gen_mips(quad *quadcode, size_t len, FILE *output)
{
	fprintf(output, ".data\n");
	fprintf(output, "_STRUE: .asciiz \"true\"\n_SFALSE: .asciiz \"false\"\n");
	init_string(strings, output);
	free_stringtable(strings);

	char *mips_WriteInt = "WriteInt:\nli $v0 1\nlw $a0 0($sp)\nsyscall\njr $ra\n";
	char *mips_WriteString = "WriteString:\nli $v0 4\nlw $a0 0($sp)\nsyscall\njr $ra\n";
	char *mips_WriteBool = "WriteBool:\nli $v0 4\nlw $t0 0($sp)\nbnez $t0,  _True\nla $a0 _SFALSE\nsyscall\njr $ra\n_True:\nla $a0 _STRUE\nsyscall\njr $ra\n";
	char *mips_ReadInt = "ReadInt:\nli $v0 5\nsyscall\njr $ra\n";
	char *mips_exit = "li $v0 17\nli $a0 0\nsyscall";

	struct s_context *t = NULL;
	int is_def = 0;
	int my_off = 0;

	fprintf(output, ".align 2\n");
	quad2mips(quadcode[0], &t, &is_def, &my_off, output);

	fprintf(output, ".text\n.globl start\nstart:\njal main\n%s\n%s\n%s\n%s\n%s\n", mips_exit, mips_WriteInt, mips_WriteString, mips_WriteBool, mips_ReadInt);
	fprintf(output, "_Q0:\n");
	for (size_t i = 1; i < len; i++)
	{
		fprintf(output, "_Q%ld:\n", i);
		quad2mips(quadcode[i], &t, &is_def, &my_off, output);
	}

	// fprintf(output, "%s\n", mips_exit);
}
