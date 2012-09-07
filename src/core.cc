#include "picrin.h"

#include <string.h>
#include <stdlib.h>


pic_val_t intern_table;
pic_val_t current_input_port;
pic_val_t current_output_port;
pic_val_t current_error_port;

pic_val_t pic_quote_sym;
pic_val_t pic_quasiquote_sym;
pic_val_t pic_unquote_sym;
pic_val_t pic_unquote_splicing_sym;


void pic_init()
{
  current_input_port = pic_make_port(stdin, true, true);
  current_output_port = pic_make_port(stdout, false, true);
  current_error_port = pic_make_port(stderr, false, true);

  intern_table = pic_nil;

  pic_quote_sym = pic_make_symbol("quote");
  pic_quasiquote_sym = pic_make_symbol("quasiquote");
  pic_unquote_sym = pic_make_symbol("unquote");
  pic_unquote_splicing_sym = pic_make_symbol("unquote-splicing");
}

pic_val_t pic_make_pair(pic_val_t car, pic_val_t cdr)
{
  pic_pair_t *obj = new pic_pair_t;
  obj->type = PIC_PAIR_T;
  obj->car = car;
  obj->cdr = cdr;
  return pic_val(obj);
}

pic_val_t pic_make_string(const std::string &str)
{
  pic_string_t *obj = new pic_string_t;
  obj->type = PIC_STRING_T;
  obj->str = str;
  return pic_val(obj);
}

pic_val_t pic_make_symbol(const std::string &name)
{
  pic_val_t str, dat;
  str = pic_make_string(name);
  dat = pic_assoc(str, intern_table);
  if (pic_falsep(dat)) {
    pic_symbol_t *obj = new pic_symbol_t;
    obj->type = PIC_SYMBOL_T;
    obj->str = str;

    /* Update intern table */
    intern_table = pic_acons(str, pic_val(obj), intern_table);
        
    return pic_val(obj);
  } else {
    return pic_cdr(dat);
  }
}

pic_val_t pic_make_port(FILE *file, bool dir, bool text)
{
  pic_port_t *obj = new pic_port_t;
  obj->file = file;
  obj->dir  = dir;
  obj->text = text;
  return pic_val(obj);
}
