#include "picrin.h"


pic_obj_t pic_c_eqp(pic_obj_t args)
{
    return (PIC_CAR(args) == PIC_CADR(args))? PIC_TRUE : PIC_FALSE;
}

pic_obj_t pic_c_pairp(pic_obj_t args)
{
  return PIC_PAIRP(PIC_CAR(args))? PIC_TRUE : PIC_FALSE;
}

pic_obj_t pic_c_symbolp(pic_obj_t args)
{
  return PIC_SYMBOLP(PIC_CAR(args))? PIC_TRUE : PIC_FALSE;
}

pic_obj_t pic_c_add(pic_obj_t args)
{
    return PIC_CAR(args) + PIC_CADR(args) - 1;
}

pic_obj_t pic_c_sub(pic_obj_t args)
{
    return PIC_CAR(args) - PIC_CADR(args) + 1;
}

pic_obj_t pic_c_mul(pic_obj_t args)
{
    if (PIC_NILP(args)) {
        return PIC_TO_FIXNUM(1);
    } else {
        return PIC_TO_FIXNUM(PIC_FROM_FIXNUM(PIC_CAR(args))
                             * PIC_FROM_FIXNUM(pic_c_mul(PIC_CDR(args))));
    }
}

pic_obj_t pic_c_eqn(pic_obj_t args)
{
    return (PIC_CAR(args) == PIC_CADR(args))? PIC_TRUE : PIC_FALSE;
}

pic_obj_t pic_c_nullp(pic_obj_t args)
{
    return PIC_NILP(PIC_CAR(args))? PIC_TRUE : PIC_FALSE;
}

pic_obj_t pic_c_car(pic_obj_t args)
{
    pic_obj_t car = PIC_CAR(PIC_CAR(args));
    PIC_XINCREF(car);
    return car;
}

pic_obj_t pic_c_cdr(pic_obj_t args)
{
    pic_obj_t cdr = PIC_CDR(PIC_CAR(args));
    PIC_XINCREF(cdr);
    return cdr;
}

pic_obj_t pic_c_cons(pic_obj_t args)
{
    return pic_cons(PIC_CAR(args), PIC_CADR(args));
}

pic_obj_t pic_c_write(pic_obj_t args)
{
    pic_write(PIC_CAR(args), curout);
    return PIC_VOID;
}
