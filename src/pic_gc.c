#include "picrin.h"


#define PIC_HEADER_SIZE sizeof(pic_header_t)

void * pic_malloc(size_t size, int type, void (*dealloc)(pic_obj_t obj))
{
  size_t real_size
    = ((size + PIC_HEADER_SIZE - 1) / PIC_HEADER_SIZE + 1) * PIC_HEADER_SIZE;
  
  pic_header_t * obj = malloc(real_size);
  obj->refc = 1;
  obj->type = type;
  obj->dealloc = dealloc;
  return obj+1;
}

void pic_free(pic_obj_t obj)
{
  free(PIC_HEADEROF(obj));
}

