#include "picrin.h"

PicObj pic_lib_sub(USE_PIC, PicObj args)
{
    return PIC_CAR(args) - PIC_CADR(args) + 1;
}

PicObj pic_lib_mul(USE_PIC, PicObj args)
{
    if (PIC_NILP(args)) {
        return PIC_TO_FIXNUM(1);
    } else {
        return PIC_TO_FIXNUM(PIC_FROM_FIXNUM(PIC_CAR(args))
                             * PIC_FROM_FIXNUM(pic_lib_mul(pic, PIC_CDR(args))));
    }
}

PicObj pic_lib_eqn(USE_PIC, PicObj args)
{
    return (PIC_CAR(args) == PIC_CADR(args))?
        PIC_TRUE : PIC_FALSE;
}


