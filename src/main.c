#include "picrin.h"

#define debug(msg, obj)                         \
    do {                                        \
    perror(msg);                                \
    pic_write(obj, curout);                     \
    puts("\n");                                 \
    fflush(stdout);                             \
    } while(0)


int main(int argc, char ** argv)
{
    PicObj form, result, env;

    puts("Picrin Scheme 0.0.1 [" __DATE__ "]");

    pic_init();

    env = pic_scheme_report_environment();

    for (;;) {
        
        printf("> ");

        form = pic_read(curin);
        result = pic_eval(form, env);
        pic_write(result, curout);

        printf("\n");

        PIC_XDECREF(form);
        PIC_XDECREF(result);
    }

    PIC_DECREF(env);
}
