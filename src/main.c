#include "picrin.h"

#define debug(msg, obj)                         \
    do {                                        \
      perror(msg);				\
      pic_write(obj, curout);			\
      puts("\n");				\
      fflush(stdout);				\
    } while(0)


/* void load_init(char * filename, pic_obj_t env) */
/* { */
/*     pic_obj_t form, result, port; */
/*     FILE * file; */
    
/*     file = fopen(filename, "r"); */
/*     if (file == NULL) { */
/*         perror("failed to open init file"); */
/*         abort(); */
/*     } */

/*     port = pic_make_port(file, true, true); */
/*     for (;;) { */
/*         form = pic_read(port); */
/*         if (PIC_VOIDP(form)) break; */
/*         result = pic_eval(form, env); */
/*         PIC_XDECREF(form); */
/*         PIC_XDECREF(result); */
/*     } */
/* } */


/* int mainx(int argc, char ** argv) */
/* { */
/*     pic_obj_t form, result, env, port; */

/*     puts("Picrin Scheme 0.0.1 [" __DATE__ "]"); */

/*     /\* Entering *\/ */
/*     pic_init(); */
/*     env = pic_scheme_report_environment(); */

/*     /\* Load initial file *\/ */
/*     if (argc < 2) { */
/*         fputs("run:./picrin <init file>\n", stderr); */
/*         abort(); */
/*     } */
/*     puts("loading init..."); */
/*     load_init(argv[1], env); */

/*     /\* Run loop *\/ */
/*     for (;;) { */
/*         printf("> "); */

/*         form = pic_read(standard_input_port); */
/*         if (PIC_VOIDP(form)) break; */

/*         result = pic_eval(form, env); */
/*         pic_write(result, standard_output_port); */

/*         puts(""); */

/*         PIC_XDECREF(form); */
/*         PIC_XDECREF(result); */
/*     } */

/*     /\* Leaving *\/ */
/*     puts("good-bye"); */
/*     PIC_DECREF(env); */
/*     PIC_DECREF(port); */
/* } */

extern pic_obj_t resolve(pic_obj_t expr, pic_obj_t env);
extern void print_ast(pic_obj_t ast);

int main()
{
  pic_obj_t env, form, result;

  pic_init();
  env = pic_make_env(PIC_NIL, PIC_NIL);

  pic_env_def(pic_get_interned_symbol("quote"), PIC_SYNTAX_QUOTE, env);
  pic_env_def(pic_get_interned_symbol("if"), PIC_SYNTAX_IF, env);
  pic_env_def(pic_get_interned_symbol("begin"), PIC_SYNTAX_BEGIN, env);
  pic_env_def(pic_get_interned_symbol("define"), PIC_SYNTAX_DEFINE, env);
  pic_env_def(pic_get_interned_symbol("lambda"), PIC_SYNTAX_LAMBDA, env);
  pic_env_def(pic_get_interned_symbol("set!"), PIC_SYNTAX_SET, env);

  for (;;) {
    form = pic_read(standard_input_port);

    pic_print(form, standard_output_port);

    result = resolve(form, env);

    perror("successfully resolved");

    print_ast(result);

    pic_newline(standard_output_port);
  }

  return 0;
}
