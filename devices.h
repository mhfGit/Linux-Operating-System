#ifndef DEVICES_H
#define DEVICES_H

#include "types.h"				//use data types from here
#include "lib.h"				//use putc in order to write characters to the screen
#include "i8259.h"				//use enable_irq in order to initialize some drivers
#include "system_calls.h"

#define VIDEO 				0xB8000
#define NUM_COLS 			80
#define NUM_ROWS 			25
#define ATTRIB 				0x7
#define MAXBUFFERSIZE		128

void init_keyboard(void);
void init_rtc(void);
void rtc_redo(void);
unsigned char getChar(void);
void updateBuffer(unsigned char);
void newLineFunc(int TermVal);
void updateCursor(int s_x, int s_y);

int32_t init_pit();

#endif