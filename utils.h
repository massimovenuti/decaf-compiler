#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

// memory check
#define MCHK(exp)                                                              \
    do {                                                                       \
        if ((exp) == NULL) {                                                   \
            perror(#exp);                                                      \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

// yacc check
#define YCHK(exp, msg)                                                         \
    do {                                                                       \
        if ((exp)) {                                                          \
            fprintf(stderr, "%s", msg);                                        \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#endif
