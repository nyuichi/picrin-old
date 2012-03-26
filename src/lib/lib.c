#include "picrin.h"


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

PicObj pic_c_length(PicObj lst)
{
    if (PIC_NILP(lst)) {
        return PIC_TO_FIXNUM(0);
    } else {
        int c = 0;
        while (!PIC_NILP(lst)) {
            if (!PIC_PAIRP(lst)) {
                /*ERROR*/
                return PIC_TO_FIXNUM(0);
            } else {
                c++;
                lst = PIC_CDR(lst);
            }
        }
        return PIC_TO_FIXNUM(c);
    }
}

PicObj pic_c_list(PicObj args)
{
    PIC_XINCREF(args);
    return args;
}

PicObj pic_c_reverse(PicObj lst)
{
    if (PIC_NILP(lst) || !PIC_PAIRP(lst)) {
        return PIC_NIL;
    } else if (PIC_NILP(PIC_CDR(lst)) || !PIC_PAIRP(PIC_CDR(lst))) {
        return lst;
    } else {
        PicObj result = PIC_NIL;
        while (!PIC_NILP(lst)) {
            PicObj tmp = pic_make_pair(PIC_CAR(lst), result);

            PIC_XINCREF(tmp);
            PIC_XDECREF(result);
            result = tmp;		

            lst = PIC_CDR(lst);	
            PIC_XDECREF(tmp);
        }
        return result;
    }
}
