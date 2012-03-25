#include "picrin.h"

int main(int argc, char ** argv)
{
    PicObj form, result, env;

    puts("Picrin Scheme 0.0.1 [" __DATE__ "]");

    env = pic_scheme_report_environment();

    for (;;) {
        printf("> ");

        form = pic_read(curin);
        result = pic_eval(form, env);
        pic_write(result, curout);

        printf("\n");

        PIC_DECREF(form);
        PIC_DECREF(result);
    }

    PIC_DECREF(env);
}
