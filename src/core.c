#include "picrin.h"

#include <string.h>
#include <stdlib.h>


pic_obj_t intern_table;
pic_obj_t curin;
pic_obj_t curout;
pic_obj_t curerr;

pic_obj_t PIC_SYMBOL_QUOTE;
pic_obj_t PIC_SYMBOL_QUASIQUOTE;
pic_obj_t PIC_SYMBOL_UNQUOTE;
pic_obj_t PIC_SYMBOL_UNQUOTE_SPLICING;

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

static void destroy_pair(pic_obj_t obj)
{
    PIC_XDECREF(PIC_CAR(obj));
    PIC_XDECREF(PIC_CDR(obj));
    pic_free(obj);
}

pic_obj_t pic_make_pair(pic_obj_t car, pic_obj_t cdr)
{
    pic_pair_t * obj = pic_malloc(sizeof(pic_pair_t),
                                  PIC_TYPE_PAIR,
                                  destroy_pair);
    PIC_XINCREF(car);
    PIC_XINCREF(cdr);
    PIC_CAR(obj) = car;
    PIC_CDR(obj) = cdr;
    return (pic_obj_t)obj;
}

static void destroy_string(pic_obj_t obj)
{
    pic_free(obj);
}

pic_obj_t pic_make_string(char * str)
{
    pic_string_t * obj = pic_malloc(sizeof(pic_string_t) + strlen(str) + 1,
                                 PIC_TYPE_STRING,
                                 destroy_string);
    strcpy(obj->data, str);
    return (pic_obj_t)obj;
}

static void destroy_symbol(pic_obj_t obj)
{
    PIC_XDECREF(PIC_SYMBOL_REP(obj));
    pic_free(obj);
}

pic_obj_t pic_make_symbol(char * rep)
{
    pic_obj_t str = pic_make_string(rep);
    pic_obj_t dat = pic_assoc(str, intern_table);
    pic_obj_t tmp;
    if (PIC_FALSEP(dat)) {
        pic_symbol_t * obj = pic_malloc(sizeof(pic_symbol_t),
                                     PIC_TYPE_SYMBOL,
                                     destroy_symbol);
        PIC_SYMBOL_REP(obj) = str;

        /* Update intern table */
        tmp = pic_acons(str, (pic_obj_t)obj, intern_table);
        PIC_XDECREF(intern_table);
        intern_table = tmp;
        
        PIC_DECREF(str);
        return (pic_obj_t)obj;
    } else {
        pic_obj_t res = PIC_CDR(dat);
        PIC_INCREF(res);
        PIC_DECREF(str);
        PIC_DECREF(dat);
        return res;
    }
}

static void destroy_port(pic_obj_t obj)
{
    pic_free(obj);
}

pic_obj_t pic_make_port(FILE * file, bool dir, bool text)
{
    pic_port_t * obj = pic_malloc(sizeof(pic_port_t),
                                PIC_TYPE_PORT,
                                destroy_port);
    obj->file = file;
    obj->dir  = dir;
    obj->text = text;
    return (pic_obj_t)obj;
}

static void destroy_syntax(pic_obj_t obj)
{
    pic_free(obj);
}

pic_obj_t pic_make_syntax(int kind)
{
    pic_syntax_t * obj = pic_malloc(sizeof(pic_syntax_t),
                                 PIC_TYPE_SYNTAX,
                                 destroy_syntax);
    obj->kind = kind;
    return (pic_obj_t)obj;
}

static void destroy_closure(pic_obj_t obj)
{
    PIC_XDECREF(PIC_CLOSURE_PARS(obj));
    PIC_XDECREF(PIC_CLOSURE_BODY(obj));
    PIC_XDECREF(PIC_CLOSURE_ENV(obj));
    pic_free(obj);
}

pic_obj_t pic_make_closure(pic_obj_t pars, pic_obj_t body, pic_obj_t env)
{
    pic_closure_t * obj = pic_malloc(sizeof(pic_closure_t),
                                  PIC_TYPE_CLOSURE,
                                  destroy_closure);
    PIC_XINCREF(pars);
    PIC_XINCREF(body);
    PIC_XINCREF(env);
    obj->pars = pars;
    obj->body = body;
    obj->env  = env;
    return (pic_obj_t)obj;
}

static void destroy_foreign(pic_obj_t obj)
{
    pic_free(obj);
}

pic_obj_t pic_make_foreign(pic_obj_t (*func)(pic_obj_t args))
{
    pic_foreign_t * obj = pic_malloc(sizeof(pic_foreign_t),
                                    PIC_TYPE_FOREIGN,
                                    destroy_foreign);
    obj->func = func;
    return (pic_obj_t)obj;
}

