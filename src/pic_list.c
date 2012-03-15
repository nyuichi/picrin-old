#include "picrin.h"

PicObj pic_cons(USE_PIC, PicObj car, PicObj cdr)
{
    return pic_make_pair(pic, car, cdr);
}

PicObj pic_acons(USE_PIC, PicObj key, PicObj val, PicObj alist)
{
    return pic_cons(pic, pic_cons(pic, key, val), alist);
}

PicObj pic_assq(USE_PIC, PicObj key, PicObj alist)
{
    if (PIC_NILP(alist)) {
        return PIC_FALSE;
    } else {
        if (pic_eqp(key, PIC_CAAR(alist))) {
            return PIC_CAR(alist);
        } else {
            return pic_assq(pic, key, PIC_CDR(alist));
        }
    }
}

PicObj pic_assoc(USE_PIC, PicObj key, PicObj alist)
{
    if (PIC_NILP(alist)) {
        return PIC_FALSE;
    } else {
        if (pic_equalp(key, PIC_CAAR(alist))) {
            return PIC_CAR(alist);
        } else {
            return pic_assoc(pic, key, PIC_CDR(alist));
        }
    }
}
