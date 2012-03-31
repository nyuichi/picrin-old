#include "picrin.h"


PicObj pic_c_eqp(PicObj args)
{
    return (PIC_CAR(args) == PIC_CADR(args))? PIC_TRUE : PIC_FALSE;
}

PicObj pic_c_add(PicObj args)
{
    return PIC_CAR(args) + PIC_CADR(args) - 1;
}

PicObj pic_c_sub(PicObj args)
{
    return PIC_CAR(args) - PIC_CADR(args) + 1;
}

PicObj pic_c_mul(PicObj args)
{
    if (PIC_NILP(args)) {
        return PIC_TO_FIXNUM(1);
    } else {
        return PIC_TO_FIXNUM(PIC_FROM_FIXNUM(PIC_CAR(args))
                             * PIC_FROM_FIXNUM(pic_c_mul(PIC_CDR(args))));
    }
}

PicObj pic_c_eqn(PicObj args)
{
    return (PIC_CAR(args) == PIC_CADR(args))? PIC_TRUE : PIC_FALSE;
}

PicObj pic_c_nullp(PicObj args)
{
    return PIC_NILP(PIC_CAR(args))? PIC_TRUE : PIC_FALSE;
}

PicObj pic_c_make_synclo(PicObj args)
{
    return pic_make_synclo(PIC_CAR(args), PIC_CADR(args), PIC_CADDR(args));
}

PicObj pic_c_car(PicObj args)
{
    PicObj car = PIC_CAR(PIC_CAR(args));
    PIC_XINCREF(car);
    return car;
}

PicObj pic_c_cdr(PicObj args)
{
    PicObj cdr = PIC_CDR(PIC_CAR(args));
    PIC_XINCREF(cdr);
    return cdr;
}

PicObj pic_c_cons(PicObj args)
{
    return pic_cons(PIC_CAR(args), PIC_CADR(args));
}

PicObj pic_c_write(PicObj args)
{
    pic_write(PIC_CAR(args), curout);
    return PIC_VOID;
}
