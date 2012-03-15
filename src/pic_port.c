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

#define NEXTC()  pic_read_raw(pic, parser->port)
#define BACKC(c) (pic_unread_raw(pic, (c), (parser->port)))

#define IS_SPACE isspace
#define IS_DELIMITER(x) (IS_SPACE(x) || x == '(' || x == ')' || x == ';')


static void get_uinteger(USE_PIC, USE_PARSER)
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
    DATA = PIC_TO_FIXNUM(k);
}


static void get_symbol(USE_PIC, USE_PARSER)
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
    DATA = pic_make_symbol(pic, str);
}

static void get_token(USE_PIC, USE_PARSER)
{
    char c = NEXTC();

    if (IS_SPACE(c)) return get_token(pic, parser);

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
        return get_uinteger(pic, parser);

    default:
        BACKC(c);
        return get_symbol(pic, parser);
    }
}




static void next(USE_PIC, USE_PARSER)
{
    if (!parser->backtrack) get_token(pic, parser);
    parser->backtrack = false;
}

static void back(USE_PARSER)
{
    parser->backtrack = true;
}




static PicObj parse(USE_PIC, USE_PARSER);


static PicObj parse_pair(USE_PIC, USE_PARSER)
{
    PicObj car, cdr;
    
    next(pic, parser);

    if (KIND == T_CLOSE) {
        return PIC_NIL;
    } else {
        back(parser);
        car = parse(pic, parser);
        cdr = parse_pair(pic, parser);
        return pic_cons(pic, car, cdr);
    }
}


static PicObj parse(USE_PIC, USE_PARSER)
{
    next(pic, parser);

    switch (KIND) {
    case T_SYMBOL:
    case T_NUMBER:
        return DATA;
    case T_PAIR:
        return parse_pair(pic, parser);
    case T_CLOSE:
    case T_EOF:
        abort();
    }
}


PicObj pic_read(USE_PIC, PicObj port)
{
    Parser parser;
    parser.port = port;
    parser.backtrack = false;
    return parse(pic, &parser);
}


char pic_read_raw(USE_PIC, PicObj port)
{
    return getc(PIC_PORT_FILE(port));
}

void pic_unread_raw(USE_PIC, char c, PicObj port)
{
    ungetc(c, PIC_PORT_FILE(port));
}


/*******************************************************************************
 * Write
 *******************************************************************************/


void pic_write(USE_PIC, PicObj obj, PicObj port)
{
    FILE * file = PIC_PORT_FILE(port);
    
    if (PIC_FIXNUMP(obj)) {
        fprintf(file, "%d", PIC_FROM_FIXNUM(obj));
    } else if (PIC_HEAPP(obj)) {
        switch (PIC_TYPEOF(obj)) {
        case PIC_TYPE_PAIR:
            fprintf(file, "(");
            pic_write(pic, PIC_CAR(obj), port);
            fprintf(file, " . ");
            pic_write(pic, PIC_CDR(obj), port);
            fprintf(file, ")");
            break;
        case PIC_TYPE_SYMBOL:
            fprintf(stdout, "%s", PIC_STRING_DATA(PIC_SYMBOL_REP(obj)));
            break;
        case PIC_TYPE_STRING:
            fprintf(file, "#<string>");
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
