#include "picrin.h"


static pic_obj_t pic_eval_all(pic_obj_t list, pic_obj_t env)
{
  if (PIC_NILP(list)) {
    return PIC_NIL;
  } else {
    pic_obj_t car = pic_eval(PIC_CAR(list), env);
    pic_obj_t cdr = pic_eval_all(PIC_CDR(list), env);
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



pic_obj_t pic_eval(pic_obj_t form, pic_obj_t env)
{
  //  printf("evaluating..."); pic_print(form, curout);

  if (PIC_SYMBOLP(form)) {
    pic_obj_t pair = pic_env_get(form, env);
    if (PIC_FALSEP(pair)) {
      printf("undefined symbol: ");
      pic_print(form, curout);
      abort();
    }
    pic_obj_t res = PIC_CDR(pair);
    PIC_XDECREF(pair);
    PIC_XINCREF(res);
    return res;
  } else if (!PIC_PAIRP(form)) {
    return form;
  } else {

    pic_obj_t proc = pic_eval(PIC_CAR(form), env);

    if (PIC_SYNTAXP(proc)) {
      int kind = PIC_SYNTAX_KIND(proc);
      PIC_DECREF(proc);
            
      switch (kind) {
        case PIC_SYNTAX_DEFINE: {
          pic_obj_t var = PIC_CADR(form);
          pic_obj_t val = pic_eval(PIC_CADDR(form), env);
          pic_env_add(var, val, env);
          PIC_XDECREF(val);
          return PIC_VOID;
        }
        case PIC_SYNTAX_SET: {
          pic_obj_t var = PIC_CADR(form);
          pic_obj_t val = pic_eval(PIC_CADDR(form), env);
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
          pic_obj_t test = pic_eval(PIC_CADR(form), env);
          pic_obj_t succ = PIC_CADDR(form);
          pic_obj_t fail = PIC_CADDDR(form);
          if (PIC_FALSEP(test)) {
            PIC_XDECREF(test);
            return pic_eval(fail, env);
          } else {
            PIC_XDECREF(test);
            return pic_eval(succ, env);
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
            head = pic_eval(PIC_CAR(tail), env);
            tail = PIC_CDR(tail);
          }
          return head;
        }
        default: {
          return -1;
        }
      }
    } else {
      pic_obj_t args = pic_eval_all(PIC_CDR(form), env);
      pic_obj_t res = pic_apply(proc, args);
      PIC_XDECREF(proc);
      PIC_XDECREF(args);
      return res;
    }
  }
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
