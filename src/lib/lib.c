#include "picrin.h"

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


