#include "picrin.h"

bool pic_env_globalp(pic_obj_t env)
{
  return PIC_NILP(PIC_ENV_PARENT(env));
}

pic_obj_t pic_env_get(pic_obj_t key, pic_obj_t env)
{
  if (PIC_NILP(env)) {
    return PIC_FALSE;
  } else {
    pic_obj_t obj = pic_assq(key, PIC_ENV_TABLE(env));
    if (PIC_FALSEP(obj)) {
      return pic_env_get(key, PIC_ENV_PARENT(env)); 
    } else {
      return obj;
    }
  }
}

void pic_env_def(pic_obj_t key, pic_obj_t val, pic_obj_t env)
{
  pic_obj_t old_env = PIC_ENV_TABLE(env);
  pic_obj_t new_env = pic_acons(key, val, old_env);
  PIC_XDECREF(old_env);
  PIC_XINCREF(new_env);
  PIC_ENV_TABLE(env) = new_env;
}

void pic_env_set(pic_obj_t key, pic_obj_t val, pic_obj_t env)
{
  pic_obj_t obj = pic_env_get(key, env);
  PIC_XINCREF(val);
  PIC_XDECREF(PIC_CDR(obj));
  PIC_CDR(obj) = val;
  PIC_XDECREF(obj);
}


#define REGISTER_SYNTAX(x,y)				\
  do {							\
    pic_obj_t sym = pic_make_symbol(x);			\
    pic_obj_t stx = pic_make_syntax(PIC_SYNTAX_##y);	\
    pic_env_add(sym, stx, env);				\
    PIC_DECREF(sym);					\
    PIC_DECREF(stx);					\
  } while(0)

#define REGISTER_CFUNC(x,y)			\
  do {						\
    pic_obj_t sym = pic_make_symbol(x);		\
    pic_obj_t fun = pic_make_foreign(y);		\
    pic_env_add(sym, fun, env);			\
    PIC_DECREF(sym);				\
    PIC_DECREF(fun);				\
  } while(0)


/* pic_obj_t pic_scheme_report_environment() */
/* { */
/*   pic_obj_t env = pic_env_new(PIC_NIL); */

/*   REGISTER_SYNTAX("define", DEFINE); */
/*   REGISTER_SYNTAX("lambda", LAMBDA); */
/*   REGISTER_SYNTAX("if", IF); */
/*   REGISTER_SYNTAX("quote", QUOTE); */
/*   REGISTER_SYNTAX("begin", BEGIN); */
/*   REGISTER_SYNTAX("set!", SET); */
/*   REGISTER_SYNTAX("define-syntax", DEFSYNTAX); */

/*   REGISTER_CFUNC("eq?", pic_c_eqp); */

/*   REGISTER_CFUNC("+", pic_c_add); */
/*   REGISTER_CFUNC("-", pic_c_sub); */
/*   REGISTER_CFUNC("*", pic_c_mul); */
/*   REGISTER_CFUNC("=", pic_c_eqn); */

/*   REGISTER_CFUNC("null?", pic_c_nullp); */
/*   REGISTER_CFUNC("car", pic_c_car); */
/*   REGISTER_CFUNC("cdr", pic_c_cdr); */
/*   REGISTER_CFUNC("cons", pic_c_cons); */
    
/*   REGISTER_CFUNC("make-syntactic-closure", pic_c_make_synclo); */

/*   REGISTER_CFUNC("write", pic_c_write); */

/*   return env; */
/* } */
