#include "picrin.h"


static PicObj pic_map_eval(USE_PIC, PicObj list, PicObj env)
{
    if (PIC_NILP(list)) {
        return PIC_NIL;
    } else {
        return pic_cons(pic, pic_eval(pic, PIC_CAR(list), env),
                         pic_map_eval(pic, PIC_CDR(list), env));
    }
}


static void pic_put_all(USE_PIC, PicObj pars, PicObj args, PicObj env)
{
    if (PIC_NILP(pars) && PIC_NILP(args)) {
        return;
    } else {
        pic_env_add(pic, PIC_CAR(pars), PIC_CAR(args), env);
        pic_put_all(pic, PIC_CDR(pars), PIC_CDR(args), env);
    }
}






PicObj pic_eval(USE_PIC, PicObj form, PicObj env)
{
    PicObj proc, fst, snd, trd;
    
    if (PIC_SYMBOLP(form)) {
        return PIC_CDR(pic_env_get(pic, form, env));
    } else if (!PIC_PAIRP(form)) {
        return form;
    } else {
        proc = pic_eval(pic, PIC_CAR(form), env);
        if (PIC_SYNTAXP(proc)) {
            switch (PIC_SYNTAX_KIND(proc)) {
            case PIC_SYNTAX_DEFINE: {
                fst = PIC_CADR(form);
                snd = pic_eval(pic, PIC_CADDR(form), env);
                pic_env_add(pic, fst, snd, env);
                return PIC_VOID;
            }
            case PIC_SYNTAX_SET: {
                fst = PIC_CADR(form);
                snd = pic_eval(pic, PIC_CADDR(form), env);
                pic_env_set(pic, fst, snd, env);
                return PIC_VOID;
            }
            case PIC_SYNTAX_LAMBDA: {
                fst = PIC_CADR(form);
                snd = PIC_CADDR(form);
                return pic_make_closure(pic, fst, snd, env);
            }
            case PIC_SYNTAX_IF: {
                fst = pic_eval(pic, PIC_CADR(form), env);
                snd = PIC_CADDR(form);
                trd = PIC_CADDDR(form);
                if (PIC_FALSEP(fst)) {
                    return pic_eval(pic, trd, env);
                } else {
                    return pic_eval(pic, snd, env);
                }
            }
            case PIC_SYNTAX_QUOTE: {
                return PIC_CADR(form);
            }
            case PIC_SYNTAX_BEGIN: {
                fst = PIC_VOID;
                snd = PIC_CDR(form);
                while (!PIC_NILP(snd)) {
                    fst = pic_eval(pic, PIC_CAR(snd), env);
                    snd = PIC_CDR(snd);
                }
                return fst;
            }

            }
        } else {
            return pic_apply(pic, proc, pic_map_eval(pic, PIC_CDR(form), env));
        }
    }
}



PicObj pic_apply(USE_PIC, PicObj proc, PicObj args)
{
    PicObj env;
    PicObj (*callee)(USE_PIC, PicObj args);
    
    if (PIC_CLOSUREP(proc)) {
        env = pic_env_new(pic, PIC_CLOSURE_ENV(proc));
        pic_put_all(pic, PIC_CLOSURE_PARS(proc), args, env);
        return pic_eval(pic, PIC_CLOSURE_BODY(proc), env);
    } else if (PIC_CFUNCTION(proc)) {
        callee = PIC_CFUNCTION_FUNC(proc);
        return callee(pic, args);
    }
}
