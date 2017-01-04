#ifndef INT_HANDLER_H
#define INT_HANDLER_H
#ifndef ASM
#include "types.h"
#include "lib.h"
#include "devices.h"
#include "i8259.h"
#include "system_calls.h"
#include "x86_desc.h"
#include "scheduler.h"

extern uint32_t EXC_DIV_ZERO_F();
extern uint32_t EXC_DEBUG_F();
extern uint32_t EXC_NMI_F();
extern uint32_t EXC_BREAKPOINT_F();
extern uint32_t EXC_OVERFLOW_F();
extern uint32_t EXC_BOUNDS_F();
extern uint32_t EXC_INV_OPCODE_F();
extern uint32_t EXC_COPRO_UNAVAIL_F();
extern uint32_t EXC_DOUBLE_FAULT_F();
extern uint32_t EXC_COPRO_SEG_OVERRUN_F();
extern uint32_t EXC_INV_TSS_F();
extern uint32_t EXC_SEG_NOT_PRES_F();
extern uint32_t EXC_STACK_FAULT_F();
extern uint32_t EXC_GEN_PROTECT_FAULT_F();
extern uint32_t EXC_PAGE_FAULT_F();
extern uint32_t EXC_RES_F();
extern uint32_t EXC_MATH_FAULT_F();
extern uint32_t EXC_ALGN_CHK_F();
extern uint32_t EXC_MCH_CHK_F();
extern uint32_t EXC_SIMD_FP_F();
//system caller (checkpt 3)
extern uint32_t EXC_INT_F();

extern uint32_t INT_KEYBOARD();
extern uint32_t INT_RTC();
extern uint32_t INT_PIT();
extern void user_mode(unsigned int entry, unsigned int stack_point);
extern uint32_t getEBP();
extern uint32_t getESP();
extern void setEBX(uint8_t);

extern uint32_t kernEsp;
extern uint32_t kernEbp;

#endif /* ASM */
#endif/* INT_HANDLER_H */