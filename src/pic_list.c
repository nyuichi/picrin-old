#include "picrin.h"

PicObj pic_cons(PicObj car, PicObj cdr)
{
    return pic_make_pair(car, cdr);
}

PicObj pic_acons(PicObj key, PicObj val, PicObj alist)
{
    PicObj car = pic_cons(key, val);
    PicObj res = pic_cons(car, alist);
    PIC_DECREF(car);
    return res;
}

PicObj pic_assq(PicObj key, PicObj alist)
{
    PicObj res;
    
    if (PIC_NILP(alist)) {
        return PIC_FALSE;
    } else {
        if (pic_eqp(key, PIC_CAAR(alist))) {
            res = PIC_CAR(alist);
            PIC_XINCREF(res);
            return res;
        } else {
            return pic_assq(key, PIC_CDR(alist));
        }
    }
}

PicObj pic_assoc(PicObj key, PicObj alist)
{
    PicObj res;
    
    if (PIC_NILP(alist)) {
        return PIC_FALSE;
    } else {
        if (pic_equalp(key, PIC_CAAR(alist))) {
            res = PIC_CAR(alist);
            PIC_XINCREF(res);
            return PIC_CAR(alist);
        } else {
            return pic_assoc(key, PIC_CDR(alist));
        }
    }
}
