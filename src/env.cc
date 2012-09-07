#include "picrin.h"
#include "lib.h"

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


#define REGISTER_CFUNC(x,y)                     \
  do {                                          \
    pic_val_t sym = pic_make_symbol(x);         \
    pic_val_t fun = pic_make_native(y);         \
    pic_env_add(sym, fun, env);                 \
  } while(0)


pic_val_t pic_scheme_report_environment()
{
  pic_val_t env = pic_env_new(pic_nil);

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
