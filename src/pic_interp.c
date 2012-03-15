#include "picrin.h"

#include <stdlib.h>



PicInterp * pic_new_interp()
{
    int size = 10000;
    PicInterp * pic = malloc(size);
    pic->heap_size = size;
    pic->heap_from = malloc(size);
    pic->heap_to   = malloc(size);
    pic->heap_end  = pic->heap_from + size/PIC_WORD;
    pic->curin  = pic_make_port(pic, stdin, true, true);
    pic->curout = pic_make_port(pic, stdout, false, true);
    pic->curerr = pic_make_port(pic, stderr, false, true);
    pic->intern_table = PIC_NIL;
    pic->topenv = pic_env_new(pic, PIC_NIL);

    pic_env_init(pic, pic->topenv);
    
    return pic;
}

