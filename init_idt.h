/*init_idt.h: Defines constants used for IDT initializing
*/
#ifndef IDTINITIAL
#define IDTINITIAL

#include "x86_desc.h"						//Needed for Constants, and defintion of IDT structure
#include "int_handler.h"					//Needed for assembler functions to setup offset into entries

#define SYS_CALL         0x80
#define USER_PRIV        3
#define KERN_PRIV        0
#define EXISTS           1
#define NON_EXIST        0
#define EXCEPTION_TOTAL  20

//Index for keyboard interrupt and RTC interrupt
//Value comes from (IRQ number) + 0x20
#define KEYBOARDENTRY    33
#define RTCENTRY         40
#define PIT_ENTRY		 32

void initIDT(void);




#endif