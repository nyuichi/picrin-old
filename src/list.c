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

PicObj pic_memq(PicObj key, PicObj list)
{
    if (PIC_NILP(list)) {
        return PIC_FALSE;
    } else {
        if (pic_eqp(key, PIC_CAR(list))) {
            PicObj res = list;
            PIC_XINCREF(res);
            return res;
        } else {
            return pic_memq(key, PIC_CDR(list));
        }
    }
}

PicObj pic_assq(PicObj key, PicObj alist)
{
    if (PIC_NILP(alist)) {
        return PIC_FALSE;
    } else {
        if (pic_eqp(key, PIC_CAAR(alist))) {
            PicObj res = PIC_CAR(alist);
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

PicObj pic_list2(PicObj obj1, PicObj obj2)
{
    PicObj pair1 = pic_cons(obj2, PIC_NIL);
    PicObj pair2 = pic_cons(obj1, pair1);
    PIC_DECREF(pair1);
    return pair2;
}

PicObj pic_list3(PicObj obj1, PicObj obj2, PicObj obj3)
{
    PicObj pair1 = pic_cons(obj3, PIC_NIL);
    PicObj pair2 = pic_cons(obj2, pair1);
    PicObj pair3 = pic_cons(obj1, pair2);
    PIC_DECREF(pair1);
    PIC_DECREF(pair2);
    return pair3;
}
