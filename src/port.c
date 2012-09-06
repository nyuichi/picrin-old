#include "picrin.h"

#include <ctype.h>
#include <string.h>

/*******************************************************************************
 * Read
 *******************************************************************************/



typedef struct Tokenizer {
    enum {
        T_EOF,
        T_BOOLEAN,
        T_NUMBER,
        T_SYMBOL,
        T_PAIR,
        T_CLOSE,
        T_QUOTE,
        T_QUASIQUOTE,
        T_UNQUOTE,
        T_UNQUOTE_SPLICING,
    } kind;
    pic_obj_t data;
} Tokenizer;


typedef struct Parser {
    pic_obj_t port;
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

    if (!strcmp(str, "#t")) {
      KIND = T_BOOLEAN;
      DATA = PIC_TRUE;
      return;
    }
    else if (!strcmp(str, "#f")) {
      KIND = T_BOOLEAN;
      DATA = PIC_FALSE;
      return;
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

    case ';':
        while ((c = NEXTC()) != '\n');
        return get_token(parser);
        
    case '(':
        KIND = T_PAIR;
        return;
        
    case ')':
        KIND = T_CLOSE;
        return;

    case '\'':
        KIND = T_QUOTE;
        return;

    case '`':
        KIND = T_QUASIQUOTE;
        return;

    case ',': {
        char d = NEXTC();
        KIND = (d == '@')? T_UNQUOTE_SPLICING : (BACKC(d), T_UNQUOTE);
        return;
    }

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




static pic_obj_t parse(USE_PARSER);


static pic_obj_t parse_abbr(USE_PARSER, pic_obj_t symbol)
{
    pic_obj_t obj = parse(parser);
    pic_obj_t res = pic_list2(symbol, obj);
    PIC_XDECREF(obj);
    return res;
}

static pic_obj_t parse_pair(USE_PARSER)
{
    next(parser);

    if (KIND == T_CLOSE) {
        return PIC_NIL;
    } else {
        pic_obj_t car, cdr, res;
        back(parser);
        car = parse(parser);
        cdr = parse_pair(parser);
        res = pic_cons(car, cdr);
        PIC_XDECREF(car);
        PIC_XDECREF(cdr);
        return res;
    }
}


static pic_obj_t parse(USE_PARSER)
{
    next(parser);

    switch (KIND) {
    case T_QUOTE: 
        return parse_abbr(parser, PIC_SYMBOL_QUOTE);
    case T_QUASIQUOTE:
        return parse_abbr(parser, PIC_SYMBOL_QUASIQUOTE);
    case T_UNQUOTE:
        return parse_abbr(parser, PIC_SYMBOL_UNQUOTE);
    case T_UNQUOTE_SPLICING:
        return parse_abbr(parser, PIC_SYMBOL_UNQUOTE_SPLICING);
    case T_SYMBOL:
    case T_NUMBER:
    case T_BOOLEAN:
        PIC_XINCREF(DATA);
        return DATA;
    case T_PAIR:
        return parse_pair(parser);
    case T_EOF:
        return PIC_VOID;
    case T_CLOSE:
        abort();
    }
}


pic_obj_t pic_read(pic_obj_t port)
{
    pic_obj_t res;
    Parser parser;
    parser.port = port;
    parser.backtrack = false;
    parser.token.data = PIC_NIL;
    res = parse(&parser);
    PIC_XDECREF(parser.token.data);
    return res;
}


char pic_read_raw(pic_obj_t port)
{
    return getc(PIC_PORT_FILE(port));
}

void pic_unread_raw(char c, pic_obj_t port)
{
    ungetc(c, PIC_PORT_FILE(port));
}


/*******************************************************************************
 * Write
 *******************************************************************************/

static
void pic_write_pair(pic_obj_t obj, pic_obj_t port)
{
    FILE * file = PIC_PORT_FILE(port);

    if (PIC_NILP(obj)) {
        fprintf(file, ")");
    } else if (PIC_PAIRP(obj)) {
        fprintf(file, " ");
        pic_write(PIC_CAR(obj), port);
        pic_write_pair(PIC_CDR(obj), port);
    } else {
        fprintf(file, " . ");
        pic_write(obj, port);
        fprintf(file, ")");
    }
}

void pic_write(pic_obj_t obj, pic_obj_t port)
{
    FILE * file = PIC_PORT_FILE(port);
    
    if (PIC_FIXNUMP(obj)) {
        fprintf(file, "%d", PIC_FROM_FIXNUM(obj));
    } else if (PIC_POINTERP(obj)) {
        switch (PIC_TYPEOF(obj)) {
        case PIC_TYPE_PAIR:
            fprintf(file, "(");
            pic_write(PIC_CAR(obj), port);
            pic_write_pair(PIC_CDR(obj), port);
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
        case PIC_TYPE_FOREIGN:
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

void pic_print(pic_obj_t obj, pic_obj_t port)
{
    pic_write(obj, port);
    pic_newline(port);
}

void pic_newline(pic_obj_t port)
{
    FILE * file = PIC_PORT_FILE(port);
    fputs("\n", file);
}
