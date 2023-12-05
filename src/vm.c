#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "headers/vm.h"
#include "headers/common.h"
#include "headers/value.h"

VM vm;

extern int32_t *__gc_stack_top, *__gc_stack_bottom;
void *__start_custom_data;
void *__stop_custom_data;

extern void __init(void);

ByteFile *read_file(char *fname) {
    FILE *f = fopen(fname, "rb");
    long size;
    ByteFile *file;

    if (f == 0) {
        failure("%s\n", strerror(errno));
    }

    if (fseek(f, 0, SEEK_END) == -1) {
        failure("%s\n", strerror(errno));
    }

    file = (ByteFile *) malloc(sizeof(int) * 4 + (size = ftell(f)));

    if (file == 0) {
        failure("*** FAILURE: unable to allocate memory.\n");
    }

    rewind(f);

    if (size != fread(&file->stringtab_size, 1, size, f)) {
        failure("%s\n", strerror(errno));
    }

    fclose(f);

    file->string_ptr = &file->buffer[file->public_symbols_number * 2 * sizeof(int)];
    file->public_ptr = (int *) file->buffer;
    file->code_ptr = (char *) &file->string_ptr[file->stringtab_size];
    file->global_ptr = (int *) malloc(file->global_area_size * sizeof(int));
    return file;
}

/* Gets a string from a string table by an index */
char *get_string_from_table(ByteFile *f, int pos) {
    return f->string_ptr + pos;
}

void swap(int32_t *a, int32_t *b) {
    int32_t temp = *a;
    *a = *b;
    *b = temp;
}

const int MAX_STACK_SIZE = 1024 * 1024;

#define VM_ST_POP_UNCHECKED (*(vm.sp++))
#define VM_ST_POP ({ if (vm.sp >= vm.fp) { failure("Stack check error during pop\n"); }; VM_ST_POP_UNCHECKED;  })
#define VM_ST_PUSH(value) ({ if (*vm.s_top == vm.sp - MAX_STACK_SIZE) { failure("Stack overflow\n"); }; *(--vm.sp) = value; })
#define VM_ST_REVERSE(elems_count) do {\
    int32_t *st = vm.sp;\
    int32_t *first_arg = st + elems_count - 1;\
    while (st < first_arg) {\
        swap(st++, first_arg--);\
    }\
} while(0)

#define VM_ST_FILL(n, value) do {\
    for (int i = 0; i < n; i++) {\
        VM_ST_PUSH(value);\
    }\
} while(0)

#define VM_ST_DROP_UNCHECKED(n) (vm.sp += n)

#define VM_ST_DROP(n) do { \
    VM_ST_DROP_UNCHECKED(n); \
    if (vm.sp > vm.fp) { \
        failure("Stack check error during drop"); \
    } \
} while(0)

void init_interpreter(ByteFile *bf, int32_t **stack_top, int32_t **stack_bottom) {
    *stack_top = malloc(MAX_STACK_SIZE * sizeof(int32_t)) + MAX_STACK_SIZE;
    if (*stack_top == NULL) {
        failure("Init: malloc failure");
    }
    *stack_bottom = *stack_top;
    vm.s_top = stack_top;
    vm.fp = *stack_bottom;
    vm.bf = bf;
    vm.sp = *stack_bottom;

    VM_ST_PUSH(0);
    VM_ST_PUSH(0);
    VM_ST_PUSH(2);
}

void free_interpreter() {
    free(vm.s_top - MAX_STACK_SIZE);
}

void jump_offset(int32_t new_ip) {
    vm.ip = vm.bf->code_ptr + new_ip;
}

int32_t *argument(int pos) {
    return vm.fp + pos + 3;
}

int32_t *local(int pos) {
    return vm.fp - pos - 1;
}

int32_t *from_closure(int pos) {
    int32_t n_args = *(vm.fp + 1);
    int32_t *current_closure = (int32_t *) *argument(n_args - 1);
    return (int32_t *) Belem_l(current_closure, BOX(pos + 1));
}

int32_t *get_by_location(char sb, int32_t value) {
    switch (sb) {
        case 0:
            return vm.bf->global_ptr + value;
        case 1:
            return local(value);
        case 2:
            return argument(value);
        case 3:
            return from_closure(value);
        default:
            failure("Invalid operation opcode");
    }
}

#define BINARY_OP(op) \
    do { \
        int b = UNBOX(VM_ST_POP); \
        int a = UNBOX(VM_ST_POP); \
        VM_ST_PUSH(BOX(a op b)); \
    } while (0)


void eval_bin_op(uint8_t bin_op_instr) {
    int bin_op = (bin_op_instr & 0x0F);
    switch (bin_op) {
        case OP_PLUS:
            BINARY_OP(+);
            break;
        case OP_MINUS:
            BINARY_OP(-);
            break;
        case OP_MUL:
            BINARY_OP(*);
            break;
        case OP_DIV:
            BINARY_OP(/);
            break;
        case OP_MOD:
            BINARY_OP(%);
            break;
        case OP_LT:
            BINARY_OP(<);
            break;
        case OP_LTEQ:
            BINARY_OP(<=);
            break;
        case OP_GT:
            BINARY_OP(>);
            break;
        case OP_GTEQ:
            BINARY_OP(>=);
            break;
        case OP_EQ:
            BINARY_OP(==);
            break;
        case OP_NEQ:
            BINARY_OP(!=);
            break;
        case OP_AND:
            BINARY_OP(&&);
            break;
        case OP_OR:
            BINARY_OP(||);
            break;
        default:
            failure("Invalid operator code");
    }
}

#undef BINARY_OP

void eval() {
#define READ_BYTE  (*vm.ip++)
#define READ_INT (vm.ip += sizeof(int), *(int*)(vm.ip - sizeof(int)))
#define READ_STRING get_string_from_table(vm.bf, READ_INT)
    vm.ip = vm.bf->code_ptr;

    do {
        uint8_t instruction = READ_BYTE;
        char first_bits = (char)((instruction & 0xF0) >> 4);
        char second_bits = (char)(instruction & 0x0F);
        if (first_bits == 15) {
            return;
        }

        if (first_bits == 0) { // BINOP
            eval_bin_op(instruction);
            continue;
        }

        if (first_bits == 2) { // LD
            int index = READ_INT;
            int32_t val = *get_by_location(second_bits, index);
            VM_ST_PUSH(val);
            continue;
        }

        if (first_bits == 3) { // LDA
            int index = READ_INT;
            int32_t val = (int32_t) get_by_location(second_bits, index);
            VM_ST_PUSH(val);
            continue;
        }

        if (first_bits == 4) { // ST
            int32_t index = READ_INT;
            int32_t val = VM_ST_POP;
            *get_by_location(second_bits, index) = val;
            VM_ST_PUSH(val);
            continue;
        }

        if (first_bits == 6) {
            int32_t *element = (int32_t *) VM_ST_POP;
            int32_t res = -1;
            switch (second_bits) {
                case PATT_STR: {
                    res = Bstring_patt(element, (int32_t *) VM_ST_POP);
                    break;
                }
                case PATT_TAG_STR: {
                    res = Bstring_tag_patt(element);
                    break;
                }
                case PATT_TAG_ARR: {
                    res = Barray_tag_patt(element);
                    break;
                }
                case PATT_TAG_SEXP: {
                    res = Bsexp_tag_patt(element);
                    break;
                }
                case PATT_BOXED: {
                    res = Bboxed_patt(element);
                    break;
                }
                case PATT_UNBOXED: {
                    res = Bunboxed_patt(element);
                    break;
                }
                case PATT_TAG_CLOSURE: {
                    res = Bclosure_tag_patt(element);
                    break;
                }
                default: {
                    failure("Unexpected pattern type");
                }
            }
            VM_ST_PUSH(res);
            continue;
        }

        switch (instruction) {
            case I_CONST: {
                VM_ST_PUSH(BOX(READ_INT));
                break;
            }
            case I_STRING: {
                VM_ST_PUSH((int32_t) Bstring(READ_STRING));
                break;
            }
            case I_SEXP: {
                char *s_exp_name = READ_STRING;
                int32_t s_exp_arity = READ_INT;
                int32_t s_exp_tag = LtagHash(s_exp_name);
                VM_ST_REVERSE(s_exp_arity);
                int32_t bs_exp = (int32_t) Bsexp_data(BOX(s_exp_arity + 1), s_exp_tag, vm.sp);
                VM_ST_DROP(s_exp_arity);
                VM_ST_PUSH(bs_exp);
                break;
            }
            case I_STI: {
                failure("Deprecated STI bytecode");
                break;
            }
            case I_STA: {
                void *val = (void *) VM_ST_POP;
                int32_t index = VM_ST_POP;
                if (IS_BOXED(index)) {
                    void *x = (void *) VM_ST_POP;
                    VM_ST_PUSH((int32_t) Bsta(val, index, x));
                } else {
                    VM_ST_PUSH((int32_t) Bsta(val, index, 0));
                }
                break;
            }
            case I_JMP: {
                jump_offset(READ_INT);
                break;
            }
            case I_CJMP_Z: {
                int32_t shift = READ_INT;
                int32_t cmp_val = UNBOX(VM_ST_POP);
                if (cmp_val == 0) {
                    jump_offset(shift);
                }
                break;
            }
            case I_CJMP_NZ: {
                int32_t shift = READ_INT;
                int32_t cmp_val = UNBOX(VM_ST_POP);
                if (cmp_val != 0) {
                    jump_offset(shift);
                }
                break;
            }
            case I_CLOSURE: {
                int32_t ip_shift = READ_INT;
                int32_t bn = READ_INT;
                int32_t binds[bn];
                for (int i = 0; i < bn; i++) {
                    char b = READ_BYTE;
                    int val = READ_INT;
                    binds[i] = *get_by_location(b, val);
                }
                VM_ST_PUSH((int32_t) Bclosure_values(BOX(bn), vm.bf->code_ptr + ip_shift, binds));
                break;
            }
            case I_ELEM: {
                int32_t index = VM_ST_POP;
                void *ptr = (void *) VM_ST_POP;
                VM_ST_PUSH((int32_t) Belem(ptr, index));
                break;
            }
            case I_BEGIN: {
                int n_args = READ_INT;
                int n_locals = READ_INT;

                VM_ST_PUSH((int32_t) vm.fp);
                vm.fp = vm.sp;
                VM_ST_FILL(n_locals, BOX(0));
                break;
            }
            case I_CBEGIN: {
                int n_args = READ_INT;
                int n_locals = READ_INT;

                VM_ST_PUSH((int32_t) vm.fp);
                vm.fp = vm.sp;
                VM_ST_FILL(n_locals, BOX(0));
                break;
            }
            case I_CALL: {
                int32_t call_shift = READ_INT;
                int32_t n_args = READ_INT;
                VM_ST_REVERSE(n_args);
                VM_ST_PUSH((int32_t) vm.ip);
                VM_ST_PUSH(n_args);
                jump_offset(call_shift);
                break;
            }
            case I_CALLC: {
                int32_t n_args = READ_INT;
                char *callee = (char *) Belem((int32_t *) vm.sp[n_args], BOX(0));
                VM_ST_REVERSE(n_args);
                VM_ST_PUSH((int32_t) vm.ip);
                VM_ST_PUSH(n_args + 1);
                vm.ip = callee;
                break;
            }
            case I_CALL_READ: {
                int r = Lread();
                VM_ST_PUSH(r);
                break;
            }
            case I_CALL_WRITE: {
                VM_ST_PUSH(Lwrite(VM_ST_POP));
                break;
            };
            case I_CALL_STRING: {
                VM_ST_PUSH((int32_t) Lstring((void *) VM_ST_POP));
                break;
            }
            case I_CALL_LENGTH: {
                VM_ST_PUSH(Llength((void *) VM_ST_POP));
                break;
            }
            case I_CALL_ARRAY: {
                int32_t arr_len = READ_INT;
                VM_ST_REVERSE(arr_len);
                int32_t res = (int32_t) Barray_data(BOX(arr_len), vm.sp);
                VM_ST_DROP(arr_len);
                VM_ST_PUSH(res);
                break;
            }
            case I_END: {
                int32_t ret_val = VM_ST_POP;
                vm.sp = vm.fp;
                vm.fp = (int32_t *) VM_ST_POP_UNCHECKED;
                int32_t n_args = VM_ST_POP;
                char *ret_addr = (char *) VM_ST_POP;
                VM_ST_DROP_UNCHECKED(n_args);
                VM_ST_PUSH(ret_val);
                vm.ip = ret_addr;
                break;
            }
            case I_DROP: {
                VM_ST_POP;
                break;
            }
            case I_DUP: {
                int32_t value = VM_ST_POP;
                VM_ST_FILL(2, value);
                break;
            }
            case I_TAG: {
                char *t_name = READ_STRING;
                int32_t n = READ_INT;
                int32_t t = LtagHash(t_name);
                void *d = (void *) VM_ST_POP;
                VM_ST_PUSH(Btag(d, t, BOX(n)));
                break;
            }
            case I_SWAP: {
                VM_ST_REVERSE(2);
                break;
            }
            case I_ARRAY: {
                int32_t arr_len = READ_INT;
                int32_t arr = (int32_t) Barray_patt((int32_t *) VM_ST_POP, BOX(arr_len));
                VM_ST_PUSH(arr);
                break;
            }
            case I_FAIL: {
                int32_t a = READ_INT;
                int32_t b = READ_INT;
                failure("Failed with FAIL %d %d", a, b);
                break;
            }
            case I_LINE: {
                READ_INT;
                break;
            }
            case I_RET: {
                failure("RET: undefined behaviour");
                break;
            }
            default: {
                failure("ERROR: invalid opcode %d-%d\n", first_bits, second_bits);
            }
        }
    } while (vm.ip != 0);
}

#undef READ_BYTE


int main(int argc, char *argv[]) {
    __init();
    ByteFile *f = read_file(argv[1]);
    init_interpreter(f, &__gc_stack_top, &__gc_stack_bottom);
    eval();
    return 0;
}

