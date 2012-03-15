#include "picrin.h"

PicObj pic_env_new(USE_PIC, PicObj parent)
{
    return pic_cons(pic, PIC_NIL, parent);
}

PicObj pic_env_get(USE_PIC, PicObj sym, PicObj env)
{
    PicObj obj;

    if (PIC_NILP(env)) {
        return PIC_FALSE;
    } else {
        obj = pic_assq(pic, sym, PIC_CAR(env));
        
        if (PIC_FALSEP(obj)) {
            return pic_env_get(pic, sym, PIC_CDR(env));
        } else {
            return obj;
        }
    }
}

void pic_env_add(USE_PIC, PicObj sym, PicObj val, PicObj env)
{
    PIC_CAR(env) = pic_acons(pic, sym, val, PIC_CAR(env));
}

void pic_env_set(USE_PIC, PicObj sym, PicObj val, PicObj env)
{
    PicObj obj = pic_env_get(pic, sym, env);
    PIC_CDR(obj) = val;
}



#define REGISTER_SYNTAX(x,y)                    \
    pic_env_add(pic, pic_make_symbol(pic, x),   \
                pic_make_syntax(pic, PIC_SYNTAX_##y, PIC_NIL), env)

#define REGISTER_CFUNC(x,y)                     \
    pic_env_add(pic, pic_make_symbol(pic, x),   \
                pic_make_cfunction(pic, y), env)


void pic_env_init(USE_PIC, PicObj env)
{
    REGISTER_SYNTAX("define", DEFINE);
    REGISTER_SYNTAX("lambda", LAMBDA);
    REGISTER_SYNTAX("if", IF);
    REGISTER_SYNTAX("quote", QUOTE);
    REGISTER_SYNTAX("begin", BEGIN);
    REGISTER_SYNTAX("set!", SET);

    REGISTER_CFUNC("-", pic_lib_sub);
    REGISTER_CFUNC("*", pic_lib_mul);
    REGISTER_CFUNC("=", pic_lib_eqn);
}
