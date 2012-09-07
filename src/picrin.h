// -*- C++ -*-

#ifndef PICRIN_H
#define PICRIN_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>


#define PICRIN_VERSION "0.0.2"


/*******************************************************************************
 * Low Level Object
 ******************************************************************************/


typedef uintptr_t pic_val_t;


// 1. Immediate Constant Values

const pic_val_t pic_true  = 0x0f;
const pic_val_t pic_false = 0x1f;
const pic_val_t pic_nil   = 0x2f;
const pic_val_t pic_void  = 0x3f;


inline bool pic_nilp(pic_val_t val) {
  return val == pic_nil;
}

inline bool pic_truep(pic_val_t val) {
  return val == pic_true;
}

inline bool pic_falsep(pic_val_t val) {
  return val == pic_false;
}

inline bool pic_voidp(pic_val_t val) {
  return val == pic_void;
}


// 2. Fixnum

inline bool pic_fixnump(pic_val_t val) {
  return (val & 0x03) == 0;
}

inline pic_val_t pic_fixnum(int val) {
  return val << 2;
}

inline int pic_int(pic_val_t val) {
  return val >> 2;
}

// 3. Character

inline bool pic_characterp(pic_val_t val) {
  return (val & 0x0f) == 0x0b;
}

inline bool pic_character(char c) {
  return (c << 4) | 0x0b;
}

inline char pic_char(char c) {
  return c >> 4;
}

// 4. Heap Object

inline bool pic_objectp(pic_val_t val) {
  return (val & 0x03) == 1;
}


struct pic_obj_t {
  int type;
};

inline pic_obj_t *pic_object(pic_val_t val) {
  return reinterpret_cast<pic_obj_t *>(val - 1);
}

inline pic_val_t pic_val(pic_obj_t *obj) {
  return reinterpret_cast<pic_val_t>(obj) + 1;
}

inline int pic_typeof(pic_val_t val) {
  return pic_object(val)->type;
}



/*******************************************************************************
 * High Level Object
 *******************************************************************************/


enum pic_object_type {
  PIC_PAIR_T,
  PIC_SYMBOL_T,
  PIC_STRING_T,
  PIC_PORT_T,
  PIC_CLOSURE_T,
  PIC_NATIVE_T,
};


struct pic_pair_t : pic_obj_t {
  pic_val_t car;
  pic_val_t cdr;
};


struct pic_string_t : pic_obj_t {
  std::string str;
};


struct pic_symbol_t : pic_obj_t {
  pic_val_t str;
};
    
    
struct pic_port_t : pic_obj_t {
  FILE *file;
  bool dir;                 /* true for input, false for output */
  bool text;                /* true for textual, false for binary */
  bool stat;                /* true for open, false for close */
};


struct pic_closure_t : pic_obj_t {
  pic_val_t env;
  pic_val_t args;
  pic_val_t body;
};


struct pic_native_t : pic_obj_t {
  pic_val_t (*function)(pic_val_t args);
};


inline bool pic_objectof(pic_val_t val, pic_object_type type) {
  return pic_objectp(val) && pic_typeof(val) == type;
}

inline bool pic_pairp(pic_val_t val) {
  return pic_objectof(val, PIC_PAIR_T);
}

inline bool pic_stringp(pic_val_t val) {
  return pic_objectof(val, PIC_STRING_T);
}

inline bool pic_symbolp(pic_val_t val) {
  return pic_objectof(val, PIC_SYMBOL_T);
}

inline bool pic_portp(pic_val_t val) {
  return pic_objectof(val, PIC_PORT_T);
}

inline bool pic_closurep(pic_val_t val) {
  return pic_objectof(val, PIC_CLOSURE_T);
}

inline bool pic_nativep(pic_val_t val) {
  return pic_objectof(val, PIC_NATIVE_T);
}



inline pic_pair_t *pic_pair(pic_val_t val) {
  return static_cast<pic_pair_t *>(pic_object(val));
}

inline pic_symbol_t *pic_symbol(pic_val_t val) {
  return static_cast<pic_symbol_t *>(pic_object(val));
}

inline pic_string_t *pic_string(pic_val_t val) {
  return static_cast<pic_string_t *>(pic_object(val));
}

inline pic_port_t *pic_port(pic_val_t val) {
  return static_cast<pic_port_t *>(pic_object(val));
}

inline pic_closure_t *pic_closure(pic_val_t val) {
  return static_cast<pic_closure_t *>(pic_object(val));
}

inline pic_native_t *pic_native(pic_val_t val) {
  return static_cast<pic_native_t *>(pic_object(val));
}

pic_val_t pic_make_pair(pic_val_t car, pic_val_t cdr);
pic_val_t pic_make_string(const std::string &str);
pic_val_t pic_make_symbol(const std::string &name);
pic_val_t pic_make_port(FILE * file, bool dir, bool text);
pic_val_t pic_make_closure(pic_val_t args, pic_val_t body, pic_val_t env);
pic_val_t pic_make_native(pic_val_t (*function)(pic_val_t args));



/*******************************************************************************
 * Global State
 *******************************************************************************/


extern pic_val_t intern_table;        /* association array */
extern pic_val_t current_input_port;
extern pic_val_t current_output_port;
extern pic_val_t current_error_port;


extern pic_val_t pic_define_sym;
extern pic_val_t pic_set_sym;
extern pic_val_t pic_lambda_sym;
extern pic_val_t pic_if_sym;
extern pic_val_t pic_begin_sym;
extern pic_val_t pic_quote_sym;
extern pic_val_t pic_quasiquote_sym;
extern pic_val_t pic_unquote_sym;
extern pic_val_t pic_unquote_splicing_sym;

void pic_init();



/*******************************************************************************
 * Procedures
 *******************************************************************************/


inline bool pic_eqp(pic_val_t x, pic_val_t y) {
  return x == y;
}

bool pic_equalp(pic_val_t x, pic_val_t y);

inline pic_val_t pic_car(pic_val_t val) {
  return pic_pair(val)->car;
}

inline pic_val_t pic_cdr(pic_val_t val) {
  return pic_pair(val)->cdr;
}

inline pic_val_t pic_caar(pic_val_t val) {
  return pic_car(pic_car(val));
}

inline pic_val_t pic_cadr(pic_val_t val) {
  return pic_car(pic_cdr(val));
}

inline pic_val_t pic_cddr(pic_val_t val) {
  return pic_cdr(pic_cdr(val));
}

inline pic_val_t pic_caddr(pic_val_t val) {
  return pic_car(pic_cddr(val));
}

inline pic_val_t pic_cdddr(pic_val_t val) {
  return pic_cdr(pic_cddr(val));
}

inline pic_val_t pic_cadddr(pic_val_t val) {
  return pic_car(pic_cdddr(val));
}

inline pic_val_t pic_cons(pic_val_t car, pic_val_t cdr) {
  return pic_make_pair(car, cdr);
}

inline pic_val_t pic_list(pic_val_t obj1) {
  return pic_cons(obj1, pic_nil);
}

inline pic_val_t pic_list(pic_val_t obj1, pic_val_t obj2) {
  return pic_cons(obj1, pic_list(obj2));
}

inline pic_val_t pic_list(pic_val_t obj1, pic_val_t obj2, pic_val_t obj3) {
  return pic_cons(obj1, pic_list(obj2, obj3));
}

pic_val_t pic_memq(pic_val_t key, pic_val_t list);
pic_val_t pic_assq(pic_val_t key, pic_val_t alist);
pic_val_t pic_assoc(pic_val_t key, pic_val_t alist);
pic_val_t pic_acons(pic_val_t key, pic_val_t val, pic_val_t alist);

pic_val_t pic_read(pic_val_t port = current_input_port);
char pic_read_char(pic_val_t port);
void pic_unread_char(char c, pic_val_t port);

void pic_write(pic_val_t obj, pic_val_t port = current_output_port);
void pic_print(pic_val_t obj, pic_val_t port = current_output_port);
void pic_newline(pic_val_t port);

pic_val_t pic_env_new(pic_val_t parent);
pic_val_t pic_env_get(pic_val_t sym, pic_val_t env);
void pic_env_add(pic_val_t sym, pic_val_t val, pic_val_t env);
void pic_env_set(pic_val_t sym, pic_val_t val, pic_val_t env);

pic_val_t pic_scheme_report_environment();

pic_val_t pic_eval(pic_val_t form, pic_val_t env);
pic_val_t pic_apply(pic_val_t proc, pic_val_t args);


#endif
