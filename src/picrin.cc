#include "picrin.h"

#define debug(msg, obj)                         \
    do {                                        \
    perror(msg);                                \
    pic_write(obj, curout);                     \
    puts("\n");                                 \
    fflush(stdout);                             \
    } while(0)


void load_init(char * filename, pic_val_t env)
{
    pic_val_t form, result, port;
    FILE * file;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("failed to open init file");
        abort();
    }

    port = pic_make_port(file, true, true);
    for (;;) {
        form = pic_read(port);
        if (pic_voidp(form)) break;
        result = pic_eval(form, env);
    }
}


int main(int argc, char ** argv)
{
    pic_val_t form, result, env;

    puts("Picrin Scheme " PICRIN_VERSION " [" __DATE__ "]");

    /* Entering */
    pic_init();
    env = pic_scheme_report_environment();

    /* Load initial file */
    if (argc < 2) {
        fputs("run: picrin <init file>\n", stderr);
        abort();
    }
    puts("loading init...");
    //    load_init(argv[1], env);

    /* Run loop */
    for (;;) {
        printf("> ");

        form = pic_read();
        if (pic_voidp(form)) break;

        result = pic_eval(form, env);
        pic_print(result);
    }

    /* Leaving */
    puts("good-bye");
}

