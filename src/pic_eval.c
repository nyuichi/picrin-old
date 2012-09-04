#include "picrin.h"


static PicObj pic_exec(PicObj form, PicObj env, PicObj freevars, PicObj freenv);


static PicObj pic_exec_all(PicObj list, PicObj env, PicObj freevars, PicObj freeenv)
{
    if (PIC_NILP(list)) {
        return PIC_NIL;
    } else {
        PicObj car = pic_exec(PIC_CAR(list), env, freevars, freeenv);
        PicObj cdr = pic_exec_all(PIC_CDR(list), env, freevars, freeenv);
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



static PicObj pic_exec(PicObj form, PicObj env, PicObj freevars, PicObj freeenv)
{
    if (PIC_SYMBOLP(form)) {
        PicObj test = pic_memq(form, freevars);
        PicObj env_ = (PIC_FALSEP(test))? env : freeenv;
        PicObj pair = pic_env_get(form, env_);
        PicObj res = PIC_CDR(pair);
        PIC_XDECREF(test);
        PIC_XDECREF(pair);
        PIC_XINCREF(res);
        return res;
    } else if (PIC_SYNCLOP(form)) {
        return pic_exec(PIC_SYNCLO_BODY(form),
                        PIC_SYNCLO_FREEENV(form),
                        PIC_SYNCLO_FREEVARS(form),
                        env);
    } else if (!PIC_PAIRP(form)) {
        return form;
    } else {

        PicObj proc = pic_exec(PIC_CAR(form), env, freevars, freeenv);

        if (PIC_SYNTAXP(proc)) {
            int kind = PIC_SYNTAX_KIND(proc);
            PIC_DECREF(proc);
            
            switch (kind) {
            case PIC_SYNTAX_DEFINE: {
                PicObj var = PIC_CADR(form);
                PicObj val = pic_exec(PIC_CADDR(form), env, freevars, freeenv);
                pic_env_add(var, val, env);
                PIC_XDECREF(val);
                return PIC_VOID;
            }
            case PIC_SYNTAX_SET: {
                PicObj var = PIC_CADR(form);
                PicObj val = pic_exec(PIC_CADDR(form), env, freevars, freeenv);
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
                PicObj test = pic_exec(PIC_CADR(form), env, freevars, freeenv);
                PicObj succ = PIC_CADDR(form);
                PicObj fail = PIC_CADDDR(form);
                if (PIC_FALSEP(test)) {
                    PIC_XDECREF(test);
                    return pic_exec(fail, env, freevars, freeenv);
                } else {
                    PIC_XDECREF(test);
                    return pic_exec(succ, env, freevars, freeenv);
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
                    head = pic_exec(PIC_CAR(tail), env, freevars, freeenv);
                    tail = PIC_CDR(tail);
                }
                return head;
            }
            case PIC_SYNTAX_DEFSYNTAX: {
                PicObj name = PIC_CADR(form);
                PicObj transformer = pic_exec(PIC_CADDR(form), env, freevars, freeenv);
                PicObj macro = pic_make_macro(transformer, env);
                pic_env_add(name, macro, env);
                PIC_DECREF(transformer);
                PIC_DECREF(macro);
                return PIC_VOID;
            }

            }
        } else if (PIC_MACROP(proc)) {
            PicObj args_ = pic_list3(form, env, PIC_MACRO_MACENV(proc));
            PicObj form_ = pic_apply(PIC_MACRO_TRANSFORMER(proc), args_);
            PicObj res;

            perror("macro expand");
            pic_print(form_, curout);
            puts("");
            
            res = pic_exec(form_, env, freevars, freeenv);
            PIC_XDECREF(args_);
            PIC_XDECREF(form_);
            return res;
        } else {
            PicObj args = pic_exec_all(PIC_CDR(form), env, freevars, freeenv);
            PicObj res = pic_apply(proc, args);
            PIC_XDECREF(proc);
            PIC_XDECREF(args);
            return res;
        }
    }
}


PicObj pic_eval(PicObj form, PicObj env)
{
    return pic_exec(form, env, PIC_NIL, PIC_NIL);
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
    } else if (PIC_FOREIGNP(proc)) {
        PicObj (*callee)(PicObj args) = PIC_FOREIGN_FUNC(proc);
        return callee(args);
    } else {
        perror("Invalid application");
        abort();
    }
}
