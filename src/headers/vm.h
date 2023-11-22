#ifndef LAMA_INTERP_H
#define LAMA_INTERP_H

#include "value.h"
#include "common.h"
#include <stdint.h>

typedef struct {
    char *ip;
    int *sp;
    int *fp;
    ByteFile *bf;
} VM;

extern VM vm;

void vm_st_push(int32_t value);

int32_t vm_st_pop();

void vm_st_drop(int n);

void vm_st_reverse(int elems_count);

void fill(int n, int32_t value);

void init_interpreter();

void free_interpreter();

void eval();


#endif //LAMA_INTERP_H
