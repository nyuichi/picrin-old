#ifndef PICRIN_H
#define PICRIN_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



/*******************************************************************************
 * Low Level Object
 ******************************************************************************/


typedef intptr_t pic_obj_t;


#define PIC_TRUE (0x0f)
#define PIC_FALSE (0x1f)
#define PIC_NIL (0x2f)
#define PIC_VOID (0x3f)


#define PIC_NILP(obj) ((obj) == PIC_NIL)
#define PIC_TRUEP(obj) ((obj) == PIC_TRUE)
#define PIC_FALSEP(obj) ((obj) == PIC_FALSE)
#define PIC_VOIDP(obj) ((obj) == PIC_VOID)

#define PIC_POINTERP(obj) (((obj)&0x03) == 0)
#define PIC_FIXNUMP(obj) (((obj)&0x03) == 1)

#define PIC_TO_FIXNUM(n) (((n)<<2)+1)
#define PIC_FROM_FIXNUM(obj) ((int)((obj)>>2))

#define PIC_SYNTAX_DEFINE (0x1ff)
#define PIC_SYNTAX_SET (0x2ff)
#define PIC_SYNTAX_LAMBDA (0x3ff)
#define PIC_SYNTAX_IF (0x4ff)
#define PIC_SYNTAX_QUOTE (0x5ff)
#define PIC_SYNTAX_BEGIN (0x6ff)
#define PIC_SYNTAX_DEFSYNTAX (0x7ff)

#define PIC_SYNTAXP(obj) (((obj)&0xff) == 0xff)



/*******************************************************************************
 * GC
 *******************************************************************************/


typedef struct {
  int refc;
  int type;
  void (*dealloc)(pic_obj_t obj);
} pic_header_t;


#define PIC_HEADEROF(obj) (((pic_header_t*)(obj))-1)
#define PIC_REFCOF(obj) (PIC_HEADEROF(obj)->refc)
#define PIC_TYPEOF(obj) (PIC_HEADEROF(obj)->type)
#define PIC_DEALLOC(obj) (PIC_HEADEROF(obj)->dealloc)

#define PIC_INCREF(obj) (++PIC_REFCOF(obj))
#define PIC_DECREF(obj) ((--PIC_REFCOF(obj))?: pic_free(obj))

#define PIC_XINCREF(obj) ((PIC_POINTERP(obj))? PIC_INCREF(obj) : (void)0)
#define PIC_XDECREF(obj) ((PIC_POINTERP(obj))? PIC_DECREF(obj) : (void)0)

void * pic_malloc(size_t size, int type, void (*dealloc)(pic_obj_t obj));
void   pic_free(pic_obj_t obj);




/*******************************************************************************
 * High Level Object
 *******************************************************************************/


enum pic_types {
  PIC_TYPE_PAIR,
  PIC_TYPE_SYMBOL,
  PIC_TYPE_STRING,
  PIC_TYPE_PORT,
  PIC_TYPE_ENV,
};


/* --- Runtime Objects --- */

typedef struct {
  pic_obj_t car;
  pic_obj_t cdr;
} pic_pair_t;


typedef struct {
  char data[0];
} pic_string_t;


typedef struct {
  pic_obj_t rep;
} pic_symbol_t;
    
    
typedef struct {
  FILE * file;
  bool dir;                 /* true for input, false for output */
  bool text;                /* true for textual, false for binary */
  bool stat;                /* true for open, false for close */
} pic_port_t;



/* --- Compile-time Objects --- */

/* TODO: want to remove function from env */

typedef struct {
  pic_obj_t parent;
  pic_obj_t function;
  pic_obj_t table;		/* association list for now */
} pic_env_t;


#define PIC_PAIRP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_PAIR)
#define PIC_STRINGP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_STRING)
#define PIC_SYMBOLP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYMBOL)
#define PIC_PORTP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_PORT)
#define PIC_ENVP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_ENV)


/* These accessors LENT the ownership of the return value. */

#define PIC_PAIR(obj) ((pic_pair_t*)(obj))
#define PIC_CAR(obj) (PIC_PAIR(obj)->car)
#define PIC_CDR(obj) (PIC_PAIR(obj)->cdr)
#define PIC_CAAR(obj) (PIC_CAR(PIC_CAR(obj)))
#define PIC_CADR(obj) (PIC_CAR(PIC_CDR(obj)))
#define PIC_CDDR(obj) (PIC_CDR(PIC_CDR(obj)))
#define PIC_CADDR(obj) (PIC_CAR(PIC_CDDR(obj)))
#define PIC_CDDDR(obj) (PIC_CDR(PIC_CDDR(obj)))
#define PIC_CADDDR(obj) (PIC_CAR(PIC_CDDDR(obj)))
#define PIC_CDDDDR(obj) (PIC_CDR(PIC_CDDDR(obj)))
#define PIC_CADDDDR(obj) (PIC_CAR(PIC_CDDDDR(obj)))
#define PIC_CDDDDDR(obj) (PIC_CDR(PIC_CDDDDR(obj)))
#define PIC_CADDDDDR(obj) (PIC_CAR(PIC_CDDDDDR(obj)))

#define PIC_SYMBOL(obj) ((pic_symbol_t*)(obj))
#define PIC_SYMBOL_REP(obj) (PIC_SYMBOL(obj)->rep)

#define PIC_STRING(obj) ((pic_string_t*)(obj))
#define PIC_STRING_DATA(obj) (PIC_STRING(obj)->data)

#define PIC_PORT(obj) ((pic_port_t*)(obj))
#define PIC_PORT_FILE(obj) (PIC_PORT(obj)->file)
#define PIC_PORT_DIR(obj) (PIC_PORT(obj)->dir)
#define PIC_PORT_TYPE(obj) (PIC_PORT(obj)->type)
#define PIC_PORT_STAT(obj) (PIC_PORT(obj)->stat)

#define PIC_ENV(obj) ((pic_env_t*)(obj))
#define PIC_ENV_PARENT(obj) (PIC_ENV(obj)->parent)
#define PIC_ENV_FUNCTION(obj) (PIC_ENV(obj)->function)
#define PIC_ENV_TABLE(obj) (PIC_ENV(obj)->table)


pic_obj_t pic_make_pair(pic_obj_t car, pic_obj_t cdr);
pic_obj_t pic_make_string(char * str);
pic_obj_t pic_make_symbol(char * rep);
pic_obj_t pic_make_port(FILE * file, bool dir, bool text);
pic_obj_t pic_make_env(pic_obj_t base_env, pic_obj_t function);



/*******************************************************************************
 * Global State
 *******************************************************************************/


extern pic_obj_t standard_input_port;
extern pic_obj_t standard_output_port;
extern pic_obj_t standard_error_port;

void pic_init();



/*******************************************************************************
 * Procedures
 *******************************************************************************/


bool pic_eqp(pic_obj_t x, pic_obj_t y);
bool pic_eqvp(pic_obj_t x, pic_obj_t y);
bool pic_equalp(pic_obj_t x, pic_obj_t y); /* FIXME */

pic_obj_t pic_intern(pic_obj_t symbol);
pic_obj_t pic_get_interned_symbol(char * rep);

pic_obj_t pic_cons(pic_obj_t car, pic_obj_t cdr);
bool pic_listp(pic_obj_t obj);
pic_obj_t pic_list1(pic_obj_t obj1);
pic_obj_t pic_list2(pic_obj_t obj1, pic_obj_t obj2);
pic_obj_t pic_list3(pic_obj_t obj1, pic_obj_t obj2, pic_obj_t obj3);
pic_obj_t pic_list4(pic_obj_t obj1, pic_obj_t obj2, pic_obj_t obj3, pic_obj_t obj4);
pic_obj_t pic_memq(pic_obj_t key, pic_obj_t list);
pic_obj_t pic_assq(pic_obj_t key, pic_obj_t alist);
pic_obj_t pic_assoc(pic_obj_t key, pic_obj_t alist);
pic_obj_t pic_acons(pic_obj_t key, pic_obj_t val, pic_obj_t alist);

pic_obj_t pic_read(pic_obj_t port);
char pic_read_raw(pic_obj_t port);
void pic_unread_raw(char c, pic_obj_t port);
void pic_write(pic_obj_t obj, pic_obj_t port);
void pic_print(pic_obj_t obj, pic_obj_t port);
void pic_newline(pic_obj_t port);

bool pic_env_globalp(pic_obj_t env);
void pic_env_def(pic_obj_t key, pic_obj_t val, pic_obj_t env);
void pic_env_set(pic_obj_t key, pic_obj_t val, pic_obj_t env);
pic_obj_t pic_env_get(pic_obj_t key, pic_obj_t env);

pic_obj_t pic_scheme_report_environment();

pic_obj_t pic_eval(pic_obj_t form, pic_obj_t env);
pic_obj_t pic_apply(pic_obj_t proc, pic_obj_t args);


/*******************************************************************************
 * Library
 *******************************************************************************/


pic_obj_t pic_c_eqp(pic_obj_t args);

pic_obj_t pic_c_add(pic_obj_t args);
pic_obj_t pic_c_sub(pic_obj_t args);
pic_obj_t pic_c_mul(pic_obj_t args);
pic_obj_t pic_c_eqn(pic_obj_t args);

pic_obj_t pic_c_nullp(pic_obj_t args);
pic_obj_t pic_c_car(pic_obj_t args);
pic_obj_t pic_c_cdr(pic_obj_t args);
pic_obj_t pic_c_cons(pic_obj_t args);

pic_obj_t pic_c_write(pic_obj_t args);

#endif
