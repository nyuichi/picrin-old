#include "picrin.h"


enum pic_opcode {
  OP_PUSH = 0x03,
  OP_CALL = 0x13,
  OP_GREF = 0x23,
  OP_GSET = 0x33,
  OP_JMP = 0x43,
  OP_JMZ = 0x53,
};

#if 0

(+ 1
  (if (null? ())
      1
      2))
  

((OP_PUSH ())
 (OP_GREF box_null?) ;; (null? ())
 (OP_JMZ LABEL1)     ;; (if ...)
 (OP_PUSH 1)         ;; 1
 (OP_JMP LABEL2)
 LABEL1
 (OP_PUSH 2)         ;; 2
 LABEL2
 (OP_PUSH 1)
 (OP_GREF +)
 (CALL 2)            ;; (+ 1 ...)
 )

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
    pic_val_t proc = pic_car(form);
    
    if (pic_symbolp(proc)) {
      pic_val_t res = pic_env_get(proc, env);

      if (pic_pairp(res) && pic_syntaxp(pic_cdr(res))) {
        switch (pic_cdr(res)) {
          case pic_define_syntax: {
            pic_env_add(pic_cadr(form), pic_undef, env);
            // *fallthrough*
          }
          case pic_set_syntax: {
            code = pic_generate(pic_caddr(form), env, code);
            pic_val_t var = pic_env_get(pic_cadr(form), env);
            pic_val_t cmd = pic_list(OP_GSET, var);
            return pic_cons(cmd, code);
          }
          case pic_begin_syntax: {
            pic_val_t exprs = pic_cdr(form);
            while (! pic_nilp(exprs)) {
              code = pic_generate(pic_car(exprs), env, code);
              exprs = pic_cdr(exprs);
            }
            return code;
          }
          case pic_quote_syntax: {
            pic_val_t cmd = pic_list(OP_PUSH, pic_cadr(form));
            return pic_cons(cmd, code);
          }
          case pic_if_syntax: {
            pic_val_t label1 = pic_gensym("label1");
            pic_val_t label2 = pic_gensym("label2");
            code = pic_generate(pic_cadr(form), env, code);
            code = pic_cons(pic_list(OP_JMZ, label1), code);
            code = pic_generate(pic_caddr(form), env, code);
            code = pic_cons(pic_list(OP_JMP, label2), code);
            code = pic_cons(label1, code);
            code = pic_generate(pic_cadddr(form), env, code);
            code = pic_cons(label2, code);
            return code;
          }
          default: {
            perror("unsupported operation");
            abort();
          }
        }
      }
    }

    int n = -1;
    form = pic_reverse(form);

    while (! pic_nilp(form)) {
      code = pic_generate(pic_car(form), env, code);
      form = pic_cdr(form);
      ++n;
    }
    
    pic_val_t cmd = pic_list(OP_CALL, pic_fixnum(n));
    return pic_cons(cmd, code);
  }
  else {
    perror("dotted list in source");
    abort();
  }
}

pic_val_t pic_assemble(pic_val_t code) {
  pic_val_t labels = pic_nil, cmds = pic_nil;

  // mark phase
  while (! pic_nilp(code)) {
    if (pic_symbolp(pic_car(code))) {
      labels = pic_acons(pic_car(code), cmds, labels);
      code = pic_cdr(code);
      continue;
    }
    cmds = pic_cons(pic_car(code), cmds);
    code = pic_cdr(code);
  }

  code = cmds;

  // replace phase
  while (! pic_nilp(cmds)) {
    pic_val_t cmd = pic_car(cmds);
    if (pic_car(cmd) == OP_JMZ) {
      pic_pair(pic_cdr(cmd))->car = pic_cdr(pic_assq(pic_cadr(cmd), labels));
    }
    else if (pic_car(cmd) == OP_JMP) {
      pic_pair(pic_cdr(cmd))->car = pic_cdr(pic_assq(pic_cadr(cmd), labels));
    }
    cmds = pic_cdr(cmds);
  }

  return code;
}

pic_val_t pic_compile(pic_val_t form, pic_val_t env) {
  return pic_assemble(pic_generate(form, env, pic_nil));
}


pic_val_t pic_unbox(pic_val_t box) {
  return pic_cdr(box);
}

void pic_box_set(pic_val_t box, pic_val_t val) {
  pic_pair(box)->cdr = val;
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
          for (int i = 0, len = pic_int(ARG1()); i < len; ++i) {
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
        PUSH(value);
        break;
      }
      case OP_GSET: {
        pic_box_set(ARG1(), PEEK());
        POP();
        break;
      }
      case OP_JMP: {
        code = ARG1();
        continue;
      }
      case OP_JMZ: {
        if (pic_falsep(PEEK())) {
          code = ARG1();
          continue;
        }
        break;
      }
    }

    code = pic_cdr(code);
  }

  if (pic_nilp(stack)) {
    return pic_void;
  }
  else {
    return pic_car(stack);
  }
}

pic_val_t pic_eval(pic_val_t form, pic_val_t env) {
  puts("compiling...");
  pic_val_t code = pic_compile(form, env);
  puts("compiled");
  return pic_vm(code, pic_nil);
}
