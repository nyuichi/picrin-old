#include "picrin.h"

#include <string.h>
#include <stdlib.h>


PicObj intern_table;
PicObj curin;
PicObj curout;
PicObj curerr;

PicObj PIC_SYMBOL_QUOTE;
PicObj PIC_SYMBOL_QUASIQUOTE;
PicObj PIC_SYMBOL_UNQUOTE;
PicObj PIC_SYMBOL_UNQUOTE_SPLICING;

void pic_init()
{
    /* There are implicit increfs to give ownerships to the global variables
       and decrefs to dispose the ownerships by this function.  */
    curin  = pic_make_port(stdin, true, true);
    curout = pic_make_port(stdout, false, true);
    curerr = pic_make_port(stderr, false, true);
    intern_table = PIC_NIL;

    PIC_SYMBOL_QUOTE = pic_make_symbol("quote");
    PIC_SYMBOL_QUASIQUOTE = pic_make_symbol("quasiquote");
    PIC_SYMBOL_UNQUOTE = pic_make_symbol("unquote");
    PIC_SYMBOL_UNQUOTE_SPLICING = pic_make_symbol("unquote-splicing");
}

static void destroy_pair(PicObj obj)
{
    PIC_XDECREF(PIC_CAR(obj));
    PIC_XDECREF(PIC_CDR(obj));
    pic_free(obj);
}

PicObj pic_make_pair(PicObj car, PicObj cdr)
{
    PicPair * obj = pic_malloc(sizeof(PicPair),
                               PIC_TYPE_PAIR,
                               destroy_pair);
    PIC_XINCREF(car);
    PIC_XINCREF(cdr);
    PIC_CAR(obj) = car;
    PIC_CDR(obj) = cdr;
    return (PicObj)obj;
}

static void destroy_string(PicObj obj)
{
    pic_free(obj);
}

PicObj pic_make_string(char * str)
{
    PicString * obj = pic_malloc(sizeof(PicString) + strlen(str) + 1,
                                 PIC_TYPE_STRING,
                                 destroy_string);
    strcpy(obj->data, str);
    return (PicObj)obj;
}

static void destroy_symbol(PicObj obj)
{
    PIC_XDECREF(PIC_SYMBOL_REP(obj));
    pic_free(obj);
}

PicObj pic_make_symbol(char * rep)
{
    PicObj str = pic_make_string(rep);
    PicObj dat = pic_assoc(str, intern_table);
    PicObj tmp;
    if (PIC_FALSEP(dat)) {
        PicSymbol * obj = pic_malloc(sizeof(PicSymbol),
                                     PIC_TYPE_SYMBOL,
                                     destroy_symbol);
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

static void destroy_port(PicObj obj)
{
    pic_free(obj);
}

PicObj pic_make_port(FILE * file, bool dir, bool text)
{
    PicPort * obj = pic_malloc(sizeof(PicPort),
                                PIC_TYPE_PORT,
                                destroy_port);
    obj->file = file;
    obj->dir  = dir;
    obj->text = text;
    return (PicObj)obj;
}

static void destroy_syntax(PicObj obj)
{
    pic_free(obj);
}

PicObj pic_make_syntax(int kind)
{
    PicSyntax * obj = pic_malloc(sizeof(PicSyntax),
                                 PIC_TYPE_SYNTAX,
                                 destroy_syntax);
    obj->kind = kind;
    return (PicObj)obj;
}

static void destroy_closure(PicObj obj)
{
    PIC_XDECREF(PIC_CLOSURE_PARS(obj));
    PIC_XDECREF(PIC_CLOSURE_BODY(obj));
    PIC_XDECREF(PIC_CLOSURE_ENV(obj));
    pic_free(obj);
}

PicObj pic_make_closure(PicObj pars, PicObj body, PicObj env)
{
    PicClosure * obj = pic_malloc(sizeof(PicClosure),
                                  PIC_TYPE_CLOSURE,
                                  destroy_closure);
    PIC_XINCREF(pars);
    PIC_XINCREF(body);
    PIC_XINCREF(env);
    obj->pars = pars;
    obj->body = body;
    obj->env  = env;
    return (PicObj)obj;
}

static void destroy_foreign(PicObj obj)
{
    pic_free(obj);
}

PicObj pic_make_foreign(PicObj (*func)(PicObj args))
{
    PicForeign * obj = pic_malloc(sizeof(PicForeign),
                                    PIC_TYPE_FOREIGN,
                                    destroy_foreign);
    obj->func = func;
    return (PicObj)obj;
}

static void destroy_synclo(PicObj obj)
{
    PIC_XDECREF(PIC_SYNCLO_FREEENV(obj));
    PIC_XDECREF(PIC_SYNCLO_FREEVARS(obj));
    PIC_XDECREF(PIC_SYNCLO_BODY(obj));
    pic_free(obj);
}

PicObj pic_make_synclo(PicObj freeenv, PicObj freevars, PicObj body)
{
    PicSynclo * obj = pic_malloc(sizeof(PicSynclo),
                                 PIC_TYPE_SYNCLO,
                                 destroy_synclo);
    PIC_XINCREF(freeenv);
    PIC_XINCREF(freevars);
    PIC_XINCREF(body);
    obj->freeenv = freeenv;
    obj->freevars = freevars;
    obj->body = body;
    return (PicObj)obj;
}

static void destroy_macro(PicObj obj)
{
    PIC_XDECREF(PIC_MACRO_TRANSFORMER(obj));
    PIC_XDECREF(PIC_MACRO_MACENV(obj));
    pic_free(obj);
}

PicObj pic_make_macro(PicObj transformer, PicObj macenv)
{
    PicMacro * obj = pic_malloc(sizeof(PicMacro),
                                PIC_TYPE_MACRO,
                                destroy_macro);
    PIC_XINCREF(transformer);
    PIC_XINCREF(macenv);
    obj->transformer = transformer;
    obj->macenv = macenv;
    return (PicObj)obj;
}


