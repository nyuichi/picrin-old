#include "picrin.h"


enum pic_opcode {
  OP_PUSH = 0x03,
  OP_CALL = 0x13,
  OP_GREF = 0x23,
};

#if 0

(+ 1 2)

((OP_PUSH 2)
 (OP_PUSH 1)
 (OP_GREF box_+)
 (OP_CALL 2))

#endif


pic_val_t pic_generate(pic_val_t form, pic_val_t env, pic_val_t code) {
  if (pic_symbolp(form)) {
    pic_val_t cmd = pic_list(OP_GREF, pic_env_get(form, env));
    return pic_cons(cmd, code);
  }
  else if (! pic_pairp(form)) {
    pic_val_t cmd = pic_list(OP_PUSH, form);
    return pic_cons(cmd, code);
  }
  else if (pic_listp(form)) {
    int n = -1;
    form = pic_reverse(form);

    while (! pic_nilp(form)) {
      code = pic_generate(pic_car(form), env, code);
      form = pic_cdr(form);
      ++n;
    }
    
    pic_val_t cmd = pic_list(OP_CALL, n);
    return pic_cons(cmd, code);
  }
  else {
    perror("dotted list in source");
    abort();
  }
}

pic_val_t pic_compile(pic_val_t form, pic_val_t env) {
  return pic_reverse(pic_generate(form, env, pic_nil));
}


pic_val_t pic_unbox(pic_val_t box) {
  return pic_cdr(box);
}

pic_val_t pic_box_name(pic_val_t box) {
  return pic_car(box);
}


#define ARG1() pic_cadr(cmd)
#define PUSH(value) stack = pic_cons(value, stack)
#define PEEK() pic_car(stack)
#define POP() stack = pic_cdr(stack)


pic_val_t pic_vm(pic_val_t code, pic_val_t stack) {

  while (! pic_nilp(code)) {
    pic_val_t cmd = pic_car(code);

    switch (pic_car(cmd)) {
      case OP_PUSH: {
        PUSH(ARG1());
        break;
      }
      case OP_CALL: {
        pic_val_t proc = PEEK(); POP();

        if (pic_closurep(proc)) {
          // TODO
          perror("FIXME: cannot call closures for now");
          abort();
        }
        else if (pic_nativep(proc)) {
          pic_val_t args = pic_nil;
          for (int i = 0, len = ARG1(); i < len; ++i) {
            args = pic_cons(PEEK(), args);
            POP();
          }
          args = pic_reverse(args);
          pic_val_t (*function)(pic_val_t args) = pic_native(proc)->function;
          PUSH(function(args));
          break;
        }
        else {
          perror("Invalid application");
          abort();
        }
      }
      case OP_GREF: {
        pic_val_t value = pic_unbox(ARG1());
        if (value == pic_undef) {
          fputs("unbound variable: ", stderr);
          pic_print(pic_box_name(ARG1()));
          abort();
        }
        stack = pic_cons(value, stack);
        break;
      }
    }

    code = pic_cdr(code);
  }

  return pic_car(stack);
}

pic_val_t pic_eval(pic_val_t form, pic_val_t env) {
  puts("compiling...");
  pic_val_t code = pic_compile(form, env);
  puts("compiled");
  return pic_vm(code, pic_nil);
}
