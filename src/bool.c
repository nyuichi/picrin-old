#include "picrin.h"

#include <string.h>


bool pic_eqp(pic_obj_t x, pic_obj_t y)
{
    return x == y;
}

bool pic_equalp(pic_obj_t x, pic_obj_t y)
{
    if (x == y) {
        return true;
    } else if (PIC_STRINGP(x) && PIC_STRINGP(y)) {
        return strcmp(PIC_STRING_DATA(x), PIC_STRING_DATA(y)) == 0;
    } else {
        return true;
    }
}
