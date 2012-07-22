#include "picrin.h"

/* lambda
 * (pic-ast-lambda formals local-vars closure-set free-vars body)
 */

/* FIXME */
enum pic_asts {
  PIC_AST_VAR = 0x0f,
  PIC_AST_LITERAL = 0x1f,
  PIC_AST_IF = 0x2f,
  PIC_AST_SET = 0x3f,
  PIC_AST_FUNCTION = 0x4f,
  PIC_AST_BEGIN = 0x5f,
  PIC_AST_CALL = 0x6f,
};


/* 
 * ast ::= begin
 *       | function
 *       | var
 *       | if
 *       | set
 *       | call
 *       | literal
 *
 * var ::= (PIC_AST_VAR index function)
 * 
 * function ::= (
 *               PIC_AST_FUNCTION
 *               num_parameters
 *               num_local_vars
 *               body
 *               list of freevar
 *               list of boxedvar
 *               )
 *
 */

pic_obj_t pic_make_global_var()
{
  return pic_list3(PIC_AST_VAR, PIC_TO_FIXNUM(0), PIC_NIL);
}

pic_obj_t pic_make_local_var(pic_obj_t env)
{
  pic_obj_t function = PIC_ENV_FUNCTION(env);
  pic_obj_t index = PIC_CADDR(function);
  PIC_CADDR(function) = PIC_TO_FIXNUM(PIC_FROM_FIXNUM(PIC_CADDR(function))+1);
  return pic_list3(PIC_AST_VAR, index, function);
}

pic_obj_t pic_make_parameter(pic_obj_t env)
{
  pic_obj_t function = PIC_ENV_FUNCTION(env);
  pic_obj_t index = PIC_TO_FIXNUM(~PIC_FROM_FIXNUM(PIC_CADR(function)));
  PIC_CADR(function) = PIC_TO_FIXNUM(PIC_FROM_FIXNUM(PIC_CADR(function))+1);
  return pic_list3(PIC_AST_VAR, index, function);
}

pic_obj_t pic_get_or_make_var(pic_obj_t symbol, pic_obj_t env)
{
  pic_obj_t res = pic_env_get(symbol, env);
  if (PIC_FALSEP(res)) {
    /* FIXME */
    return 0;
  } else {
    pic_obj_t ret = PIC_CDR(res);
    PIC_INCREF(ret);
    PIC_DECREF(res);
    return ret;
  }
}

pic_obj_t pic_list6(pic_obj_t a, pic_obj_t b, pic_obj_t c, pic_obj_t d, pic_obj_t e, pic_obj_t f)
{
  pic_obj_t tail1 = pic_list4(c, d, e, f);
  pic_obj_t tail2 = pic_cons(b, tail1);
  pic_obj_t list = pic_cons(a, tail2);
  PIC_DECREF(tail1);
  PIC_DECREF(tail2);
  return list;
}
 
pic_obj_t pic_make_function(pic_obj_t body)
{
  PIC_XINCREF(body);
  return pic_list6(PIC_AST_FUNCTION, PIC_TO_FIXNUM(0), PIC_TO_FIXNUM(0),
		   body, PIC_NIL, PIC_NIL);
}

pic_obj_t pic_function_set_body(pic_obj_t body, pic_obj_t function)
{
  PIC_XDECREF(PIC_CADDDR(function));
  PIC_XINCREF(body);
  PIC_CADDDR(function) = body;
}


/*******************************************************************************
 * Variable resolution
 *******************************************************************************/


/* FIXME : when pic_env_get returned syntax? */

pic_obj_t resolve(pic_obj_t expr, pic_obj_t env);
pic_obj_t resolve_all(pic_obj_t list, pic_obj_t env);

pic_obj_t resolve_begin(pic_obj_t list, pic_obj_t env)
{
  /* TODO: warn if list is NIL */
  /* TODO: when list is single, return the expression directly */
  pic_obj_t body = resolve_all(list, env);
  pic_obj_t res = pic_cons(PIC_AST_BEGIN, body);
  PIC_XDECREF(body);
  return res;
}

pic_obj_t resolve_set(pic_obj_t symbol, pic_obj_t val, pic_obj_t env)
{
  /* TODO: var is a symbol? */
  pic_obj_t res = pic_env_get(symbol, env);
  if (PIC_FALSEP(res)) {
    /* TODO : throw error */
    return 0;
  } else {
    pic_obj_t value = resolve(val, env);
    pic_obj_t result = pic_list3(PIC_AST_SET, PIC_CDR(res), value);
    PIC_DECREF(res);
    PIC_DECREF(value);
    return result;
  }
}

pic_obj_t resolve_define(pic_obj_t symbol, pic_obj_t val, pic_obj_t env)
{
  /* TOOD: var must be a symbol */
  /* TODO: emit warning when sym is already added to env */
  pic_obj_t var, res;
  if (pic_env_globalp(env)) {
    var = pic_make_global_var();
    pic_env_def(symbol, var, env);
  } else {
    var = pic_make_local_var(env);
    pic_env_def(symbol, var, env);
  }
  res = resolve_set(symbol, val, env);
  PIC_DECREF(var);
  return res;
}

pic_obj_t resolve_lambda(pic_obj_t formals, pic_obj_t body, pic_obj_t env)
{
  /* TODO: varg */
  /* TODO: assure formals consist only of symbols */
  perror("lambda");
  pic_obj_t function = pic_make_function(body);
  pic_obj_t new_env = pic_make_env(env, function);
  pic_obj_t params = formals, tmp;
  perror("generating parameters");
  while (!PIC_NILP(params)) {
    pic_obj_t var = pic_make_parameter(new_env);
    pic_env_def(PIC_CAR(params), var, new_env);
    PIC_DECREF(var);
    tmp = PIC_CDR(params);
    PIC_XINCREF(tmp);
    PIC_DECREF(params);
    params = tmp;
  }
  perror("generated parameters");
  body = resolve_begin(body, new_env);
  pic_function_set_body(body, function);
  PIC_XDECREF(body);
  PIC_DECREF(new_env);
  return function;
}

pic_obj_t resolve_if(pic_obj_t test, pic_obj_t succ, pic_obj_t fail, pic_obj_t env)
{
  pic_obj_t result;
  test = resolve(test, env);
  succ = resolve(succ, env);
  fail = resolve(fail, env);
  result = pic_list4(PIC_AST_IF, test, succ, fail);
  PIC_DECREF(test);
  PIC_DECREF(succ);
  PIC_DECREF(fail);
  return result;
}

pic_obj_t resolve_literal(pic_obj_t obj)
{
  return pic_list2(PIC_AST_LITERAL, obj);
}

pic_obj_t resolve_list(pic_obj_t list, pic_obj_t env)
{
  pic_print(list, standard_output_port);

  pic_obj_t res = PIC_NIL, ast_tag;

  if (!PIC_SYMBOLP(PIC_CAR(list)))
    goto CALL;

  res = pic_env_get(PIC_CAR(list), env);

  if (PIC_FALSEP(res))
    goto CALL;

  ast_tag = PIC_CDR(res);
  PIC_XINCREF(ast_tag);
  PIC_DECREF(res);

  /* TODO assert the num of args is correct */
  switch (ast_tag) {
  default:
    goto CALL;
  case PIC_SYNTAX_DEFINE:
    return resolve_define(PIC_CADR(list), PIC_CADDR(list), env);
  case PIC_SYNTAX_SET:
    return resolve_set(PIC_CADR(list), PIC_CADDR(list), env);
  case PIC_SYNTAX_LAMBDA:
    return resolve_lambda(PIC_CADR(list), PIC_CDDR(list), env);
  case PIC_SYNTAX_IF:
    return resolve_if(PIC_CADR(list), PIC_CADDR(list), PIC_CADDDR(list), env);
  case PIC_SYNTAX_QUOTE:
    return resolve_literal(PIC_CADR(list));
  case PIC_SYNTAX_BEGIN:
    return resolve_begin(PIC_CDR(list), env);
  }
  
 CALL:
  perror("call");
  PIC_XDECREF(ast_tag);
  list = resolve_all(list, env);
  res = pic_cons(PIC_AST_CALL, list);
  PIC_DECREF(list);
  return res;
}

pic_obj_t resolve_var(pic_obj_t symbol, pic_obj_t env)
{
  return pic_get_or_make_var(symbol, env);
}

pic_obj_t resolve_all(pic_obj_t list, pic_obj_t env)
{
  if (PIC_NILP(list)) {
    return PIC_NIL;
  } else {
    pic_obj_t car = resolve(PIC_CAR(list), env);
    pic_obj_t cdr = resolve_all(PIC_CDR(list), env);
    pic_obj_t res = pic_cons(car, cdr);
    PIC_XDECREF(car);
    PIC_XDECREF(cdr);
    return res;
  }
}

pic_obj_t resolve(pic_obj_t expr, pic_obj_t env)
{
  if (PIC_POINTERP(expr)) {
    if (PIC_SYMBOLP(expr)) {
      return resolve_var(expr, env);
    }
    if (pic_listp(expr)) {
      return resolve_list(expr, env);
    }
  }
  /* emit warning if expr is a syntax of macro */
  return resolve_literal(expr);
}

void print_ast(pic_obj_t);

void print_ast_list(pic_obj_t list)
{
  while (!PIC_NILP(list)) {
    print_ast(PIC_CAR(list));
    printf(" ");
    list = PIC_CDR(list);
  }
  printf("END");
}

void print_ast(pic_obj_t expr)
{
  switch (PIC_CAR(expr)) {
  case PIC_AST_VAR:
    printf("(VAR-%x %d %x)", (int)expr,
	   PIC_FROM_FIXNUM(PIC_CADR(expr)), (int)PIC_CADDR(expr));
    break;
  case PIC_AST_LITERAL:
    printf("(LITERAL-%x ", (int)expr);
    pic_write(PIC_CADR(expr), standard_output_port);
    printf(")");
    break;
  case PIC_AST_SET:
    printf("(SET-%x ", (int)expr);
    print_ast(PIC_CADR(expr));
    printf(" ");
    print_ast(PIC_CADDR(expr));
    printf(")");
    break;
  case PIC_AST_IF:
    printf("(IF-%x ", (int)expr);
    print_ast(PIC_CADR(expr));
    print_ast(PIC_CADDR(expr));
    print_ast(PIC_CADDDR(expr));
    printf(")");
    break;
  case PIC_AST_FUNCTION:
    printf("(FUNCTION-%x %d %d ", (int)expr, (int)PIC_CADR(expr), (int)PIC_CADDR(expr));
    print_ast(PIC_CADDDR(expr));
    print_ast_list(PIC_CADDDDR(expr));
    print_ast_list(PIC_CADDDDDR(expr));
    printf(")");
    break;
  case PIC_AST_BEGIN:
    printf("(BEGIN-%x ", (int)expr);
    print_ast_list(PIC_CDR(expr));
    printf(")");
    break;
  case PIC_AST_CALL:
    printf("(CALL-%x ", (int)expr);
    print_ast_list(PIC_CDR(expr));
    printf(")");
    break;
  }
}





/*******************************************************************************
 * Closure Conversion
 *******************************************************************************/


/* void mark_closure_set_var(pic_obj_t var) */
/* { */
/*   pic_lambda_push_closure_set_var(var, PIC_VAR_LAMBDA(var)); */
/* } */

/* void mark_closure_set(pic_obj_t expr, pic_obj_t lambda) */
/* { */
/*   switch (PIC_CAR(expr)) { */
/*   case PIC_AST_LAMBDA: */
/*     mark_closure_set(pic_lambda_body(expr), expr); */
/*     break; */
/*   case PIC_AST_SET: */
/*     { */
/*       pic_obj_t var = PIC_CADR(expr); */
/*       if (!pic_var_globalp(var) && lambda != PIC_VAR_LAMBDA(var)) { */
/* 	mark_closure_set_var(var); */
/*       } */
/*       break; */
/*     } */
/*   case PIC_AST_VAR: */
/*   case PIC_AST_LITERAL: */
/*     break; */
/*   case PIC_AST_IF: */
/*   case PIC_AST_CALL: */
/*   case PIC_AST_BEGIN: */
/*     { */
/*       pic_obj_t body = PIC_CDR(expr); */
/*       while (!PIC_NILP(body)) { */
/* 	mark_closure_set(PIC_CAR(body), lambda); */
/* 	body = PIC_CDR(body); */
/*       } */
/*       break; */
/*     } */
/*   } */
/* } */

/* void box_closed_variables(pic_obj_t expr) */
/* { */
/*   switch (PIC_CAR(expr)) { */
/*   case PIC_AST_LAMBDA: */
    
/*   } */
/* } */

/* void eliminate_closure_set(pic_obj_t expr) */
/* { */
/*   mark_closure_set(expr, PIC_NIL); */
/*   box_closed_variables(expr); */
/* } */

/* void eliminate_free_variables(pic_obj_t expr) */
/* { */
/*   lambda_lifting(expr); */
/*   convert_closure_refs(expr); */
/* } */

/* void closure_conversion(pic_obj_t expr) */
/* { */
/*   eliminate_closure_set(expr); */
/*   eliminate_free_variables(expr); */
/* } */
