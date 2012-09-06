#ifndef PICRIN_H
#define PICRIN_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



/*******************************************************************************
 * Low Level Object
 ******************************************************************************/


typedef uintptr_t pic_obj_t;


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
#define PIC_FROM_FIXNUM(obj) ((obj)>>2)


typedef struct pic_header {
    int refc;
    int type;
    void (*dealloc)(pic_obj_t obj);
} pic_header;


#define PIC_HEADEROF(obj) (((pic_header*)(obj))-1)
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


enum {
    PIC_TYPE_PAIR,
    PIC_TYPE_SYMBOL,
    PIC_TYPE_STRING,
    PIC_TYPE_SYNTAX,
    PIC_TYPE_CLOSURE,
    PIC_TYPE_PORT,
    PIC_TYPE_FOREIGN,
};

enum {
    PIC_SYNTAX_DEFINE,
    PIC_SYNTAX_SET,
    PIC_SYNTAX_LAMBDA,
    PIC_SYNTAX_IF,
    PIC_SYNTAX_QUOTE,
    PIC_SYNTAX_BEGIN,
};



typedef struct pic_pair_t {
    pic_obj_t car;
    pic_obj_t cdr;
} pic_pair_t;


typedef struct pic_string_t {
    char data[0];
} pic_string_t;


typedef struct pic_symbol_t {
    pic_obj_t rep;
} pic_symbol_t;
    
    
typedef struct pic_port_t {
    FILE * file;
    bool dir;                 /* true for input, false for output */
    bool text;                /* true for textual, false for binary */
    bool stat;                /* true for open, false for close */
} pic_port_t;


typedef struct pic_syntax_t {
    int kind;
} pic_syntax_t;


typedef struct pic_macro_t {
    pic_obj_t transformer;
    pic_obj_t macenv;
} pic_macro_t;


typedef struct pic_synclo_t {
    pic_obj_t freeenv;
    pic_obj_t freevars;
    pic_obj_t body;
} pic_synclo_t;


typedef struct pic_closure_t {
    pic_obj_t pars;
    pic_obj_t body;
    pic_obj_t env;
} pic_closure_t;


typedef struct pic_foreign_t {
    pic_obj_t (*func)(pic_obj_t args);
} pic_foreign_t;



#define PIC_PAIRP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_PAIR)
#define PIC_STRINGP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_STRING)
#define PIC_SYMBOLP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYMBOL)
#define PIC_SYNTAXP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYNTAX)
#define PIC_CLOSUREP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_CLOSURE)
#define PIC_FOREIGNP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_FOREIGN)
#define PIC_MACROP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_MACRO)


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

#define PIC_SYMBOL(obj) ((pic_symbol_t*)(obj))
#define PIC_SYMBOL_REP(obj) (PIC_SYMBOL(obj)->rep)

#define PIC_STRING(obj) ((pic_string_t*)(obj))
#define PIC_STRING_DATA(obj) (PIC_STRING(obj)->data)

#define PIC_PORT(obj) ((pic_port_t*)(obj))
#define PIC_PORT_FILE(obj) (PIC_PORT(obj)->file)
#define PIC_PORT_DIR(obj)  (PIC_PORT(obj)->dir)
#define PIC_PORT_TYPE(obj) (PIC_PORT(obj)->type)
#define PIC_PORT_STAT(obj) (PIC_PORT(obj)->stat)

#define PIC_SYNTAX(obj) ((pic_syntax_t*)(obj))
#define PIC_SYNTAX_KIND(obj) (PIC_SYNTAX(obj)->kind)

#define PIC_CLOSURE(obj) ((pic_closure_t*)(obj))
#define PIC_CLOSURE_PARS(obj) (PIC_CLOSURE(obj)->pars)
#define PIC_CLOSURE_BODY(obj) (PIC_CLOSURE(obj)->body)
#define PIC_CLOSURE_ENV(obj)  (PIC_CLOSURE(obj)->env)

#define PIC_FOREIGN(obj) ((pic_foreign_t*)(obj))
#define PIC_FOREIGN_FUNC(obj) (PIC_FOREIGN(obj)->func)


pic_obj_t pic_make_pair(pic_obj_t car, pic_obj_t cdr);
pic_obj_t pic_make_string(char * str);
pic_obj_t pic_make_symbol(char * rep);
pic_obj_t pic_make_port(FILE * file, bool dir, bool text);
pic_obj_t pic_make_closure(pic_obj_t pars, pic_obj_t body, pic_obj_t env);
pic_obj_t pic_make_syntax(int kind);
pic_obj_t pic_make_foreign(pic_obj_t (*func)(pic_obj_t args));



/*******************************************************************************
 * Global State
 *******************************************************************************/


extern pic_obj_t intern_table;        /* an association array */
extern pic_obj_t curin;
extern pic_obj_t curout;
extern pic_obj_t curerr;

extern pic_obj_t PIC_SYMBOL_QUOTE;
extern pic_obj_t PIC_SYMBOL_QUASIQUOTE;
extern pic_obj_t PIC_SYMBOL_UNQUOTE;
extern pic_obj_t PIC_SYMBOL_UNQUOTE_SPLICING;

void pic_init();



/*******************************************************************************
 * Procedures
 *******************************************************************************/


bool pic_eqp(pic_obj_t x, pic_obj_t y);
bool pic_eqvp(pic_obj_t x, pic_obj_t y);
bool pic_equalp(pic_obj_t x, pic_obj_t y); /* FIXME */

pic_obj_t pic_cons(pic_obj_t car, pic_obj_t cdr);
pic_obj_t pic_memq(pic_obj_t key, pic_obj_t list);
pic_obj_t pic_assq(pic_obj_t key, pic_obj_t alist);
pic_obj_t pic_assoc(pic_obj_t key, pic_obj_t alist);
pic_obj_t pic_acons(pic_obj_t key, pic_obj_t val, pic_obj_t alist);
pic_obj_t pic_list2(pic_obj_t obj1, pic_obj_t obj2);
pic_obj_t pic_list3(pic_obj_t obj1, pic_obj_t obj2, pic_obj_t obj);

char pic_read_raw(pic_obj_t port);
void pic_unread_raw(char c, pic_obj_t port);

pic_obj_t pic_read(pic_obj_t port);
void pic_write(pic_obj_t obj, pic_obj_t port);
void pic_print(pic_obj_t obj, pic_obj_t port);
void pic_newline(pic_obj_t port);

pic_obj_t pic_env_new(pic_obj_t parent);
pic_obj_t pic_env_get(pic_obj_t sym, pic_obj_t env);
void pic_env_add(pic_obj_t sym, pic_obj_t val, pic_obj_t env);
void pic_env_set(pic_obj_t sym, pic_obj_t val, pic_obj_t env);

pic_obj_t pic_scheme_report_environment();

pic_obj_t pic_eval(pic_obj_t form, pic_obj_t env);
pic_obj_t pic_apply(pic_obj_t proc, pic_obj_t args);


/*******************************************************************************
 * Library
 *******************************************************************************/


pic_obj_t pic_c_eqp(pic_obj_t args);
pic_obj_t pic_c_pairp(pic_obj_t args);
pic_obj_t pic_c_symbolp(pic_obj_t args);

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
