# x86_desc.S - Set up x86 segment descriptors, descriptor tables 
# vim:ts=4 noexpandtab
# comments by Dalton Lee 
# to whomever wrote this, comment your damned code >:(

#define ASM     1
#include "x86_desc.h"

.text #code segment

#labels
.globl  ldt_size, tss_size
.globl  gdt_desc_ptr, ldt_desc, tss_desc	  # changed gdt_desc to gdt_desc_ptr because gdt_desc actually did nothing and we need the ptr to the gdt for boot
.globl  tss, tss_desc_ptr, ldt, ldt_desc_ptr  
.globl  gdt_ptr, gdt
.globl  idt_desc_ptr, idt

.align 4			# Note: alignment doesn't make sense here in general. borisov doesn't even get it. just leave as it is
tss_size:
	.long tss_bottom - tss - 1	# not exactly the size of the segment... but the max index; also true for ldt size

ldt_size:
	.long ldt_bottom - ldt - 1
	.word 0						#Padding; no sense

tss:			
_tss:
	.rept TSS_SIZE	# tss is 104 byte long
	.byte 0
	.endr

tss_bottom:

.align 16
ldt_desc:
	.word KERNEL_LDT	# offset of where the LDT is in the GDT
	.long ldt			# pointer to LDT in physical memory
	
.align 4
gdt_desc_ptr:					# what we changed; low 16 bits specify the 16-bit limit (size -1); upper 32 bits point to the physical gdt in memory
	.word gdt_bottom - gdt - 1
	.long gdt
	
# GDT structure
.align 16
gdt:
_gdt:

	# First GDT entry cannot be used
	.quad 0
	
	# NULL entry
	.quad 0
	
	# Segmentation will not be used
	# CS and DS both are 0-4GB r/w segments
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
	# |----------------------------------------------------------------------|
	# |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
	# | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
	# |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
	# |----------------------------------------------------------------------|
	#
	# |----------------------------------------------------------------------|
	# |                                    |                                 |
	# | Base 15:0                          | Segment Limit 15:0              |
	# |                                    |                                 |
	# |----------------------------------------------------------------------|

gdt_ptr:
	# Set up an entry for kernel CS
	.quad 0x00CF9A000000FFFF

	# Set up an entry for kernel DS
	.quad 0x00CF92000000FFFF

	# Set up an entry for user CS
	.quad 0x00CFFA000000FFFF

	# Set up an entry for user DS
	.quad 0x00CFF2000000FFFF

# Set up an entry for TSS
tss_desc_ptr:
	.quad 0 # leave undefined for now? 

# Set up one LDT
ldt_desc_ptr:
	.quad 0 # leave undefined for now? 

gdt_bottom:

.align 16
ldt:			# ldt is 32 bytes in size
	.rept 4
	.quad 0
	.endr	

ldt_bottom:

.align 4
.word 0 # Padding; makes no sense
idt_desc_ptr:						
	.word idt_bottom - idt - 1		# 16-bit limit (size - 1) of the IDT
	.long idt						# pointer to IDT in physical memory
	
.align  16			
idt:					# IDT; 2048 bytes in size
_idt:
	.rept NUM_VEC
	.quad 0
	.endr

idt_bottom:



	











