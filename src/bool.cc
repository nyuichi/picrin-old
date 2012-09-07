#include "picrin.h"

#include <string.h>


bool pic_equalp(pic_val_t x, pic_val_t y)
{
  if (pic_eqp(x, y)) {
    return true;
  } else if (pic_stringp(x) && pic_stringp(y)) {
    return pic_string(x)->str == pic_string(y)->str;
  } else {
    return false;
  }
}
