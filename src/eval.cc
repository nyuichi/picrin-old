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


;; 1. ordinally
(begin
 (define f (lambda (x) (+ x 1)))
 (f 1))

  ; analyzed
  (begin
   (define f (lambda (x) () (+ x 1)))
   (f 1))

  ; generated
  ((JUMP MAIN)
   CLOSURE0
   (LREF 0)
   (LREF 0)
   (GREF box_+)
   (CALL 2)
   (RETURN)
   MAIN
   (CLOSURE 0 CLOSURE0)
   (GSET box_f)
   (POP)
   (PUSH 1)
   (GREF box_f)
   (CALL 1))
   

;; 2. free variable
(begin
 (define f (lambda (a) (lambda (b) (+ a b))))
 (define g (f 1))
 (g 2))

  ; analyzed
  (begin
   (define f (lambda (a) () (lambda (b) (a) (+ a b))))
   (define g (f 1))
   (g 2))
   
  ; generated
  ((JUMP MAIN)
   CLOSURE0
   (LREF 0)
   (CLOSURE 1 CLOSURE1)
   (RETURN)
   CLOSURE1
   (LREF 0)
   (CREF 0)
   (GREF box_+)
   (CALL 2)
   (RETURN)
   MAIN
   (CLOSURE 0 CLOSURE0)
   (GSET box_f)
   (POP)
   (PUSH 1)
   (GREF box_f)
   (CALL 1)
   (GSET box_g)
   (POP)
   (PUSH 1)
   (GREF box_g)
   (CALL 1))

;;

#endif


#define PUSH_FORM(x) form_stack = pic_cons(x, form_stack)
#define PEEK_FORM() pic_car(form_stack)
#define POP_FORM() form_stack = pic_cdr(form_stack)

#define PUSH_FREEVAR(x) free_vars = pic_cons(x, free_vars)
#define PUSH_FREEVARS(xs) free_vars = pic_append(xs, free_vars)

#define RESOLVE_SYNTAX_OPERATOR(x) pic_pair(form)->car = x

pic_val_t pic_analyze(pic_val_t form, pic_val_t env) {
  pic_val_t free_vars = pic_nil, form_stack = pic_nil;

  PUSH_FORM(form);
  while (! pic_nilp(form_stack)) {
    form = PEEK_FORM(); POP_FORM();
    if (pic_symbolp(form)) {
      if (pic_env_freevarp(form, env)) {
        PUSH_FREEVAR(form);
      }
      continue;
    } else if (! pic_pairp(form)) {
      continue;
    }
    else if (pic_listp(form)) {
      pic_val_t proc = pic_car(form);
    
      if (pic_symbolp(proc)) {
        pic_val_t res = pic_env_get(proc, env);

        if (pic_pairp(res) && pic_syntaxp(pic_cdr(res))) {
          switch (pic_cdr(res)) {
            case pic_define_syntax: {
              RESOLVE_SYNTAX_OPERATOR(pic_define_syntax);
              pic_env_add(pic_cadr(form), pic_undef, env);
              PUSH_FORM(pic_caddr(form));
              continue;
            }
            case pic_set_syntax: {
              RESOLVE_SYNTAX_OPERATOR(pic_set_syntax);
              PUSH_FORM(pic_caddr(form));
              continue;
            }
            case pic_begin_syntax: {
              RESOLVE_SYNTAX_OPERATOR(pic_begin_syntax);
              pic_val_t exprs = pic_cdr(form);
              while (! pic_nilp(exprs)) {
                PUSH_FORM(pic_car(exprs));
                exprs = pic_cdr(exprs);
              }
              continue;
            }
            case pic_quote_syntax: {
              RESOLVE_SYNTAX_OPERATOR(pic_quote_syntax);
              continue;
            }
            case pic_if_syntax: {
              RESOLVE_SYNTAX_OPERATOR(pic_if_syntax);
              PUSH_FORM(pic_cadr(form));
              PUSH_FORM(pic_caddr(form));
              PUSH_FORM(pic_cadddr(form));
              continue;
            }
            case pic_lambda_syntax: {
              RESOLVE_SYNTAX_OPERATOR(pic_lambda_syntax);
              pic_val_t formals = pic_cadr(form);

              pic_val_t local_env = pic_env_new(env);
              for (;;) {
                if (pic_nilp(formals)) {
                  break;
                }
                else if (pic_symbolp(formals)) {
                  pic_env_add(formals, pic_undef, local_env);
                  break;
                }
                else {
                  pic_env_add(pic_car(formals), pic_undef, local_env);
                  formals = pic_cdr(formals);
                }
              }

              pic_val_t fvs = pic_analyze(pic_caddr(form), local_env);

              formals = pic_cadr(form);
              for (;;) {
                if (pic_nilp(formals)) {
                  break;
                }
                else if (pic_symbolp(formals)) {
                  fvs = pic_delete_eq(formals, fvs);
                  break;
                }
                else {
                  fvs = pic_delete_eq(pic_car(formals), fvs);
                  formals = pic_cdr(formals);
                }
              }
              
              PUSH_FREEVARS(fvs);
              pic_pair(pic_cdr(form))->cdr = pic_list(fvs, pic_caddr(form));
              continue;
            }
            default: {
              perror("unsupported operation");
              abort();
            }
          }
        }
      }

      pic_val_t exprs = form;
      while (! pic_nilp(exprs)) {
        PUSH_FORM(pic_car(exprs));
        exprs = pic_cdr(exprs);
      }
      continue;
    }
    else {
      perror("dotted list in source");
      abort();
    }
  }
  return free_vars;
}

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
            // FIXME: pop return values
            return code;
          }
          case pic_quote_syntax: {
            pic_val_t cmd = pic_list(OP_PUSH, pic_cadr(form));
            return pic_cons(cmd, code);
          }
          case pic_if_syntax: {
            pic_val_t label1 = pic_gensym("fail");
            pic_val_t label2 = pic_gensym("succ");
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


// The VM
pic_val_t pic_execute(pic_val_t code, pic_val_t stack) {

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
  //  pic_val_t code = pic_compile(form, env);
  puts("compiled");
  //  return pic_execute(code, pic_nil);
  pic_analyze(form, pic_minimal_environment());
  return form;
}
