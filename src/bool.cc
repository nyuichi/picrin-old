#include "picrin.h"

#include <string.h>


bool pic_equalp(pic_val_t x, pic_val_t y)
{
  if (pic_eqp(x, y)) {
    return true;
  }
  else if (pic_stringp(x) && pic_stringp(y)) {
    return pic_string(x)->str == pic_string(y)->str;
  }
  else if (pic_pairp(x) && pic_pairp(y)) {
    return pic_equalp(pic_car(x), pic_car(y))
        && pic_equalp(pic_cdr(x), pic_cdr(y));
  }
  else {
    return false;
  }
}
