#define ASM     1
#include "int_handler.h"

.data

NUM_VEC = 256
BYTES_IN_LONG = 4
ERROR = -1
MINSYS = 1
MAXSYS = 8
TABLEOFFSET = 4
USER_DS = 0x002B
USER_CS = 0x0023


TEST:
	.string "a\n"

SAY_VEC:
	.string "Interrupt vector number 0x%x.\n"

EXC_DIV_ZERO:
	.string "Division by zero.\n"

EXC_DEBUG:
	.string "Debugger.\n"

EXC_NMI:
	.string "Non-maskable Interrupt.\n"

EXC_BREAKPOINT:
	.string "Breakpoint.\n"

EXC_OVERFLOW:
	.string "Overflow.\n"

EXC_BOUNDS:
	.string "Bounds.\n"

EXC_INV_OPCODE:
	.string "Invalid Opcode.\n"

EXC_COPRO_UNAVAIL:
	.string "Coprocessor not available.\n"

EXC_DOUBLE_FAULT:
	.string "Double fault.\n"

EXC_COPRO_SEG_OVERRUN:
	.string "Coprocessor segment overrun.\n"

EXC_INV_TSS:
	.string "Invalid task state segment.\n"

EXC_SEG_NOT_PRES:
	.string "Segment not present.\n"

EXC_STACK_FAULT:
	.string "Stack fault.\n"

EXC_GEN_PROTECT_FAULT:
	.string "General protection fault.\n"

EXC_PAGE_FAULT:
	.string "Page fault.\n"

EXC_RES:
	.string "reserved\n"

EXC_MATH_FAULT:
	.string "Math fault.\n"

EXC_ALGN_CHK:
	.string "Alignment check.\n"

EXC_MCH_CHK:
	.string "Machine check.\n"

EXC_SIMD_FP:
	.string "MIND Floating-Point Exception.\n"


SYSTEM_TABLE:
	.long halt, execute, read, write, open, close, getargs, vidmap

.text

.globl EXC_DIV_ZERO_F,EXC_DEBUG_F,EXC_NMI_F,EXC_BREAKPOINT_F,EXC_OVERFLOW_F,EXC_BOUNDS_F
.globl EXC_INV_OPCODE_F,EXC_COPRO_UNAVAIL_F,EXC_DOUBLE_FAULT_F,EXC_COPRO_SEG_OVERRUN_F
.globl EXC_INV_TSS_F,EXC_SEG_NOT_PRES_F,EXC_STACK_FAULT_F,EXC_GEN_PROTECT_FAULT_F
.globl EXC_PAGE_FAULT_F,EXC_RES_F,EXC_MATH_FAULT_F,EXC_ALGN_CHK_F,EXC_MCH_CHK_F,EXC_SIMD_FP_F
.globl INT_KEYBOARD, INT_RTC, EXC_INT_F, user_mode, getEBP, getESP, setEBX, INT_PIT
.globl getuserESP, getuserEIP

// 	Exception Handler
//  Inputs: void
//  Return Value: void
//	Function: Each exception goes through the code below to get the correct string. After the correct string and number
//			  are placed into %eax and %ecx the exception continues to print it to the screen.

EXC_DIV_ZERO_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_DIV_ZERO,%eax
	movl $0x0,%ecx
	jmp continue

EXC_DEBUG_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_DEBUG,%eax
	movl $0x1,%ecx
	jmp continue

EXC_NMI_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_NMI,%eax
	movl $0x2,%ecx
	jmp continue

EXC_BREAKPOINT_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_BREAKPOINT,%eax
	movl $0x3,%ecx
	jmp continue

EXC_OVERFLOW_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_OVERFLOW,%eax
	movl $0x4,%ecx
	jmp continue

EXC_BOUNDS_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_BOUNDS,%eax
	movl $0x5,%ecx
	jmp continue

EXC_INV_OPCODE_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_INV_OPCODE,%eax
	movl $0x6,%ecx
	jmp continue

EXC_COPRO_UNAVAIL_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_COPRO_UNAVAIL,%eax
	movl $0x7,%ecx
	jmp continue

EXC_DOUBLE_FAULT_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_DOUBLE_FAULT,%eax
	movl $0x8,%ecx
	jmp continue

EXC_COPRO_SEG_OVERRUN_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_COPRO_SEG_OVERRUN,%eax
	movl $0x9,%ecx
	jmp continue

EXC_INV_TSS_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_INV_TSS,%eax
	movl $0xa,%ecx
	jmp continue

EXC_SEG_NOT_PRES_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_SEG_NOT_PRES,%eax
	movl $0xb,%ecx
	jmp continue

EXC_STACK_FAULT_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_STACK_FAULT,%eax
	movl $0xc,%ecx
	jmp continue

EXC_GEN_PROTECT_FAULT_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_GEN_PROTECT_FAULT,%eax
	movl $0xd,%ecx
	jmp continue

EXC_PAGE_FAULT_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_PAGE_FAULT,%eax
	movl $0xe,%ecx
	jmp continue

EXC_RES_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_RES,%eax
	movl $0xf,%ecx
	jmp continue

EXC_MATH_FAULT_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_MATH_FAULT,%eax
	movl $0x10,%ecx
	jmp continue

EXC_ALGN_CHK_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_ALGN_CHK,%eax
	movl $0x11,%ecx
	jmp continue

EXC_MCH_CHK_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_MCH_CHK,%eax
	movl $0x12,%ecx
	jmp continue

EXC_SIMD_FP_F:
	push %ebp 
	movl %ebp, %esp 
	movl $EXC_SIMD_FP,%eax
	movl $0x13,%ecx
	jmp continue

continue:

	push %eax				
	call clear
	movl $SAY_VEC,%edx
	push %ecx
	push %edx
	call printf
	addl $8,%esp
	cmpl $0, %eax
	je   error
	call printf
	cmpl $0, %eax
	je 	 error

spin:
	jmp spin

error:
	movl $ERROR,%eax
	leave
	ret

// 	INT_KEYBOARD();
//  Inputs: void
//  Return Value: void
//	Function: Acquires the character from the port associated with the keyboard. The character then goes through multiple checks
//			  and will eventually be updated in a keyboard buffer if it passes all checks. When this is done an eoi is sent for
//			  another interrupt to happen.
INT_KEYBOARD:
	cli
	pushal
	call getChar
	push %eax
	call updateBuffer
	push $1				//IRQ number for keyboard
	call send_eoi
	add $8, %esp
	pushf
	movl (%esp),%eax
	or $0x200,%eax
	add $4, %esp
	movl %eax,40(%esp)
	popal
	iret

// 	INT_RTC();
//  Inputs: void
//  Return Value: void
//	Function: Sends some garbage to a RTC port in order for the RTC to continue interrupting.
// 			  Then send eoi for hardware.
INT_RTC:
	cli
	pushal
	call rtc_redo
	push $8			//IRQ number for keyboard
	call send_eoi
	add $4, %esp
	pushf
	movl (%esp),%eax
	or $0x200,%eax
	add $4, %esp
	movl %eax,40(%esp)
	popal
	iret
	
// 	INT_PIT();
//  Inputs: void
//  Return Value: void
//	Function: Save ESP and EBP values in order to return back to them later for the iret context which was
// 			  pushed onto the stack.
INT_PIT:
	cli
	movl %esp, kernEsp
	movl %ebp, kernEbp
	pushal
	call schedule
	push $0 				//IRQ number for PIT
	call send_eoi
	add $4, %esp
	pushf
	movl (%esp),%eax
	or $0x200,%eax
	add $4, %esp
	movl %eax,40(%esp)
	popal
	movl kernEsp, %esp
	movl kernEbp, %ebp
	iret

//System caller (for checkpoint 3...)
// 	EXC_INT_F();
//  Inputs: arguements are passed in through registers, %eax, %ebx, %ecx, %edx
//			sometimes not all are used but we will assume they will be.
//  Return Value: void
//	Function: System call handler is responsible for using eax to jump to the correct system call function.
//			  Also, pushing the arguements from user to the respective function that will be called.
EXC_INT_F:
	cli
	push %ecx					#save all registers
	push %edx					#can't use pushal instruction because
	push %ebx					#it saves eax, and eax has return value
	push %esp
	push %ebp
	push %esi
	push %edi

	push %edx					#push the third arg
	push %ecx					#push the second arg
	push %ebx					#push the first arg

    cmpl $MINSYS, %eax			# Call numbers range from 1 - 8 so these numbers
    jl ERROR_INT_F				# are the minimum and maximum values of eax
    cmpl $MAXSYS, %eax
    jg ERROR_INT_F

    addl $-1, %eax				# subtract 1 to get the correct index for jump table
    call *SYSTEM_TABLE(,%eax,TABLEOFFSET)
    jmp done_call

ERROR_INT_F:
	mov $ERROR, %eax
done_call:
	addl $12, %esp 				# 12 represents here the three pushed in arguements edx, ecx, ebx
	push %eax
	movl 40(%esp),%eax
	or $0x200,%eax				# 0x200 is the interrupt flag in the EFLAGS register
	movl %eax,40(%esp)
	pop %eax
	popl %edi 					# here we restore all the saved values of the registers
	popl %esi 					# pushed onto the stack from before
	popl %ebp
	popl %esp
	popl %ebx
	popl %edx
	popl %ecx
	//sti
	iret


//void user_mode(unsigned int entry, unsigned int stack_point);
//  Inputs: entry: the EIP for the user program to switch to
//			stack_point: The stack pointer for the user program to use for the kernal for system calls.
//  Return Value: void
//	Function: This function sets up an artifial iret context for execute to switch into.
//			  The function will set up the stack based on the passed in parameters such as the entry point and stack point.
user_mode:
	
	movl $USER_DS, %eax		# put user ds into eax so I could do next step compiler error free		 
	movw %ax, %ds 			# put user ds value into ds register
	movl 4(%esp), %eax		# entry value into program, this should be the first instruction
	movl 8(%esp), %ebx

	pushl $USER_DS
	pushl %ebx				# bottom of the 4mb page, user stack pointer
	pushf					# push flags
	pushl $USER_CS
	pushl %eax				# push entry_point
	iret					

//uint32_t getEBP();
//  Inputs: void
//  Return Value: uint32_t: the EBP value that we find 
//	Function: Simple function that returns the current EBP.
getEBP:
	movl %ebp, %eax
	ret
	
//uint32_t getESP();
//  Inputs: void
//  Return Value: uint32_t: the ESP value that we find 
//	Function: Simple function that returns the current ESP.
getESP:
	movl %esp,%eax
	addl $4, %eax
	ret

//void setEBX(uint8_t);
//  Inputs: uint8_t: the value to be placed into eax for a return call
//  Return Value: void
//	Function: Simple function that returns the blaces the lower byte of ebx into eax, for a return call after halt.
setEBX:
	xorl %eax,%eax
	movb %bl, %al
	ret

.end