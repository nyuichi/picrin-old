#include "picrin.h"

#define debug(msg, obj)                         \
    do {                                        \
    perror(msg);                                \
    pic_write(obj, curout);                     \
    puts("\n");                                 \
    fflush(stdout);                             \
    } while(0)


#if 0

void load_init(char * filename, pic_obj_t env)
{
    pic_obj_t form, result, port;
    FILE * file;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("failed to open init file");
        abort();
    }

    port = pic_make_port(file, true, true);
    for (;;) {
        form = pic_read(port);
        if (PIC_VOIDP(form)) break;
        result = pic_eval(form, env);
        PIC_XDECREF(form);
        PIC_XDECREF(result);
    }
}


int main(int argc, char ** argv)
{
    pic_obj_t form, result, env, port;

    puts("Picrin Scheme 0.0.1 [" __DATE__ "]");

    /* Entering */
    pic_init();
    env = pic_scheme_report_environment();

    /* Load initial file */
    if (argc < 2) {
        fputs("run:./picrin <init file>\n", stderr);
        abort();
    }
    puts("loading init...");
    load_init(argv[1], env);

    /* Run loop */
    for (;;) {
        printf("> ");

        form = pic_read(curin);
        if (PIC_VOIDP(form)) break;

        result = pic_eval(form, env);
        pic_write(result, curout);

        puts("");

        PIC_XDECREF(form);
        PIC_XDECREF(result);
    }

    /* Leaving */
    puts("good-bye");
    PIC_DECREF(env);
    PIC_DECREF(port);
}

#endif

int main() {
  pic_init();
  for (;;) {
    pic_print(pic_read(current_input_port), current_output_port);
  }
}
