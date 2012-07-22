#include "picrin.h"

pic_obj_t pic_cons(pic_obj_t car, pic_obj_t cdr)
{
  return pic_make_pair(car, cdr);
}

bool pic_listp(pic_obj_t obj)
{
  if (PIC_NILP(obj)) {
    return true;
  } else if (PIC_PAIRP(obj)) {
    return pic_listp(PIC_CDR(obj));
  } else {
    return false;
  }
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

pic_obj_t pic_list1(pic_obj_t obj1)
{
  return pic_cons(obj1, PIC_NIL);
}

pic_obj_t pic_list2(pic_obj_t obj1, pic_obj_t obj2)
{
  pic_obj_t tail = pic_list1(obj2);
  pic_obj_t list = pic_cons(obj1, tail);
  PIC_DECREF(tail);
  return list;
}

pic_obj_t pic_list3(pic_obj_t obj1, pic_obj_t obj2, pic_obj_t obj3)
{
  pic_obj_t tail = pic_list2(obj2, obj3);
  pic_obj_t list = pic_cons(obj1, tail);
  PIC_DECREF(tail);
  return list;
}

pic_obj_t pic_list4(pic_obj_t obj1, pic_obj_t obj2, pic_obj_t obj3, pic_obj_t obj4)
{
  pic_obj_t tail = pic_list3(obj2, obj3, obj4);
  pic_obj_t list = pic_cons(obj1, tail);
  PIC_DECREF(tail);
  return list;
}
