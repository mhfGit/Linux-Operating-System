/*init_idt.c: Set up the IDT with correct entries.
* Entries contain the offset location of the handler's
* address in their respective segment (idt[i].seg_selector defines segment).
* Reserved bits 0 - 4 specify whether the entry is a TRAP, INTERRUPT, or TASK gate.
*/
#include "init_idt.h"

void initIDT(void)
{
	int i;
	for(i = 0; i < NUM_VEC; i++) 				// iterate over the entire IDT to initialize it
	{
		idt[i].reserved4 = 0; 					// this byte is always zeroed
		idt[i].seg_selector = KERNEL_CS; 		// handled by kernel code segment

		if(i == SYS_CALL) 						// int 0x80 system call
		{			
			idt[i].reserved3 = 1;				// 0xe == INTERRUPT Gate
			idt[i].reserved2 = 1;
			idt[i].reserved1 = 1;
			idt[i].reserved0 = 0;
			idt[i].size = 1;					// 0 for interrupt gate
			idt[i].dpl = USER_PRIV; 			// user priveledge because user system call
			idt[i].present = EXISTS; 			// entry exists on the table
		}
		else if(i == 15 || (i < 32 && i > (EXCEPTION_TOTAL - 1))) // intel reserved IDT entries
		{
			continue;							// do not modify these protected entries
		}
		else if((i >= 0 && i < EXCEPTION_TOTAL) || (i == KEYBOARDENTRY) || (i == RTCENTRY) || (i == PIT_ENTRY)) 
		{
			idt[i].reserved3 = 1;				// 0xe == INTERRUPT Gate
			idt[i].reserved2 = 1;
			idt[i].reserved1 = 1;
			idt[i].reserved0 = 0;
			idt[i].size = 1;					// 0 for interrupts; not sure why but says to do it
			idt[i].dpl = KERN_PRIV; 			// interrupts need kernel priveledges
			idt[i].present = EXISTS; 			// entry exists
		}
		else
			idt[i].present = NON_EXIST; 		// not defined on table
	}
		
	//Changes offset values here for all Exceptions
	SET_IDT_ENTRY(idt[0], EXC_DIV_ZERO_F);
	SET_IDT_ENTRY(idt[1], EXC_DEBUG_F);
	SET_IDT_ENTRY(idt[2], EXC_NMI_F);
	SET_IDT_ENTRY(idt[3], EXC_BREAKPOINT_F);
	SET_IDT_ENTRY(idt[4], EXC_OVERFLOW_F);
	SET_IDT_ENTRY(idt[5], EXC_BOUNDS_F);
	SET_IDT_ENTRY(idt[6], EXC_INV_OPCODE_F);
	SET_IDT_ENTRY(idt[7], EXC_COPRO_UNAVAIL_F);
	SET_IDT_ENTRY(idt[8], EXC_DOUBLE_FAULT_F);
	SET_IDT_ENTRY(idt[9], EXC_COPRO_SEG_OVERRUN_F);
	SET_IDT_ENTRY(idt[10], EXC_INV_TSS_F);
	SET_IDT_ENTRY(idt[11], EXC_SEG_NOT_PRES_F);
	SET_IDT_ENTRY(idt[12], EXC_STACK_FAULT_F);
	SET_IDT_ENTRY(idt[13], EXC_GEN_PROTECT_FAULT_F);
	SET_IDT_ENTRY(idt[14], EXC_PAGE_FAULT_F);
	SET_IDT_ENTRY(idt[15], EXC_RES_F);
	SET_IDT_ENTRY(idt[16], EXC_MATH_FAULT_F);
	SET_IDT_ENTRY(idt[17], EXC_ALGN_CHK_F);
	SET_IDT_ENTRY(idt[18], EXC_MCH_CHK_F);
	SET_IDT_ENTRY(idt[19], EXC_SIMD_FP_F);

	//int 0x80 (System Caller/ checkpoint 3)
	SET_IDT_ENTRY(idt[SYS_CALL], EXC_INT_F);

	//Link handlers to correct idt entry
	SET_IDT_ENTRY(idt[PIT_ENTRY], INT_PIT);
	SET_IDT_ENTRY(idt[KEYBOARDENTRY], INT_KEYBOARD);
	SET_IDT_ENTRY(idt[RTCENTRY], INT_RTC);

}

