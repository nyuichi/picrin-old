#ifndef PICRIN_H
#define PICRIN_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


typedef intptr_t PicObj;

#define PIC_WORD (sizeof(PicObj))



/*******************************************************************************
 * Interpeter
 *******************************************************************************/


typedef struct PicInterp {
    size_t heap_size;
    void * heap_from;
    void * heap_to;
    void * heap_end;
    PicObj intern_table;        /* an association array */
    PicObj curin;
    PicObj curout;
    PicObj curerr;
    PicObj topenv;
}  PicInterp;


PicInterp * pic_new_interp();




/*******************************************************************************
 * Objects
 *******************************************************************************/


#define PIC_TRUE (0x0f)
#define PIC_FALSE (0x1f)
#define PIC_NIL (0x2f)
#define PIC_VOID (0x3f)


#define PIC_OBJ_HEADER PicObj type


typedef struct PicObjTemp {
    PIC_OBJ_HEADER;
} PicObjTemp;


enum {
    PIC_TYPE_PAIR = 0x1f,
    PIC_TYPE_SYMBOL = 0x2f,
    PIC_TYPE_STRING = 0x3f,
    PIC_TYPE_SYNTAX = 0x4f,
    PIC_TYPE_CLOSURE = 0x5f,
    PIC_TYPE_PORT = 0x6f,
    PIC_TYPE_CFUNCTION = 0x7f,
};

enum {
    PIC_SYNTAX_DEFINE,
    PIC_SYNTAX_SET,
    PIC_SYNTAX_LAMBDA,
    PIC_SYNTAX_IF,
    PIC_SYNTAX_QUOTE,
    PIC_SYNTAX_BEGIN,
};



typedef struct PicPair {
    PIC_OBJ_HEADER;
    PicObj car;
    PicObj cdr;
} PicPair;


typedef struct PicString {
    PIC_OBJ_HEADER;
    char data[0];
} PicString;


typedef struct PicSymbol {
    PIC_OBJ_HEADER;
    PicObj rep;
} PicSymbol;
    
    
typedef struct PicPort {
    PIC_OBJ_HEADER;
    FILE * file;
    bool dir;                 /* true for input, false for output */
    bool text;                /* true for textual, false for binary */
    bool stat;                /* true for open, false for close */
} PicPort;


typedef struct PicSyntax {
    PIC_OBJ_HEADER;
    int kind;
    PicObj userdata;                /* (transformer . mac-env) */
} PicSyntax;


typedef struct PicClosure {
    PIC_OBJ_HEADER;
    PicObj pars;
    PicObj body;
    PicObj env;
} PicClosure;


typedef struct PicCFunction {
    PIC_OBJ_HEADER;
    PicObj (*func)(PicInterp * pic, PicObj args);
} PicCFunction;



#define PIC_NILP(obj) ((obj) == PIC_NIL)
#define PIC_TRUEP(obj) ((obj) == PIC_TRUE)
#define PIC_FALSEP(obj) ((obj) == PIC_FALSE)

#define PIC_HEAPP(obj) (((obj)&0x03) == 0)
#define PIC_FIXNUMP(obj) (((obj)&0x03) == 1)

#define PIC_TO_FIXNUM(n) (((n)<<2)+1)
#define PIC_FROM_FIXNUM(obj) ((obj)>>2)

#define PIC_TYPEOF(obj) (((PicObjTemp*)(obj))->type)

#define PIC_PAIRP(obj) (PIC_HEAPP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_PAIR)
#define PIC_STRINGP(obj) (PIC_HEAPP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_STRING)
#define PIC_SYMBOLP(obj) (PIC_HEAPP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYMBOL)
#define PIC_SYNTAXP(obj) (PIC_HEAPP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_SYNTAX)
#define PIC_CLOSUREP(obj) (PIC_HEAPP(obj) && PIC_TYPEOF(obj) == PIC_TYPE_CLOSURE)

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
#define PIC_SYNTAX_DATA(obj) (PIC_SYNTAX(obj)->data)
#define PIC_SYNTAX_KIND(obj) (PIC_SYNTAX(obj)->kind)


#define PIC_CLOSURE(obj) ((PicClosure*)(obj))
#define PIC_CLOSURE_PARS(obj) (PIC_CLOSURE(obj)->pars)
#define PIC_CLOSURE_BODY(obj) (PIC_CLOSURE(obj)->body)
#define PIC_CLOSURE_ENV(obj)  (PIC_CLOSURE(obj)->env)


#define PIC_CFUNCTION(obj) ((PicCFunction*)(obj))
#define PIC_CFUNCTION_FUNC(obj) (PIC_CFUNCTION(obj)->func)





/*******************************************************************************
 * Procedures
 *******************************************************************************/


#define USE_PIC PicInterp * pic


PicObj pic_make_pair(USE_PIC, PicObj car, PicObj cdr);
PicObj pic_make_string(USE_PIC, char * str);
PicObj pic_make_symbol(USE_PIC, char * rep);
PicObj pic_make_port(USE_PIC, FILE * file, bool dir, bool text);
PicObj pic_make_closure(USE_PIC, PicObj pars, PicObj body, PicObj env);
PicObj pic_make_syntax(USE_PIC, int kind, PicObj userdata);
PicObj pic_make_cfunction(USE_PIC, PicObj (*func)(USE_PIC, PicObj args));


bool   pic_eqp(PicObj x, PicObj y);
bool   pic_eqvp(PicObj x, PicObj y);
bool   pic_equalp(PicObj x, PicObj y); /* FIXME */

PicObj pic_cons(USE_PIC, PicObj car, PicObj cdr);
PicObj pic_assq(USE_PIC, PicObj key, PicObj alist);
PicObj pic_assoc(USE_PIC, PicObj key, PicObj alist);
PicObj pic_acons(USE_PIC, PicObj key, PicObj val, PicObj alist);

char   pic_read_raw(USE_PIC, PicObj port);
void   pic_unread_raw(USE_PIC, char c, PicObj port);

PicObj pic_read(USE_PIC, PicObj port);
void   pic_write(USE_PIC, PicObj obj, PicObj port);

PicObj pic_env_new(USE_PIC, PicObj parent);
PicObj pic_env_get(USE_PIC, PicObj sym, PicObj env);
void   pic_env_add(USE_PIC, PicObj sym, PicObj val, PicObj env);
void   pic_env_set(USE_PIC, PicObj sym, PicObj val, PicObj env);
void   pic_env_init(USE_PIC, PicObj env);

PicObj pic_eval(USE_PIC, PicObj form, PicObj env);
PicObj pic_apply(USE_PIC, PicObj proc, PicObj args);


/*******************************************************************************
 * Library
 *******************************************************************************/


PicObj pic_lib_sub(USE_PIC, PicObj args);
PicObj pic_lib_mul(USE_PIC, PicObj args);
PicObj pic_lib_eqn(USE_PIC, PicObj args);


#endif
