#ifndef __QUAD2MIPS_H__
#define __QUAD2MIPS_H__

#include <stdio.h>

#include "quad.h"
#include "table.h"

void load_quadop(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output);

void save(quadop qo, const char *registre, unsigned int my_off, struct s_context *t, FILE *output);

void quad2mips(quad q, struct s_context **t, int *is_def, unsigned int *my_off, FILE *output);

void gen_mips(quad *quadcode, size_t len, FILE *output);

#endif
