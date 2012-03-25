#include "picrin.h"


#define PIC_HEADER_SIZE sizeof(PicObjHeader)

void * pic_malloc(size_t size, int type, void (*dealloc)(PicObj obj))
{
    size_t real_size
        = ((size + PIC_HEADER_SIZE - 1) / PIC_HEADER_SIZE + 1) * PIC_HEADER_SIZE;

    PicObjHeader * obj = malloc(real_size);
    obj->refc = 1;
    obj->type = type;
    obj->dealloc = dealloc;
    return obj+1;
}

void pic_free(PicObj obj)
{
    free(PIC_HEADEROF(obj));
}
