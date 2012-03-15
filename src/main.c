#include "picrin.h"

int main(int argc, char ** argv)
{
    PicInterp * pic = pic_new_interp();
    PicObj obj;
    

    puts("Picrin Scheme 0.0.1 [" __DATE__ "]");

    for (;;) {
        printf("> ");
        obj = pic_read(pic, pic->curin);
        obj = pic_eval(pic, obj, pic->topenv);
        pic_write(pic, obj, pic->curout);
        printf("\n");
    }
}
