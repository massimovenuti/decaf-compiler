#ifndef __QUAD2MIPS_H__
#define __QUAD2MIPS_H__

#include <stdio.h>

#include "quad.h"

void load_quadop(quadop qo, const char *registre, FILE *output);

void save(quadop qo, const char *registre, FILE *output);

void quad2mips(quad q, FILE *output);

void gen_mips(quad *quadcode, size_t len, FILE *output);

#endif