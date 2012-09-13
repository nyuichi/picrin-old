#include "picrin.h"

pic_val_t pic_env_new(pic_val_t parent)
{
  return pic_cons(pic_nil, parent);
}

pic_val_t pic_env_get(pic_val_t sym, pic_val_t env)
{
  if (pic_nilp(env)) {
    return pic_false;
  } else {
    pic_val_t obj = pic_assq(sym, pic_car(env));
        
    if (pic_falsep(obj)) {
      return pic_env_get(sym, pic_cdr(env)); 
    } else {
      return obj;
    }
  }
}

void pic_env_add(pic_val_t sym, pic_val_t val, pic_val_t env)
{
  pic_val_t pair = pic_acons(sym, val, pic_car(env));
  pic_pair(env)->car = pair;
}

void pic_env_set(pic_val_t sym, pic_val_t val, pic_val_t env)
{
  pic_val_t obj = pic_env_get(sym, env);
  pic_pair(obj)->cdr = val;
}

static int pic_env_level(pic_val_t sym, int level, pic_val_t env) {
  if (pic_nilp(pic_cdr(env))) {
    return -1;
  }
  else {
    pic_val_t obj = pic_assq(sym, pic_car(env));

    if (pic_falsep(obj)) {
      return pic_env_level(sym, level + 1, pic_cdr(env));
    }
    else {
      return level;
    }
  }
}

bool pic_env_freevarp(pic_val_t sym, pic_val_t env) {
  int level = pic_env_level(sym, 0, env);
  return level != 0 && level != -1;
}


// Built-in Libraries


#define arg1 pic_car(args)
#define arg2 pic_cadr(args)
#define pred(f) (f)? pic_true : pic_false

pic_val_t pic_c_eqp(pic_val_t args)
{
  return pred(arg1 == arg2);
}

pic_val_t pic_c_pairp(pic_val_t args)
{
  return pred(pic_pairp(arg1));
}

pic_val_t pic_c_symbolp(pic_val_t args)
{
  return pred(pic_symbolp(arg1));
}

pic_val_t pic_c_add(pic_val_t args)
{
  return arg1 + arg2;
}

pic_val_t pic_c_sub(pic_val_t args)
{
  return arg1 - arg2;
}

pic_val_t pic_c_eqn(pic_val_t args)
{
  return pred(arg1 == arg2);
}

pic_val_t pic_c_nullp(pic_val_t args)
{
  return pred(pic_nilp(arg1));
}

pic_val_t pic_c_car(pic_val_t args)
{
  return pic_car(arg1);
}

pic_val_t pic_c_cdr(pic_val_t args)
{
  return pic_cdr(arg1);
}

pic_val_t pic_c_cons(pic_val_t args)
{
  return pic_cons(arg1, arg2);
}

#define REGISTER_SYNTAX(x,y)                    \
  do {                                          \
    pic_val_t sym = pic_make_symbol(x);         \
    pic_env_add(sym, y, env);                   \
  } while (0)

#define REGISTER_CFUNC(x,y)                     \
  do {                                          \
    pic_val_t sym = pic_make_symbol(x);         \
    pic_val_t fun = pic_make_native(y);         \
    pic_env_add(sym, fun, env);                 \
  } while(0)


pic_val_t pic_minimal_environment() {
  pic_val_t env = pic_env_new(pic_nil);

  REGISTER_SYNTAX("define", pic_define_syntax);
  REGISTER_SYNTAX("set!", pic_set_syntax);
  REGISTER_SYNTAX("begin", pic_begin_syntax);
  REGISTER_SYNTAX("quote", pic_quote_syntax);
  REGISTER_SYNTAX("lambda", pic_lambda_syntax);
  REGISTER_SYNTAX("if", pic_if_syntax);

  return env;
}


pic_val_t pic_scheme_report_environment()
{
  pic_val_t env = pic_env_new(pic_nil);

  REGISTER_SYNTAX("define", pic_define_syntax);
  REGISTER_SYNTAX("set!", pic_set_syntax);
  REGISTER_SYNTAX("begin", pic_begin_syntax);
  REGISTER_SYNTAX("quote", pic_quote_syntax);
  REGISTER_SYNTAX("lambda", pic_lambda_syntax);
  REGISTER_SYNTAX("if", pic_if_syntax);

  REGISTER_CFUNC("eq?", pic_c_eqp);
  REGISTER_CFUNC("pair?", pic_c_pairp);
  REGISTER_CFUNC("symbol?", pic_c_symbolp);

  REGISTER_CFUNC("+", pic_c_add);
  REGISTER_CFUNC("-", pic_c_sub);
  REGISTER_CFUNC("=", pic_c_eqn);

  REGISTER_CFUNC("null?", pic_c_nullp);
  REGISTER_CFUNC("car", pic_c_car);
  REGISTER_CFUNC("cdr", pic_c_cdr);
  REGISTER_CFUNC("cons", pic_c_cons);
    
  return env;
}
