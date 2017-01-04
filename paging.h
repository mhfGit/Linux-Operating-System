#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "enable_paging.h"
#include "lib.h"

#define PDE 		1024
#define PTE 		1024
#define PAGE_SIZE 	4096
#define VID_PAGE 	0xB8
#define KERN_ADDR 	0x00400000
#define PAGEOFFSET 	22
#define VIDEO 		0xB8000
#define SHELL1VID	0xB9000
#define SHELL2VID	0xBA000
#define SHELL3VID	0xBB000

void init_paging();
void init_PDT();
void init_PT();
void insert_4MBPage(unsigned int address, unsigned int flags, unsigned int process);
void insert_PageTable(unsigned int flags);

#endif