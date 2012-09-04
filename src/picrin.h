#ifndef PICRIN_H
#define PICRIN_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



/*******************************************************************************
 * Low Level Object
 ******************************************************************************/


typedef intptr_t PicObj;


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


typedef struct PicObjHeader {
    int refc;
    int type;
    void (*dealloc)(PicObj obj);
} PicObjHeader;


#define PIC_HEADEROF(obj) (((PicObjHeader*)(obj))-1)
#define PIC_REFCOF(obj) (PIC_HEADEROF(obj)->refc)
#define PIC_TYPEOF(obj) (PIC_HEADEROF(obj)->type)
#define PIC_DEALLOC(obj) (PIC_HEADEROF(obj)->dealloc)

#define PIC_INCREF(obj) (++PIC_REFCOF(obj))
#define PIC_DECREF(obj) ((--PIC_REFCOF(obj))?: pic_free(obj))

#define PIC_XINCREF(obj) ((PIC_POINTERP(obj))? PIC_INCREF(obj) : (void)0)
#define PIC_XDECREF(obj) ((PIC_POINTERP(obj))? PIC_DECREF(obj) : (void)0)

void * pic_malloc(size_t size, int type, void (*dealloc)(PicObj obj));
void   pic_free(PicObj obj);


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
    PIC_TYPE_SYNCLO,
    PIC_TYPE_MACRO,
};

enum {
    PIC_SYNTAX_DEFINE,
    PIC_SYNTAX_SET,
    PIC_SYNTAX_LAMBDA,
    PIC_SYNTAX_IF,
    PIC_SYNTAX_QUOTE,
    PIC_SYNTAX_BEGIN,
    PIC_SYNTAX_DEFSYNTAX,
};



typedef struct PicPair {
    PicObj car;
    PicObj cdr;
} PicPair;


typedef struct PicString {
    char data[0];
} PicString;


typedef struct PicSymbol {
    PicObj rep;
} PicSymbol;
    
    
typedef struct PicPort {
    FILE * file;
    bool dir;                 /* true for input, false for output */
    bool text;                /* true for textual, false for binary */
    bool stat;                /* true for open, false for close */
} PicPort;


typedef struct PicSyntax {
    int kind;
} PicSyntax;


typedef struct PicMacro {
    PicObj transformer;
    PicObj macenv;
} PicMacro;


typedef struct PicSynclo {
    PicObj freeenv;
    PicObj freevars;
    PicObj body;
} PicSynclo;


typedef struct PicClosure {
    PicObj pars;
    PicObj body;
    PicObj env;
} PicClosure;


typedef struct PicForeign {
    PicObj (*func)(PicObj args);
} PicForeign;



#define PIC_PAIRP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_PAIR)
#define PIC_STRINGP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_STRING)
#define PIC_SYMBOLP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYMBOL)
#define PIC_SYNTAXP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYNTAX)
#define PIC_CLOSUREP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_CLOSURE)
#define PIC_FOREIGNP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_FOREIGN)
#define PIC_SYNCLOP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYNCLO)
#define PIC_MACROP(obj) (PIC_POINTERP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_MACRO)


/* These accessors LENT the ownership of the return value. */

#define PIC_PAIR(obj) ((PicPair*)(obj))
#define PIC_CAR(obj) (PIC_PAIR(obj)->car)
#define PIC_CDR(obj) (PIC_PAIR(obj)->cdr)
#define PIC_CAAR(obj) (PIC_CAR(PIC_CAR(obj)))
#define PIC_CADR(obj) (PIC_CAR(PIC_CDR(obj)))
#define PIC_CDDR(obj) (PIC_CDR(PIC_CDR(obj)))
#define PIC_CADDR(obj) (PIC_CAR(PIC_CDDR(obj)))
#define PIC_CDDDR(obj) (PIC_CDR(PIC_CDDR(obj)))
#define PIC_CADDDR(obj) (PIC_CAR(PIC_CDDDR(obj)))

#define PIC_SYMBOL(obj) ((PicSymbol*)(obj))
#define PIC_SYMBOL_REP(obj) (PIC_SYMBOL(obj)->rep)

#define PIC_STRING(obj) ((PicString*)(obj))
#define PIC_STRING_DATA(obj) (PIC_STRING(obj)->data)

#define PIC_PORT(obj) ((PicPort*)(obj))
#define PIC_PORT_FILE(obj) (PIC_PORT(obj)->file)
#define PIC_PORT_DIR(obj)  (PIC_PORT(obj)->dir)
#define PIC_PORT_TYPE(obj) (PIC_PORT(obj)->type)
#define PIC_PORT_STAT(obj) (PIC_PORT(obj)->stat)

#define PIC_SYNTAX(obj) ((PicSyntax*)(obj))
#define PIC_SYNTAX_KIND(obj) (PIC_SYNTAX(obj)->kind)

#define PIC_CLOSURE(obj) ((PicClosure*)(obj))
#define PIC_CLOSURE_PARS(obj) (PIC_CLOSURE(obj)->pars)
#define PIC_CLOSURE_BODY(obj) (PIC_CLOSURE(obj)->body)
#define PIC_CLOSURE_ENV(obj)  (PIC_CLOSURE(obj)->env)

#define PIC_MACRO(obj) ((PicMacro*)(obj))
#define PIC_MACRO_TRANSFORMER(obj) (PIC_MACRO(obj)->transformer)
#define PIC_MACRO_MACENV(obj) (PIC_MACRO(obj)->macenv)

#define PIC_SYNCLO(obj) ((PicSynclo*)(obj))
#define PIC_SYNCLO_FREEENV(obj) (PIC_SYNCLO(obj)->freeenv)
#define PIC_SYNCLO_FREEVARS(obj) (PIC_SYNCLO(obj)->freevars)
#define PIC_SYNCLO_BODY(obj) (PIC_SYNCLO(obj)->body)

#define PIC_FOREIGN(obj) ((PicForeign*)(obj))
#define PIC_FOREIGN_FUNC(obj) (PIC_FOREIGN(obj)->func)


PicObj pic_make_pair(PicObj car, PicObj cdr);
PicObj pic_make_string(char * str);
PicObj pic_make_symbol(char * rep);
PicObj pic_make_port(FILE * file, bool dir, bool text);
PicObj pic_make_closure(PicObj pars, PicObj body, PicObj env);
PicObj pic_make_syntax(int kind);
PicObj pic_make_macro(PicObj transformer, PicObj macenv);
PicObj pic_make_synclo(PicObj freeenv, PicObj freevars, PicObj body);
PicObj pic_make_foreign(PicObj (*func)(PicObj args));



/*******************************************************************************
 * Global State
 *******************************************************************************/


extern PicObj intern_table;        /* an association array */
extern PicObj curin;
extern PicObj curout;
extern PicObj curerr;

extern PicObj PIC_SYMBOL_QUOTE;
extern PicObj PIC_SYMBOL_QUASIQUOTE;
extern PicObj PIC_SYMBOL_UNQUOTE;
extern PicObj PIC_SYMBOL_UNQUOTE_SPLICING;

void pic_init();



/*******************************************************************************
 * Procedures
 *******************************************************************************/


bool pic_eqp(PicObj x, PicObj y);
bool pic_eqvp(PicObj x, PicObj y);
bool pic_equalp(PicObj x, PicObj y); /* FIXME */

PicObj pic_cons(PicObj car, PicObj cdr);
PicObj pic_memq(PicObj key, PicObj list);
PicObj pic_assq(PicObj key, PicObj alist);
PicObj pic_assoc(PicObj key, PicObj alist);
PicObj pic_acons(PicObj key, PicObj val, PicObj alist);
PicObj pic_list2(PicObj obj1, PicObj obj2);
PicObj pic_list3(PicObj obj1, PicObj obj2, PicObj obj);

char pic_read_raw(PicObj port);
void pic_unread_raw(char c, PicObj port);

PicObj pic_read(PicObj port);
void pic_write(PicObj obj, PicObj port);
void pic_print(PicObj obj, PicObj port);
void pic_newline(PicObj port);

PicObj pic_env_new(PicObj parent);
PicObj pic_env_get(PicObj sym, PicObj env);
void pic_env_add(PicObj sym, PicObj val, PicObj env);
void pic_env_set(PicObj sym, PicObj val, PicObj env);

PicObj pic_scheme_report_environment();

PicObj pic_eval(PicObj form, PicObj env);
PicObj pic_apply(PicObj proc, PicObj args);


/*******************************************************************************
 * Library
 *******************************************************************************/


PicObj pic_c_eqp(PicObj args);

PicObj pic_c_add(PicObj args);
PicObj pic_c_sub(PicObj args);
PicObj pic_c_mul(PicObj args);
PicObj pic_c_eqn(PicObj args);

PicObj pic_c_nullp(PicObj args);
PicObj pic_c_car(PicObj args);
PicObj pic_c_cdr(PicObj args);
PicObj pic_c_cons(PicObj args);

PicObj pic_c_make_synclo(PicObj args);

PicObj pic_c_write(PicObj args);

#endif
