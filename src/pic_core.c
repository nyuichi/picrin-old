#include "picrin.h"

#include <string.h>
#include <stdlib.h>


PicObj intern_table;
PicObj curin;
PicObj curout;
PicObj curerr;


void pic_init()
{
    /* There are implicit increfs to give ownerships to the global variables
       and decrefs to dispose the ownerships by this function.  */
    curin  = pic_make_port(stdin, true, true);
    curout = pic_make_port(stdout, false, true);
    curerr = pic_make_port(stderr, false, true);
    intern_table = PIC_NIL;
}


static void dealloc_pair(PicObj obj)
{
    PIC_XDECREF(PIC_CAR(obj));
    PIC_XDECREF(PIC_CDR(obj));
    pic_free(obj);
}

static void dealloc_string(PicObj obj)
{
    pic_free(obj);
}

static void dealloc_symbol(PicObj obj)
{
    PIC_XDECREF(PIC_SYMBOL_REP(obj));
    pic_free(obj);
}

static void dealloc_port(PicObj obj)
{
    pic_free(obj);
}

static void dealloc_syntax(PicObj obj)
{
    PIC_XDECREF(PIC_SYNTAX_DATA(obj));
    pic_free(obj);
}

static void dealloc_closure(PicObj obj)
{
    PIC_XDECREF(PIC_CLOSURE_PARS(obj));
    PIC_XDECREF(PIC_CLOSURE_BODY(obj));
    PIC_XDECREF(PIC_CLOSURE_ENV(obj));
    pic_free(obj);
}

static void dealloc_cfunction(PicObj obj)
{
    pic_free(obj);
}


PicObj pic_make_pair(PicObj car, PicObj cdr)
{
    PicPair * obj = pic_malloc(sizeof(PicPair),
                               PIC_TYPE_PAIR,
                               dealloc_pair);
    PIC_XINCREF(car);
    PIC_XINCREF(cdr);
    PIC_CAR(obj) = car;
    PIC_CDR(obj) = cdr;
    return (PicObj)obj;
}

PicObj pic_make_string(char * str)
{
    PicString * obj = pic_malloc(sizeof(PicString) + strlen(str) + 1,
                                 PIC_TYPE_STRING,
                                 dealloc_string);
    strcpy(obj->data, str);
    return (PicObj)obj;
}

PicObj pic_make_symbol(char * rep)
{
    PicObj str = pic_make_string(rep);
    PicObj dat = pic_assoc(str, intern_table);
    PicObj tmp;
    if (PIC_FALSEP(dat)) {
        PicSymbol * obj = pic_malloc(sizeof(PicSymbol),
                                     PIC_TYPE_SYMBOL,
                                     dealloc_symbol);
        PIC_SYMBOL_REP(obj) = str;

        /* Update intern table */
        tmp = pic_acons(str, (PicObj)obj, intern_table);
        PIC_XDECREF(intern_table);
        intern_table = tmp;
        
        PIC_DECREF(str);
        return (PicObj)obj;
    } else {
        PicObj res = PIC_CDR(dat);
        PIC_INCREF(res);
        PIC_DECREF(str);
        PIC_DECREF(dat);
        return res;
    }
}

PicObj pic_make_port(FILE * file, bool dir, bool text)
{
    PicPort * obj = pic_malloc(sizeof(PicPort),
                                PIC_TYPE_PORT,
                                dealloc_port);
    obj->file = file;
    obj->dir  = dir;
    obj->text = text;
    return (PicObj)obj;
}


PicObj pic_make_closure(PicObj pars, PicObj body, PicObj env)
{
    PicClosure * obj = pic_malloc(sizeof(PicClosure),
                                  PIC_TYPE_CLOSURE,
                                  dealloc_closure);
    PIC_XINCREF(pars);
    PIC_XINCREF(body);
    PIC_XINCREF(env);
    obj->pars = pars;
    obj->body = body;
    obj->env  = env;
    return (PicObj)obj;
}


PicObj pic_make_syntax(int kind, PicObj data)
{
    PicSyntax * obj = pic_malloc(sizeof(PicSyntax),
                                 PIC_TYPE_SYNTAX,
                                 dealloc_syntax);
    PIC_XINCREF(data);
    obj->kind = kind;
    obj->data = data;
    return (PicObj)obj;
}


PicObj pic_make_foreign(PicObj (*func)(PicObj args))
{
    PicForeign * obj = pic_malloc(sizeof(PicForeign),
                                    PIC_TYPE_FOREIGN,
                                    dealloc_cfunction);
    obj->func = func;
    return (PicObj)obj;
}

