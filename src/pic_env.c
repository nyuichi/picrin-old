#include "picrin.h"

PicObj pic_env_new(PicObj parent)
{
    return pic_cons(PIC_NIL, parent);
}

PicObj pic_env_get(PicObj sym, PicObj env)
{
    PicObj obj;

    if (PIC_NILP(env)) {
        return PIC_FALSE;
    } else {
        obj = pic_assq(sym, PIC_CAR(env));
        
        if (PIC_FALSEP(obj)) {
            PIC_XDECREF(obj);
            return pic_env_get(sym, PIC_CDR(env)); 
        } else {
            return obj;
        }
    }
}

void pic_env_add(PicObj sym, PicObj val, PicObj env)
{
    /* Implicit incref and decref */
    PIC_CAR(env) = pic_acons(sym, val, PIC_CAR(env));
}

void pic_env_set(PicObj sym, PicObj val, PicObj env)
{
    PicObj obj = pic_env_get(sym, env);
    PIC_XINCREF(val);
    PIC_XDECREF(PIC_CDR(obj));
    PIC_CDR(obj) = val;
    PIC_XDECREF(obj);
}


#define REGISTER_SYNTAX(x,y)                                       \
    do {                                                           \
        PicObj sym = pic_make_symbol(x);                           \
        PicObj stx = pic_make_syntax(PIC_SYNTAX_##y, PIC_NIL);     \
        pic_env_add(sym, stx, env);                                \
        PIC_DECREF(sym);                                           \
        PIC_DECREF(stx);                                           \
    } while(0)

#define REGISTER_CFUNC(x,y)                        \
    do {                                           \
        PicObj sym = pic_make_symbol(x);           \
        PicObj fun = pic_make_cfunction(y);        \
        pic_env_add(sym, fun, env);                \
        PIC_DECREF(sym);                           \
        PIC_DECREF(fun);                           \
    } while(0)


PicObj pic_scheme_report_environment()
{
    PicObj env = pic_env_new(PIC_NIL);

    REGISTER_SYNTAX("define", DEFINE);
    REGISTER_SYNTAX("lambda", LAMBDA);
    REGISTER_SYNTAX("if", IF);
    REGISTER_SYNTAX("quote", QUOTE);
    REGISTER_SYNTAX("begin", BEGIN);
    REGISTER_SYNTAX("set!", SET);

    REGISTER_CFUNC("-", pic_c_sub);
    REGISTER_CFUNC("*", pic_c_mul);
    REGISTER_CFUNC("=", pic_c_eqn);

    return env;
}
