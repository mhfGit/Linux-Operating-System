#ifndef ENABLE_PAGING_H
#define ENABLE_PAGING_H
#ifndef ASM

#include "types.h"

extern void Load_PDBR(uint32_t*);
extern void Do_Paging();

#endif /* ASM */
#endif/* INT_HANDLER_H */