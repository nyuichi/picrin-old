#include "picrin.h"

pic_val_t pic_acons(pic_val_t key, pic_val_t val, pic_val_t alist)
{
  return pic_cons(pic_cons(key, val), alist);
}

pic_val_t pic_memq(pic_val_t key, pic_val_t list)
{
  if (pic_nilp(list)) {
    return pic_false;
  }
  else {
    if (pic_eqp(key, pic_car(list))) {
      return list;
    }
    else {
      return pic_memq(key, pic_cdr(list));
    }
  }
}

pic_val_t pic_assq(pic_val_t key, pic_val_t alist)
{
  if (pic_nilp(alist)) {
    return pic_false;
  }
  else {
    if (pic_eqp(key, pic_caar(alist))) {
      return pic_car(alist);
    }
    else {
      return pic_assq(key, pic_cdr(alist));
    }
  }
}

pic_val_t pic_assoc(pic_val_t key, pic_val_t alist)
{
  if (pic_nilp(alist)) {
    return pic_false;
  }
  else {
    if (pic_equalp(key, pic_caar(alist))) {
      return pic_car(alist);
    }
    else {
      return pic_assoc(key, pic_cdr(alist));
    }
  }
}

bool pic_listp(pic_val_t val) {
  if (pic_nilp(val)) {
    return true;
  }
  else if (pic_pairp(val)) {
    return pic_listp(pic_cdr(val));
  }
  else {
    return false;
  }
}

static pic_val_t pic_reverse_(pic_val_t list, pic_val_t acc) {
  if (pic_nilp(list)) {
    return acc;
  }
  else {
    return pic_reverse_(pic_cdr(list), pic_cons(pic_car(list), acc));
  }
}

pic_val_t pic_reverse(pic_val_t list) {
  return pic_reverse_(list, pic_nil);
}

pic_val_t pic_append(pic_val_t list1, pic_val_t list2) {
  if (pic_nilp(list1)) {
    return list2;
  }
  else {
    return pic_cons(pic_car(list1), pic_append(pic_cdr(list1), list2));
  }
}

pic_val_t pic_delete_eq(pic_val_t x, pic_val_t list) {
  if (pic_nilp(list)) {
    return pic_nil;
  }
  else {
    if (pic_eqp(x, pic_car(list))) {
      return pic_delete_eq(x, pic_cdr(list));
    }
    else {
      return pic_cons(pic_car(list), pic_delete_eq(x, pic_cdr(list)));
    }
  }
}
