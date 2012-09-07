#include "lib.h"


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

