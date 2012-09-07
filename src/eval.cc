#include "picrin.h"


static pic_val_t pic_eval_all(pic_val_t list, pic_val_t env)
{
  if (pic_nilp(list)) {
    return pic_nil;
  }
  else {
    pic_val_t car, cdr;
    car = pic_eval(pic_car(list), env);
    cdr = pic_eval_all(pic_cdr(list), env);
    return pic_cons(car, cdr);
  }
}


static void pic_add_all(pic_val_t pars, pic_val_t args, pic_val_t env)
{
  if (pic_nilp(pars) && pic_nilp(args)) {
    return;
  }
  else if (pic_symbolp(pars)) {
    pic_env_add(pars, args, env);
  }
  else {
    pic_env_add(pic_car(pars), pic_car(args), env);
    pic_add_all(pic_cdr(pars), pic_cdr(args), env);
  }
}

pic_val_t pic_eval(pic_val_t form, pic_val_t env)
{
  // printf("evaluating..."); pic_print(form, curout);

  if (pic_symbolp(form)) {
    pic_val_t pair = pic_env_get(form, env);
    if (pic_falsep(pair)) {
      printf("undefined symbol: ");
      pic_print(form, current_error_port);
      abort();
    }
    return pic_cdr(pair);
  }
  else if (! pic_pairp(form)) {
    return form;
  }
  else {
    pic_val_t proc = pic_car(form);

    if (pic_symbolp(proc)) {
      if (pic_eqp(proc, pic_define_sym)) {
        pic_val_t var = pic_cadr(form);
        pic_val_t val = pic_eval(pic_caddr(form), env);
        pic_env_add(var, val, env);
        return pic_void;
      }
      else if (pic_eqp(proc, pic_set_sym)) {
        pic_val_t var = pic_cadr(form);
        pic_val_t val = pic_eval(pic_caddr(form), env);
        pic_env_set(var, val, env);
        return pic_void;
      }
      else if (pic_eqp(proc, pic_lambda_sym)) {
        pic_val_t params = pic_cadr(form);
        pic_val_t body = pic_caddr(form);
        return pic_make_closure(params, body, env);
      }
      else if (pic_eqp(proc, pic_if_sym)) {
        pic_val_t test = pic_eval(pic_cadr(form), env);
        if (pic_falsep(test)) {
          return pic_eval(pic_cadddr(form), env);
        }
        else {
          return pic_eval(pic_caddr(form), env);
        }
      }
      else if (pic_eqp(proc, pic_quote_sym)) {
        return pic_cadr(form);
      }
      else if (pic_eqp(proc, pic_begin_sym)) {
        pic_val_t exprs = pic_cdr(form), res = pic_void;
        while (! pic_nilp(exprs)) {
          res = pic_eval(pic_car(exprs), env);
          exprs = pic_cdr(exprs);
        }
        return res;
      }
    }

    proc = pic_eval(proc, env);
    return pic_apply(proc, pic_eval_all(pic_cdr(form), env));
  }
}


pic_val_t pic_apply(pic_val_t proc, pic_val_t args)
{
  if (pic_closurep(proc)) {
    pic_val_t env = pic_env_new(pic_closure(proc)->env);
    pic_add_all(pic_closure(proc)->args, args, env);
    return pic_eval(pic_closure(proc)->body, env);
  }
  else if (pic_nativep(proc)) {
    pic_val_t (*callee)(pic_val_t args) = pic_native(proc)->function;
    return callee(args);
  }
  else {
    perror("Invalid application");
    abort();
  }
}
