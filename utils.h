#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

// memory check
#define MEMCHECK(exp)                                                          \
    do {                                                                       \
        if ((exp) == NULL) {                                                   \
            perror(#exp);                                                      \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

// yacc check
#define ERRORIF(exp, msg)                                                      \
    do {                                                                       \
        if ((exp)) {                                                           \
            yyerror(msg);                                                       \
            YYERROR;                                                           \
        }                                                                      \
    } while (0)

#endif
