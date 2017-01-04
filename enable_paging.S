#define ASM     1
#include "enable_paging.h"

.text

.globl Load_PDBR, Do_Paging

// void Load_PDBR(uint32_t*);
//   Inputs: uint32_t*: pointer to the Page Directory Table
//   Return Value: void
//	Function: Loads the Page Directory Base Register to the given pointer. This Register
//			  is located in register CR3.
Load_PDBR:
	push %ebp
	movl %esp,%ebp
	push %ecx
	movl 8(%ebp),%ecx
	movl %ecx,%cr3
	pop %ecx
	leave
	ret

// void Do_Paging();
//   Inputs: none
//   Return Value: void
//	Function: Sets the Page Global Enable bit, and Page Size Extension bit in CR4.
//		      Also sets the Paging Enable bit in CR0.
Do_Paging:
	push %ebp
	movl %esp,%ebp
	push %ecx
	movl %cr4,%ecx
	or 	 $0x00000090,%ecx
	movl %ecx,%cr4
	movl %cr0,%ecx
	or 	 $0x80000000,%ecx
	movl %ecx,%cr0
	pop %ecx
	leave
	ret
	
.end