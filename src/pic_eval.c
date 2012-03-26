#include "picrin.h"


static PicObj pic_eval_all(PicObj list, PicObj env)
{
    if (PIC_NILP(list)) {
        return PIC_NIL;
    } else {
        PicObj car = pic_eval(PIC_CAR(list), env);
        PicObj cdr = pic_eval_all(PIC_CDR(list), env);
        PicObj res = pic_cons(car, cdr);
        PIC_XDECREF(car);
        PIC_XDECREF(cdr);
        return res;
    }
}


static void pic_add_all(PicObj pars, PicObj args, PicObj env)
{
    if (PIC_NILP(pars) && PIC_NILP(args)) {
        return;
    } else {
        pic_env_add(PIC_CAR(pars), PIC_CAR(args), env);
        pic_add_all(PIC_CDR(pars), PIC_CDR(args), env);
    }
}






PicObj pic_eval(PicObj form, PicObj env)
{
    if (PIC_SYMBOLP(form)) {
        PicObj pair = pic_env_get(form, env);
        PicObj res = PIC_CDR(pair);
        PIC_XINCREF(res);
        PIC_DECREF(pair);
        return res;
    } else if (!PIC_PAIRP(form)) {
        return form;
    } else {

        PicObj proc = pic_eval(PIC_CAR(form), env);

        if (PIC_SYNTAXP(proc)) {
            int kind = PIC_SYNTAX_KIND(proc);
            PIC_DECREF(proc);
            
            switch (kind) {
            case PIC_SYNTAX_DEFINE: {
                PicObj var = PIC_CADR(form);
                PicObj val = pic_eval(PIC_CADDR(form), env);
                pic_env_add(var, val, env);
                PIC_XDECREF(val);
                return PIC_VOID;
            }
            case PIC_SYNTAX_SET: {
                PicObj var = PIC_CADR(form);
                PicObj val = pic_eval(PIC_CADDR(form), env);
                pic_env_set(var, val, env);
                PIC_XDECREF(val);
                return PIC_VOID;
            }
            case PIC_SYNTAX_LAMBDA: {
                PicObj params = PIC_CADR(form);
                PicObj body = PIC_CADDR(form);
                return pic_make_closure(params, body, env);
            }
            case PIC_SYNTAX_IF: {
                PicObj test = pic_eval(PIC_CADR(form), env);
                PicObj succ = PIC_CADDR(form);
                PicObj fail = PIC_CADDDR(form);
                if (PIC_FALSEP(test)) {
                    PIC_XDECREF(test);
                    return pic_eval(fail, env);
                } else {
                    PIC_XDECREF(test);
                    return pic_eval(succ, env);
                }
            }
            case PIC_SYNTAX_QUOTE: {
                PicObj res = PIC_CADR(form);
                PIC_XINCREF(res);
                return res;
            }
            case PIC_SYNTAX_BEGIN: {
                PicObj head = PIC_VOID;
                PicObj tail = PIC_CDR(form);
                while (!PIC_NILP(tail)) {
                    PIC_XDECREF(head);
                    head = pic_eval(PIC_CAR(tail), env);
                    tail = PIC_CDR(tail);
                }
                return head;
            }

            }
        } else {
            PicObj args = pic_eval_all(PIC_CDR(form), env);
            PicObj res = pic_apply(proc, args);
            PIC_XDECREF(proc);
            PIC_XDECREF(args);
            return res;
        }
    }
}



PicObj pic_apply(PicObj proc, PicObj args)
{
    if (PIC_CLOSUREP(proc)) {
        PicObj env, res;
        env = pic_env_new(PIC_CLOSURE_ENV(proc));
        pic_add_all(PIC_CLOSURE_PARS(proc), args, env);
        res = pic_eval(PIC_CLOSURE_BODY(proc), env);
        PIC_XDECREF(env);
        return res;
    } else if (PIC_CFUNCTION(proc)) {
        PicObj (*callee)(PicObj args) = PIC_CFUNCTION_FUNC(proc);
        return callee(args);
    }
}
