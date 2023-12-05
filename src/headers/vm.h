#ifndef LAMA_INTERP_H
#define LAMA_INTERP_H

#include "value.h"
#include "common.h"
#include <stdint.h>

typedef struct {
    char *ip;
    int *sp;
    int **s_top;
    int *fp;
    ByteFile *bf;
} VM;

extern VM vm;

void init_interpreter(ByteFile *bf, int32_t **stack_top, int32_t **stack_bottom);

void free_interpreter();

void eval();


#endif //LAMA_INTERP_H
