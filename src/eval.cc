#include "picrin.h"


enum pic_opcode {
  OP_PUSH = 0x03,
  OP_CALL = 0x13,
  OP_GREF = 0x23,
  OP_GSET = 0x33,
  OP_JMP = 0x43,
  OP_JMZ = 0x53,
  OP_LREF = 0x63,
  OP_CREF = 0x73,
  OP_CLOSURE = 0x83,
  OP_RETURN = 0x93,
  OP_EXIT = 0xa3,
  OP_POP = 0xb3,
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
  (MAIN
   (CLOSURE 0 CLOSURE0)
   (GSET box_f)
   (POP)
   (PUSH 1)
   (GREF box_f)
   (CALL 1)
   (EXIT)
   CLOSURE0
   (LREF 0)
   (LREF 0)
   (GREF box_+)
   (CALL 2)
   (RETURN 1))
   

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
  (MAIN
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
   (CALL 1)
   (EXIT)
   CLOSURE0
   (LREF 0)
   (CLOSURE 1 CLOSURE1)
   (RETURN)
   CLOSURE1
   (LREF 0)
   (CREF 0)
   (GREF box_+)
   (CALL 2)
   (RETURN))


  // TODO
  // internal definition
  // assignment to local/free variable
  // variable arguments
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

static pic_val_t closures;
static pic_val_t genv;
static pic_val_t lvars;
static pic_val_t fvars;

pic_val_t pic_generate_expr(pic_val_t form, pic_val_t code) {
  if (pic_symbolp(form)) {
    pic_val_t cmd;
    int index;
    if ((index = pic_locate_eq(form, lvars)) != -1) {
      cmd = pic_list(OP_LREF, pic_fixnum(index));
    }
    else if ((index = pic_locate_eq(form, fvars)) != -1) {
      cmd = pic_list(OP_CREF, pic_fixnum(index));
    }
    else {
      cmd = pic_list(OP_GREF, pic_env_get(form, genv));
    }
    code = pic_cons(cmd, code);
    return code;
  }
  else if (! pic_pairp(form)) {
    pic_val_t cmd = pic_list(OP_PUSH, form);
    code = pic_cons(cmd, code);
    return code;
  }
  else { // if (pic_listp(form))
    switch (pic_car(form)) {
      case pic_define_syntax: {       // TODO: internal def
        pic_env_add(pic_cadr(form), pic_undef, genv);
        // *fallthrough*
      }
      case pic_set_syntax: {          // TODO: set! to local/free var
        code = pic_generate_expr(pic_caddr(form), code);
        pic_val_t var = pic_env_get(pic_cadr(form), genv);
        pic_val_t cmd = pic_list(OP_GSET, var);
        code = pic_cons(cmd, code);
        return code;
      }
      case pic_begin_syntax: {
        pic_val_t exprs = pic_cdr(form);
        while (! pic_nilp(exprs)) {
          code = pic_generate_expr(pic_car(exprs), code);
          code = pic_cons(pic_list(OP_POP), code);
          exprs = pic_cdr(exprs);
        }
        code = pic_cdr(code); // remve the last pop
        return code;
      }
      case pic_quote_syntax: {
        pic_val_t cmd = pic_list(OP_PUSH, pic_cadr(form));
        code = pic_cons(cmd, code);
      }
      case pic_if_syntax: {
        pic_val_t label1 = pic_gensym("fail");
        pic_val_t label2 = pic_gensym("succ");
        code = pic_generate_expr(pic_cadr(form), code);
        code = pic_cons(pic_list(OP_JMZ, label1), code);
        code = pic_generate_expr(pic_caddr(form), code);
        code = pic_cons(pic_list(OP_JMP, label2), code);
        code = pic_cons(label1, code);
        code = pic_generate_expr(pic_cadddr(form), code);
        code = pic_cons(label2, code);
        return code;
      }
      case pic_lambda_syntax: {
        // create new label
        pic_val_t label = pic_gensym("closure");

        // delays code generation of the closure
        closures = pic_acons(label, form, closures);

        // push variables to be closed
        pic_val_t freevars = pic_reverse(pic_caddr(form));
        int num_fvs = 0;
        while (! pic_nilp(freevars)) {
          code = pic_generate_expr(pic_car(freevars), code);
          freevars = pic_cdr(freevars);
          ++num_fvs;
        }

        // make the closure
        pic_val_t cmd = pic_list(OP_CLOSURE, pic_fixnum(num_fvs), label);
        code = pic_cons(cmd, code);
        return code;
      }
      default: {
        int n = -1;
        form = pic_reverse(form);

        while (! pic_nilp(form)) {
          code = pic_generate_expr(pic_car(form), code);
          form = pic_cdr(form);
          ++n;
        }
    
        pic_val_t cmd = pic_list(OP_CALL, pic_fixnum(n));
        return pic_cons(cmd, code);
      }
    } // switch
  } // else
}

pic_val_t pic_generate(pic_val_t form, pic_val_t env) {
  genv = env;
  closures = pic_nil;
  
  pic_val_t code = pic_nil;
  
  // generate main
  pic_val_t main = pic_gensym("main");
  lvars = fvars = pic_nil;
  code = pic_cons(main, code);
  code = pic_generate_expr(form, code);
  code = pic_cons(pic_list(OP_EXIT), code); // TODO Can't we replace EXIT with RETURN?

  // generate closures
  while (! pic_nilp(closures)) {
    pic_val_t x = pic_car(closures); closures = pic_cdr(closures);

    pic_val_t label = pic_car(x), closure = pic_cdr(x);
    lvars = pic_cadr(closure);
    fvars = pic_caddr(closure);

    code = pic_cons(label, code);
    code = pic_generate_expr(pic_cadddr(closure), code);
    code = pic_cons(pic_list(OP_RETURN), code);
  }

  return code;
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
      pic_pair(pic_cdr(cmd))->car
          = pic_cdr(pic_assq(pic_cadr(cmd), labels));
    }
    else if (pic_car(cmd) == OP_JMP) {
      pic_pair(pic_cdr(cmd))->car
          = pic_cdr(pic_assq(pic_cadr(cmd), labels));
    }
    else if (pic_car(cmd) == OP_CLOSURE) {
      pic_pair(pic_cddr(cmd))->car
          = pic_cdr(pic_assq(pic_caddr(cmd), labels));
    }
    cmds = pic_cdr(cmds);
  }

  return code;
}

pic_val_t pic_print_assemply(pic_val_t code) {
  code = pic_reverse(code);
  while (! pic_nilp(code)) {
    pic_print(pic_car(code));
    code = pic_cdr(code);
  }
}

pic_val_t pic_compile(pic_val_t form, pic_val_t env) {
  pic_val_t code;
  pic_analyze(form, pic_minimal_environment());
  puts("**analyzed**");
  pic_print(form);
  puts("**end**");
  code = pic_generate(form, env);
  puts("**geenrated**");
  pic_print_assemply(code);
  puts("**end**");
  code = pic_assemble(code);
  puts("**assembled**");
  pic_print_assemply(code);
  puts("**end**");
  return code;
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
#define ARG2() pic_caddr(cmd)
#define PUSH(value) stack = pic_cons(value, stack)
#define PEEK() pic_car(stack)
#define POP() stack = pic_cdr(stack)
#define PARAMETERS() pic_caddr(pic_car(frames))


// The VM
pic_val_t pic_execute(pic_val_t code) {

  pic_val_t stack = pic_nil;            // stack
  pic_val_t frames = pic_nil;            // frame is list of pc and numarg

  while (! pic_nilp(code)) {
    pic_val_t cmd = pic_car(code);

    perror("executing:");
    pic_print(cmd);

    switch (pic_car(cmd)) {
      case OP_PUSH: {
        PUSH(ARG1());
        break;
      }
      case OP_CALL: {
        pic_val_t proc = PEEK();

        if (pic_closurep(proc)) {
          // What is necessary
          // current_pc
          // num_args
          // local_vars (with proc)
          frames = pic_cons(pic_list(code, ARG1(), stack), frames);
          code = pic_closure(proc)->code;
          continue;
        }
        else if (pic_nativep(proc)) {
          POP();
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
      case OP_LREF: {
        int n = pic_int(ARG1());
        pic_val_t lvars = pic_cdr(PARAMETERS());
        for (int i = 0; i != n; ++i) {
          lvars = pic_cdr(lvars);
        }
        PUSH(pic_car(lvars));
        break;
      }
      case OP_CREF: {
        int n = pic_int(ARG1());
        pic_val_t closed = pic_closure(pic_car(PARAMETERS()))->closed;
        for (int i = 0; i != n; ++i) {
          closed = pic_cdr(closed);
        }
        PUSH(pic_car(closed));
        break;
      }
      case OP_CLOSURE: {
        pic_val_t closed = pic_nil;
        for (int i = 0, len = pic_int(ARG1()); i < len; ++i) {
          closed = pic_cons(PEEK(), closed);
          POP();
        }
        perror("closed");
        pic_print(closed);
        pic_val_t closure = pic_make_closure(ARG2(), closed);
        PUSH(closure);
        break;
      }
      case OP_RETURN: {
        pic_val_t return_value = PEEK(); POP();
        pic_val_t frame = pic_car(frames); frames = pic_cdr(frames);
        code = pic_car(frame);
        for (int i = -1, len = pic_int(pic_cadr(frame)); i < len; ++i) {
          POP();
        }
        PUSH(return_value);
        break;
      }
      case OP_EXIT: {
        if (pic_nilp(stack)) {
          return pic_void;
        }
        else { 
          return pic_car(stack);
        }
      }
      case OP_POP: {
        POP();
        break;
      }
    }

    code = pic_cdr(code);
  }
}

pic_val_t pic_eval(pic_val_t form, pic_val_t env) {
  puts("compiling...");
  pic_val_t code = pic_compile(form, env);
  puts("compiled");
  return pic_execute(code);
}
