#include "picrin.h"

pic_obj_t pic_cons(pic_obj_t car, pic_obj_t cdr)
{
    return pic_make_pair(car, cdr);
}

pic_obj_t pic_acons(pic_obj_t key, pic_obj_t val, pic_obj_t alist)
{
    pic_obj_t car = pic_cons(key, val);
    pic_obj_t res = pic_cons(car, alist);
    PIC_DECREF(car);
    return res;
}

pic_obj_t pic_memq(pic_obj_t key, pic_obj_t list)
{
    if (PIC_NILP(list)) {
        return PIC_FALSE;
    } else {
        if (pic_eqp(key, PIC_CAR(list))) {
            pic_obj_t res = list;
            PIC_XINCREF(res);
            return res;
        } else {
            return pic_memq(key, PIC_CDR(list));
        }
    }
}

pic_obj_t pic_assq(pic_obj_t key, pic_obj_t alist)
{
    if (PIC_NILP(alist)) {
        return PIC_FALSE;
    } else {
        if (pic_eqp(key, PIC_CAAR(alist))) {
            pic_obj_t res = PIC_CAR(alist);
            PIC_XINCREF(res);
            return res;
        } else {
            return pic_assq(key, PIC_CDR(alist));
        }
    }
}

pic_obj_t pic_assoc(pic_obj_t key, pic_obj_t alist)
{
    pic_obj_t res;
    
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

pic_obj_t pic_list2(pic_obj_t obj1, pic_obj_t obj2)
{
    pic_obj_t pair1 = pic_cons(obj2, PIC_NIL);
    pic_obj_t pair2 = pic_cons(obj1, pair1);
    PIC_DECREF(pair1);
    return pair2;
}

pic_obj_t pic_list3(pic_obj_t obj1, pic_obj_t obj2, pic_obj_t obj3)
{
    pic_obj_t pair1 = pic_cons(obj3, PIC_NIL);
    pic_obj_t pair2 = pic_cons(obj2, pair1);
    pic_obj_t pair3 = pic_cons(obj1, pair2);
    PIC_DECREF(pair1);
    PIC_DECREF(pair2);
    return pair3;
}
