#include "picrin.h"

#include <ctype.h>
#include <string.h>

static inline bool is_delimiter(char c) {
  return isspace(c) || c == '(' || c == ')' || c == ';';
}

static pic_val_t pic_read_abbriviated(pic_val_t port, pic_val_t symbol) {
  pic_val_t succeeding = pic_read(port);

  if (pic_voidp(succeeding)) {
    perror("EOF while reading");
    abort();
  }

  return pic_list(symbol, succeeding);
}


pic_val_t pic_read(pic_val_t port)
{
  // TODO: error handling when the file were not of input stream

  char c = pic_read_char(port);

  if (isspace(c)) {
    return pic_read(port);
  }

  switch (c) {
    case EOF: {
      return pic_void;
    }

    case ';': {
      while ((c = pic_read_char(port)) != '\n') ;
      return pic_read(port);
    }
        
    case '(': {
      while (isspace(c = pic_read_char(port))) ;

      if (c == ')') {
        return pic_nil;
      }
      else if (c == '.') {
        perror("No values before '.'");
        abort();
      }

      pic_val_t list, val;
      pic_pair_t *last_pair;

      // first element
      pic_unread_char(c, port);
      val = pic_read(port);

      if (pic_voidp(val)) {
        perror("EOF while reading list");
        abort();
      }

      list = pic_cons(val, pic_nil);
      last_pair = pic_pair(list);

      for (;;) {
        c = pic_read_char(port);
        
        if (c == ')') {
          return list;
        }
        else if (c == '.') {
          val = pic_read(port);

          if (pic_voidp(val)) {
            perror("EOF while reading list");
            abort();
          }

          last_pair->cdr = val;

          // expects ')'
          while (isspace((c = pic_read_char(port)))) ;
          if (c != ')') {
            perror("Too many values after '.'");
            abort();
          }
          
          return list;
        }
        else {
          pic_unread_char(c, port);
          val = pic_read(port);

          if (pic_voidp(val)) {
            perror("EOF while reading list");
            abort();
          }

          last_pair->cdr = pic_cons(val, pic_nil);
          last_pair = pic_pair(last_pair->cdr);
        }
      }
    }
        
    case ')': {
      perror("unbalanced parenthesis");
      abort();
    }

    case '\'': {
      return pic_read_abbriviated(port, pic_quote_sym);
    }

    case '`': {
      return pic_read_abbriviated(port, pic_quasiquote_sym);
    }

    case ',': {
      pic_val_t sym;

      c = pic_read_char(port);
      if (c == '@') {
        sym = pic_unquote_splicing_sym;
      }
      else {
        sym = pic_unquote_sym;
      }

      return pic_read_abbriviated(port, sym);
    }

    case '#': {
      c = pic_read_char(port);

      switch (c) {
        case 't': {
          return pic_true;
        }

        case 'f': {
          return pic_false;
        }

        default: {
          perror("unexpected character after '#'");
          abort();
        }
      }
    }

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
      int num = c - '0';

      for (;;) {
        c = pic_read_char(port);
        if (is_delimiter(c)) {
          pic_unread_char(c, port);
          break;
        } else {
          num = num * 10 + (c - '0');
        }
      }

      return pic_fixnum(num);
    }

    default: {
      std::string str(1, c);

      for (;;) {
        c = pic_read_char(port);
        if (is_delimiter(c)) {
          pic_unread_char(c, port);
          break;
        } else {
          str += c;
        }
      }

      return pic_make_symbol(str);
    }
  }
}
  
char pic_read_char(pic_val_t port)
{
  FILE *input = pic_port(port)->file;
  return getc(input);
}

void pic_unread_char(char c, pic_val_t port)
{
  FILE *input = pic_port(port)->file;
  ungetc(c, input);
}


/*******************************************************************************
 * Write
 *******************************************************************************/


void pic_write(pic_val_t obj, pic_val_t port)
{
  FILE * file = pic_port(port)->file;
    
  if (pic_fixnump(obj)) {
    fprintf(file, "%d", pic_int(obj));
  }
  else if (pic_objectp(obj)) {
    switch (pic_typeof(obj)) {
      case PIC_PAIR_T: {
        fprintf(file, "(");
        pic_write(pic_car(obj), port);
        obj = pic_cdr(obj);
        for (;;) {
          if (pic_nilp(obj)) {
            fprintf(file, ")");
            break;
          }
          else if (pic_pairp(obj)) {
            fprintf(file, " ");
            pic_write(pic_car(obj), port);
            obj = pic_cdr(obj);
          } else {
            fprintf(file, " . ");
            pic_write(obj, port);
            fprintf(file, ")");
            break;
          }
        }
        break;
      }
      case PIC_SYMBOL_T: {
        fprintf(file, "%s", pic_string(pic_symbol(obj)->str)->str.c_str());
        break;
      }
      case PIC_STRING_T: {
        fprintf(file, "\"%s\"", pic_string(obj)->str.c_str());
        break;
      }
      case PIC_PORT_T: {
        fprintf(file, "#<port>");
        break;
      }
      case PIC_NATIVE_T: {
        fprintf(file, "#<native>");
        break;
      }
    }
  } else {

#define ccase(x,y) case x: fprintf(file, y); break;

    switch (obj) {
      ccase(0x03, "OP_PUSH");
      ccase(0x13, "OP_CALL");
      ccase(0x23, "OP_GREF");
      ccase(0x33, "OP_GSET");
      ccase(0x43, "OP_JMP");
      ccase(0x53, "OP_JMZ");

      case pic_true:
        fprintf(file, "#t");
        break;
      case pic_false:
        fprintf(file, "#f");
        break;
      case pic_nil:
        fprintf(file, "()");
        break;
      case pic_void:
        break;
      case pic_undef:
        fprintf(file, "#undef");
        break;
    }
  }

  fflush(stdout);
}

void pic_print(pic_val_t obj, pic_val_t port)
{
    pic_write(obj, port);
    pic_newline(port);
}

void pic_newline(pic_val_t port)
{
  FILE * output = pic_port(port)->file;
  fputs("\n", output);
}
