# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"

.text

	# Multiboot header (required for GRUB to boot us)
	.long MULTIBOOT_HEADER_MAGIC
	.long MULTIBOOT_HEADER_FLAGS
	.long -(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_HEADER_FLAGS)

# Entrypoint to the kernel
.globl  start, _start

.align 4

start:
_start:
	# Make sure interrupts are off
	cli
	jmp     continue

continue:
	# LOAD THE GDT
    lgdt    gdt_desc_ptr
	lidt	idt_desc_ptr

	# Load CS segment with the new descriptor value
	ljmp    $KERNEL_CS, $keep_going

keep_going:
	# Set up ESP so we can have an initial stack
	movl    $0x800000, %esp

	# Set up the rest of the segment selector registers
	movw    $KERNEL_DS, %cx   
	movw    %cx, %ss    #ss = stack segment
	movw    %cx, %ds 	#ds = data segment
	movw    %cx, %es    #es = extra segment
	movw    %cx, %fs    #fs = general segment
	movw    %cx, %gs    #gs = general segment 

	# Push the parameters that entry() expects (see kernel.c):
	# eax = multiboot magic
	# ebx = address of multiboot info struct
	pushl   %ebx
	pushl   %eax

	# Jump to the C entrypoint to the kernel.
	call    entry

	# We'll never get back here, but we put in a hlt anyway.
halt:
	hlt
	jmp     halt
