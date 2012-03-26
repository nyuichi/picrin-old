#include "picrin.h"

int main(int argc, char ** argv)
{
    PicObj form, result, env;

    puts("Picrin Scheme 0.0.1 [" __DATE__ "]");

    pic_init();

    env = pic_scheme_report_environment();

    for (;;) {
        perror("env");
        pic_write(env, curout);
        perror("\nend\n");
        puts("");


        perror("intern_table");
        pic_write(intern_table, curout);
        perror("\nend\n");
        puts("");

        printf("> ");

        form = pic_read(curin);

        perror("read done");
        pic_write(form, curout);
        perror("\n");
        
    
        result = pic_eval(form, env);

        perror("eval done");
        pic_write(result, curout);
        perror("\n");
        
        pic_write(result, curout);

        printf("\n");

        PIC_XDECREF(form);
        PIC_XDECREF(result);
    }

    PIC_DECREF(env);
}
