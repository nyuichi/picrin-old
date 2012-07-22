#include "picrin.h"

#include <string.h>
#include <stdlib.h>


pic_obj_t standard_input_port;
pic_obj_t standard_output_port;
pic_obj_t standard_error_port;

void pic_init()
{
  /* There are implicit increfs to give ownerships to the global variables
     and decrefs to dispose the ownerships by this function.  */
  standard_input_port  = pic_make_port(stdin, true, true);
  standard_output_port = pic_make_port(stdout, false, true);
  standard_error_port  = pic_make_port(stderr, false, true);
}

static void destroy_pair(pic_obj_t obj);
static void destroy_symbol(pic_obj_t obj);
static void destroy_string(pic_obj_t obj);
static void destroy_port(pic_obj_t obj);
static void dealloc_env(pic_obj_t obj);

pic_obj_t pic_make_pair(pic_obj_t car, pic_obj_t cdr)
{
  pic_pair_t * obj
    = pic_malloc(sizeof(pic_pair_t), PIC_TYPE_PAIR, destroy_pair);
  PIC_XINCREF(car);
  PIC_XINCREF(cdr);
  PIC_CAR(obj) = car;
  PIC_CDR(obj) = cdr;
  return (pic_obj_t)obj;
}

pic_obj_t pic_make_string(char * str)
{
  pic_string_t * obj
    = pic_malloc(sizeof(pic_string_t) + strlen(str) + 1,
		 PIC_TYPE_STRING,
		 destroy_string);
  strcpy(obj->data, str);
  return (pic_obj_t)obj;
}

pic_obj_t pic_make_symbol(char * rep)
{
  pic_obj_t str = pic_make_string(rep);
  pic_symbol_t * obj
    = pic_malloc(sizeof(pic_symbol_t), PIC_TYPE_SYMBOL, destroy_symbol);
  PIC_SYMBOL_REP(obj) = str;
  return (pic_obj_t)obj;
}

pic_obj_t pic_intern(pic_obj_t symbol)
{
  static pic_obj_t intern_table = PIC_NIL;

  pic_obj_t res = pic_assoc(PIC_SYMBOL_REP(symbol), intern_table);
  if (PIC_FALSEP(res)) {
    pic_obj_t tmp = pic_acons(PIC_SYMBOL_REP(symbol), symbol, intern_table);
    PIC_XDECREF(intern_table);
    intern_table = tmp;
    PIC_INCREF(symbol);
    return symbol;
  } else {
    pic_obj_t sym = PIC_CDR(res);
    PIC_INCREF(sym);
    PIC_DECREF(res);
    return sym;
  }
}

pic_obj_t pic_get_interned_symbol(char * rep)
{
  pic_obj_t symbol = pic_make_symbol(rep);
  pic_obj_t result = pic_intern(symbol);
  PIC_DECREF(symbol);
  return result;
}

pic_obj_t pic_make_port(FILE * file, bool dir, bool text)
{
  pic_port_t * obj
    = pic_malloc(sizeof(pic_port_t), PIC_TYPE_PORT, destroy_port);
  obj->file = file;
  obj->dir  = dir;
  obj->text = text;
  return (pic_obj_t)obj;
}

pic_obj_t pic_make_env(pic_obj_t base_env, pic_obj_t function)
{
  pic_env_t * obj = pic_malloc(sizeof(pic_env_t), PIC_TYPE_ENV, dealloc_env);
  obj->parent = base_env;
  obj->function = function;
  obj->table = PIC_NIL;
  PIC_XINCREF(base_env);
  PIC_XINCREF(function);
  return (pic_obj_t)obj;
}

static void destroy_pair(pic_obj_t obj)
{
  PIC_XDECREF(PIC_CAR(obj));
  PIC_XDECREF(PIC_CDR(obj));
  pic_free(obj);
}

static void destroy_symbol(pic_obj_t obj)
{
  PIC_XDECREF(PIC_SYMBOL_REP(obj));
  pic_free(obj);
}

static void destroy_string(pic_obj_t obj)
{
  pic_free(obj);
}

static void destroy_port(pic_obj_t obj)
{
  pic_free(obj);
}

static void dealloc_env(pic_obj_t obj)
{
  PIC_XDECREF(PIC_ENV_PARENT(obj));
  PIC_XDECREF(PIC_ENV_FUNCTION(obj));
  PIC_XDECREF(PIC_ENV_TABLE(obj));
  pic_free(obj);
}
