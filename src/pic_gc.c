#include "picrin.h"


#include <string.h>


static void * alloc(USE_PIC, size_t size)
{
/*
    if (pic->heap_from + size < pic->heap_end) {
        void * obj = pic->heap_from;
        pic->heap_from += size;
        return obj;
    } else {
        abort();                // not yet implemented
    }
*/
    return malloc(size);
}


PicObj pic_make_pair(USE_PIC, PicObj car, PicObj cdr)
{
    PicPair * obj = alloc(pic, sizeof(PicPair));
    obj->type = PIC_TYPE_PAIR;
    obj->car  = car;
    obj->cdr  = cdr;
    return (PicObj)obj;
}


PicObj pic_make_string(USE_PIC, char * str)
{
    size_t size = sizeof(PicString) + sizeof(char)*(strlen(str)+1);
    
    PicString * obj = alloc(pic, size);
    obj->type = PIC_TYPE_STRING;
        
    strcpy(obj->data, str);
    
    return (PicObj)obj;
}


PicObj pic_make_symbol(USE_PIC, char * rep)
{
    PicSymbol * sym;
    PicObj str = pic_make_string(pic, rep);
    PicObj dat = pic_assoc(pic, str, pic->intern_table);
    if (PIC_FALSEP(dat)) {
        sym = alloc(pic, sizeof(PicSymbol));
        sym->type = PIC_TYPE_SYMBOL;
        sym->rep  = str;
        
        pic->intern_table = pic_acons(pic, str, (PicObj)sym,
                                      pic->intern_table);

        return (PicObj)sym;
    } else {
        return PIC_CDR(dat);
    }
}


PicObj pic_make_port(USE_PIC, FILE * file, bool dir, bool text)
{
    PicPort * obj = alloc(pic, sizeof(PicPort));
    obj->type = PIC_TYPE_PORT;
    obj->file = file;
    obj->dir = dir;
    obj->text = text;
    obj->stat = true;
    return (PicObj)obj;
}


PicObj pic_make_closure(USE_PIC, PicObj pars, PicObj body, PicObj env)
{
    PicClosure * obj = alloc(pic, sizeof(PicClosure));
    obj->type = PIC_TYPE_CLOSURE;
    obj->pars = pars;
    obj->body = body;
    obj->env = env;
    return (PicObj)obj;
}


PicObj pic_make_syntax(USE_PIC, int kind, PicObj userdata)
{
    PicSyntax * obj = alloc(pic, sizeof(PicSyntax));
    obj->type = PIC_TYPE_SYNTAX;
    obj->kind = kind;
    obj->userdata = userdata;
    return (PicObj)obj;
}


PicObj pic_make_cfunction(USE_PIC, PicObj (*func)(USE_PIC, PicObj args))
{
    PicCFunction * obj = alloc(pic, sizeof(PicCFunction));
    obj->type = PIC_TYPE_CFUNCTION;
    obj->func = func;
    return (PicObj)obj;
}

