#include "picrin.h"

#include <ctype.h>


/*******************************************************************************
 * Read
 *******************************************************************************/



typedef struct Tokenizer {
    enum {
        T_EOF,
        T_NUMBER,
        T_SYMBOL,
        T_PAIR,
        T_CLOSE,
    } kind;
    PicObj data;
} Tokenizer;


typedef struct Parser {
    PicObj port;
    Tokenizer token;
    bool backtrack;
} Parser;


#define USE_PARSER Parser * parser

#define KIND parser->token.kind
#define DATA parser->token.data

#define NEXTC()  pic_read_raw(parser->port)
#define BACKC(c) pic_unread_raw((c), (parser->port))

#define IS_SPACE isspace
#define IS_DELIMITER(x) (IS_SPACE(x) || x == '(' || x == ')' || x == ';')


static void get_uinteger(USE_PARSER)
{
    int k = 0;
    char c;
    for (;;) {
        c = NEXTC();
        if (IS_DELIMITER(c)) {
            BACKC(c);
            break;
        } else {
            k = k*10 + (c - '0');
        }
    }
    KIND = T_NUMBER;
    PIC_XDECREF(DATA);
    DATA = PIC_TO_FIXNUM(k);
}


static void get_symbol(USE_PARSER)
{
    char str[100], *buf = str, c;
    for (;;) {
        c = NEXTC();
        if (IS_DELIMITER(c)) {
            *buf = '\0';
            BACKC(c);
            break;
        } else {
            *buf++ = c;
        }
    }
    KIND = T_SYMBOL;
    PIC_XDECREF(DATA);
    DATA = pic_make_symbol(str);
}

static void get_token(USE_PARSER)
{
    char c = NEXTC();

    if (IS_SPACE(c)) return get_token(parser);

    switch (c) {
    case EOF:
        KIND = T_EOF;
        return;
        
    case '(':
        KIND = T_PAIR;
        return;
        
    case ')':
        KIND = T_CLOSE;
        return;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        BACKC(c);
        return get_uinteger(parser);

    default:
        BACKC(c);
        return get_symbol(parser);
    }
}




static void next(USE_PARSER)
{
    if (!parser->backtrack) get_token(parser);
    parser->backtrack = false;
}

static void back(USE_PARSER)
{
    parser->backtrack = true;
}




static PicObj parse(USE_PARSER);


static PicObj parse_pair(USE_PARSER)
{
    next(parser);

    if (KIND == T_CLOSE) {
        return PIC_NIL;
    } else {
        PicObj car, cdr, res;
        back(parser);
        car = parse(parser);
        cdr = parse_pair(parser);
        res = pic_cons(car, cdr);
        PIC_XDECREF(car);
        PIC_XDECREF(cdr);
        return res;
    }
}


static PicObj parse(USE_PARSER)
{
    next(parser);

    switch (KIND) {
    case T_SYMBOL:
    case T_NUMBER:
        PIC_XINCREF(DATA);
        return DATA;
    case T_PAIR:
        return parse_pair(parser);
    case T_CLOSE:
    case T_EOF:
        abort();
    }
}


PicObj pic_read(PicObj port)
{
    PicObj res;
    Parser parser;
    parser.port = port;
    parser.backtrack = false;
    parser.token.data = PIC_NIL;
    res = parse(&parser);
    PIC_XDECREF(parser.token.data);
    return res;
}


char pic_read_raw(PicObj port)
{
    return getc(PIC_PORT_FILE(port));
}

void pic_unread_raw(char c, PicObj port)
{
    ungetc(c, PIC_PORT_FILE(port));
}


/*******************************************************************************
 * Write
 *******************************************************************************/


void pic_write(PicObj obj, PicObj port)
{
    FILE * file = PIC_PORT_FILE(port);
    
    if (PIC_FIXNUMP(obj)) {
        fprintf(file, "%d", PIC_FROM_FIXNUM(obj));
    } else if (PIC_POINTERP(obj)) {
        switch (PIC_TYPEOF(obj)) {
        case PIC_TYPE_PAIR:
            fprintf(file, "(");
            pic_write(PIC_CAR(obj), port);
            fprintf(file, " . ");
            pic_write(PIC_CDR(obj), port);
            fprintf(file, ")");
            break;
        case PIC_TYPE_SYMBOL:
            fprintf(stdout, "%s", PIC_STRING_DATA(PIC_SYMBOL_REP(obj)));
            break;
        case PIC_TYPE_STRING:
            fprintf(file, "\"%s\"", PIC_STRING_DATA(obj));
            break;
        case PIC_TYPE_SYNTAX:
            fprintf(file, "#<syntax>");
            break;
        case PIC_TYPE_CLOSURE:
            fprintf(file, "#<closure>");
            break;
        case PIC_TYPE_PORT:
            fprintf(file, "#<port>");
            break;
        case PIC_TYPE_CFUNCTION:
            fprintf(file, "#<cfunction>");
            break;
        }
    } else {
        switch (obj) {
        case PIC_TRUE:
            fprintf(file, "#t");
            break;
        case PIC_FALSE:
            fprintf(file, "#f");
            break;
        case PIC_NIL:
            fprintf(file, "()");
            break;
        case PIC_VOID:
            break;
        }
    }

    fflush(stdout);
}
