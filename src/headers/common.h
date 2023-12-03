#ifndef LAMA_COMMON_H
#define LAMA_COMMON_H

#include <stdint.h>
#include "../runtime/runtime.h"

extern int Lread();
extern int Lwrite(int);
extern int Llength(void*);
extern void* Lstring (void *p);
extern void* Bstring(void*);
extern void* Belem (void *p, int i);
extern void* Bsta (void *v, int i, void *x);
extern void* Barray_data (int bn, int *data_);
extern void* Bsexp_data (int bn, int tag, int *data_);
extern int LtagHash (char *s);
extern int Btag (void *d, int t, int n);
extern int Barray_patt (void *d, int n);
extern void* Bclosure_values (int bn, void *entry, int *values);
extern void* Belem_l (void *p, int i);
extern int Bstring_patt (void *x, void *y);
extern int Bstring_tag_patt (void *x);
extern int Barray_tag_patt (void *x);
extern int Bsexp_tag_patt (void *x);
extern int Bunboxed_patt (void *x);
extern int Bboxed_patt (void *x);
extern int Bclosure_tag_patt (void *x);


typedef struct {
    char *string_ptr;              /* A pointer to the beginning of the string table */
    int *public_ptr;              /* A pointer to the beginning of publics table    */
    char *code_ptr;                /* A pointer to the bytecode itself               */
    int *global_ptr;              /* A pointer to the global area                   */
    int stringtab_size;          /* The size (in bytes) of the string table        */
    int global_area_size;        /* The size (in words) of global area             */
    int public_symbols_number;   /* The number of public symbols                   */
    char buffer[0];
} ByteFile;


typedef enum {
    I_BINOP, // binary operator
    I_CONST = 0x10, // put a constant on the stack
    I_STRING = 0x11, // put a string on the stack
    I_SEXP = 0x12, // create an S-expression
    I_STI = 0x13, // store a value into a reference
    I_STA = 0x14, // store a value into array/sexp/string
    I_JMP = 0x15, // unconditional jump
    I_END = 0x16, // end procedure definition
    I_RET = 0x17, // returns from a function
    I_DROP = 0x18, // drops the top element off
    I_DUP = 0x19, // duplicates the top element
    I_SWAP = 0x1a, // swaps two top elements
    I_ELEM = 0x1b, // takes an element of array/string/sexp
    I_LD, // load a variable to the stack
    I_LDA, // load a variable address to the stack
    I_ST, // store a value into a variable
    I_CJMP_Z = 0x50, // conditional jump
    I_CJMP_NZ = 0x51, // conditional jump
    I_BEGIN = 0x52, // begins procedure definition (with no closure)
    I_CBEGIN = 0x53, // begins procedure definition (with a closure)
    I_CLOSURE = 0x54, // create a closure
    I_CALLC = 0x55, // calls a closure
    I_CALL = 0x56, // calls a function/procedure
    I_TAG = 0x57, // checks the tag and arity of S-expression
    I_ARRAY = 0x58, // checks the tag and size of array
    I_FAIL = 0x59, // match failure (location, leave a value)
    I_LINE = 0x5a,// line info
    I_CALL_READ = 0x70,
    I_CALL_WRITE = 0x71,
    I_CALL_LENGTH = 0x72,
    I_CALL_STRING = 0x73,
    I_CALL_ARRAY = 0x74,
    I_PATT, // checks various patterns
} OpCode;

typedef enum {
    OP_PLUS = 1,  // +
    OP_MINUS, // -
    OP_MUL,   // *
    OP_DIV,   // /
    OP_MOD,   // %
    OP_LT,    // <
    OP_LTEQ,  // <=
    OP_GT,    // >
    OP_GTEQ,  // >=
    OP_EQ,    // ==
    OP_NEQ,   // !=
    OP_AND,   // &&
    OP_OR,    // !!
} BinOP;

enum Pattern {
    PATT_STR,  // =str
    PATT_TAG_STR,  // #string
    PATT_TAG_ARR,   // #array
    PATT_TAG_SEXP,    // #sexp
    PATT_BOXED,   // #ref
    PATT_UNBOXED, // #val
    PATT_TAG_CLOSURE, // #fun
};

#endif //LAMA_COMMON_H
