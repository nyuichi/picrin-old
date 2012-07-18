#include <picrin.h>

enum OP_CODE {
    OP_PUSH,
    OP_CALL,
    OP_TAILCALL,
    OP_RETURN,
    OP_GREF,
    OP_GSET,
    OP_LREF,
    OP_LSET,
    OP_CREF,
    OP_CSET,
};

typedef struct PicBytecode {
    char * begin;
    char * end;
    char * current;
} PicBytecode;


#define emit_push(obj) \
    *bc++ = OP_CALL; \
    emit_word(obj);


void compile(PicObj expr, PicObj env)
{
    if (PIC_POINTERP(expr) && PIC_TYPEOF(expr) == PIC_TYPE_PAIR) {
        if (!PIC_LISTP(expr)) {
            /* throw error */
        }
        if (PIC_TYPEOF(PIC_CAR(expr)) == PIC_SYMBOL_TYPE) {
            PicObj proc = pic_env_get(PIC_CAR(expr), env);
            if (PIC_NILP(proc)) {
                puts("symbol is not defined!");
                /* write later */
            } else {
                if (PIC_TYPEOF(proc) == PIC_SYNTAX_TYPE) {
                    switch (PIC_SYNTAX_KIND(proc)) {
                    case PIC_SYNTAX_DEFINE:
                    case PIC_SYNTAX_SET:
                    case PIC_SYNTAX_LAMBDA:
                    case PIC_SYNTAX_IF:
                    case PIC_SYNTAX_QUOTE:
                    case PIC_SYNTAX_BEGIN:
                    case PIC_SYNTAX_DEFSYNTAX: 
                    case PIC_SYNTAX_USER:
                    }
                } else {
                    PicObj list = pic_reverse(expr);
                    
                }
            }
        }
    } else {
        /* throw error when expr is a macro or a syntax object or things like that */
        emit_push(expr);
    }
}

void run(PicBytecode * bytecode)
{
    char * bc = bytecode->begin;

    switch (*bc++) {
    case OP_CALL:
    case OP_TAILCAL:
    case OP_RETURN,
    case OP_GREF,
    case OP_GSET,
    case OP_LREF,
    case OP_LSET,
    case OP_CREF,
    case OP_CSET,
    }
}

void test_vm()
{
}

int main()
{
    test_vm();
}
