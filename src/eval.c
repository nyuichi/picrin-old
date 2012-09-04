#include "picrin.h"


static pic_obj_t pic_exec(pic_obj_t form, pic_obj_t env, pic_obj_t freevars, pic_obj_t freenv);


static pic_obj_t pic_exec_all(pic_obj_t list, pic_obj_t env, pic_obj_t freevars, pic_obj_t freeenv)
{
    if (PIC_NILP(list)) {
        return PIC_NIL;
    } else {
        pic_obj_t car = pic_exec(PIC_CAR(list), env, freevars, freeenv);
        pic_obj_t cdr = pic_exec_all(PIC_CDR(list), env, freevars, freeenv);
        pic_obj_t res = pic_cons(car, cdr);
        PIC_XDECREF(car);
        PIC_XDECREF(cdr);
        return res;
    }
}


static void pic_add_all(pic_obj_t pars, pic_obj_t args, pic_obj_t env)
{
    if (PIC_NILP(pars) && PIC_NILP(args)) {
        return;
    } else if (PIC_SYMBOLP(pars)) {
      pic_env_add(pars, args, env);
    } else {
        pic_env_add(PIC_CAR(pars), PIC_CAR(args), env);
        pic_add_all(PIC_CDR(pars), PIC_CDR(args), env);
    }
}



static pic_obj_t pic_exec(pic_obj_t form, pic_obj_t env, pic_obj_t freevars, pic_obj_t freeenv)
{
  //  printf("evaluating..."); pic_print(form, curout);

    if (PIC_SYMBOLP(form)) {
        pic_obj_t test = pic_memq(form, freevars);
        pic_obj_t env_ = (PIC_FALSEP(test))? env : freeenv;
        pic_obj_t pair = pic_env_get(form, env_);
        if (PIC_FALSEP(pair)) {
          printf("undefined symbol: ");
          pic_print(form, curout);
          abort();
        }
        pic_obj_t res = PIC_CDR(pair);
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

        pic_obj_t proc = pic_exec(PIC_CAR(form), env, freevars, freeenv);

        if (PIC_SYNTAXP(proc)) {
            int kind = PIC_SYNTAX_KIND(proc);
            PIC_DECREF(proc);
            
            switch (kind) {
            case PIC_SYNTAX_DEFINE: {
                pic_obj_t var = PIC_CADR(form);
                pic_obj_t val = pic_exec(PIC_CADDR(form), env, freevars, freeenv);
                pic_env_add(var, val, env);
                PIC_XDECREF(val);
                return PIC_VOID;
            }
            case PIC_SYNTAX_SET: {
                pic_obj_t var = PIC_CADR(form);
                pic_obj_t val = pic_exec(PIC_CADDR(form), env, freevars, freeenv);
                pic_env_set(var, val, env);
                PIC_XDECREF(val);
                return PIC_VOID;
            }
            case PIC_SYNTAX_LAMBDA: {
                pic_obj_t params = PIC_CADR(form);
                pic_obj_t body = PIC_CADDR(form);
                return pic_make_closure(params, body, env);
            }
            case PIC_SYNTAX_IF: {
                pic_obj_t test = pic_exec(PIC_CADR(form), env, freevars, freeenv);
                pic_obj_t succ = PIC_CADDR(form);
                pic_obj_t fail = PIC_CADDDR(form);
                if (PIC_FALSEP(test)) {
                    PIC_XDECREF(test);
                    return pic_exec(fail, env, freevars, freeenv);
                } else {
                    PIC_XDECREF(test);
                    return pic_exec(succ, env, freevars, freeenv);
                }
            }
            case PIC_SYNTAX_QUOTE: {
                pic_obj_t res = PIC_CADR(form);
                PIC_XINCREF(res);
                return res;
            }
            case PIC_SYNTAX_BEGIN: {
                pic_obj_t head = PIC_VOID;
                pic_obj_t tail = PIC_CDR(form);
                while (!PIC_NILP(tail)) {
                    PIC_XDECREF(head);
                    head = pic_exec(PIC_CAR(tail), env, freevars, freeenv);
                    tail = PIC_CDR(tail);
                }
                return head;
            }
            case PIC_SYNTAX_DEFSYNTAX: {
                pic_obj_t name = PIC_CADR(form);
                pic_obj_t transformer = pic_exec(PIC_CADDR(form), env, freevars, freeenv);
                pic_obj_t macro = pic_make_macro(transformer, env);
                pic_env_add(name, macro, env);
                PIC_DECREF(transformer);
                PIC_DECREF(macro);
                return PIC_VOID;
            }

            }
        } else if (PIC_MACROP(proc)) {
            pic_obj_t args_ = pic_list3(form, env, PIC_MACRO_MACENV(proc));
            pic_obj_t form_ = pic_apply(PIC_MACRO_TRANSFORMER(proc), args_);
            pic_obj_t res;

            printf("expanded from:\n");
            pic_print(form, curout);
            printf("to:\n");
            pic_print(form_, curout);
            puts("");
            
            res = pic_exec(form_, env, freevars, freeenv);
            PIC_XDECREF(args_);
            PIC_XDECREF(form_);
            return res;
        } else {
            pic_obj_t args = pic_exec_all(PIC_CDR(form), env, freevars, freeenv);
            pic_obj_t res = pic_apply(proc, args);
            PIC_XDECREF(proc);
            PIC_XDECREF(args);
            return res;
        }
    }
}


pic_obj_t pic_eval(pic_obj_t form, pic_obj_t env)
{
    return pic_exec(form, env, PIC_NIL, PIC_NIL);
}

pic_obj_t pic_apply(pic_obj_t proc, pic_obj_t args)
{
    if (PIC_CLOSUREP(proc)) {
        pic_obj_t env, res;
        env = pic_env_new(PIC_CLOSURE_ENV(proc));
        pic_add_all(PIC_CLOSURE_PARS(proc), args, env);
        res = pic_eval(PIC_CLOSURE_BODY(proc), env);
        PIC_XDECREF(env);
        return res;
    } else if (PIC_FOREIGNP(proc)) {
        pic_obj_t (*callee)(pic_obj_t args) = PIC_FOREIGN_FUNC(proc);
        return callee(args);
    } else {
        perror("Invalid application");
        abort();
    }
}
