/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


 

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
	outb(ICW1, MASTER_8259_PORT); // Sets the Ports up, start with ICW1
	outb(ICW1, SLAVE_8259_PORT);

	outb(ICW2_MASTER, MASTER_DATA); // Now do ICW2
	outb(ICW2_SLAVE, SLAVE_DATA);

	outb(ICW3_MASTER, MASTER_DATA); // Now do ICW3
	outb(ICW3_SLAVE, SLAVE_DATA);

	outb(ICW4, MASTER_DATA); // End with ICW4
	outb(ICW4, SLAVE_DATA);

	master_mask = 0xFB;   //MASK INTERRUPTS, except for IRQ2 on master
	slave_mask = 0xFF; 
	outb(master_mask, MASTER_DATA);
	outb(slave_mask, SLAVE_DATA);	
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	uint8_t temp = 0x01;   //0xFE = 0000 0001
	if(irq_num >= 0 && irq_num < 8)  				//master irq
	{
		master_mask = master_mask & ~(temp << irq_num);
		outb(master_mask, MASTER_DATA);	//send OCW1
	}
	else if(irq_num >= 8 && irq_num < 16)			 //slave irq
	{
		slave_mask = slave_mask & ~(temp << (irq_num - 8));
		outb(slave_mask, SLAVE_DATA);	//send OCW1
	}
	else
	{
		printf("INVALID IRQ WAS SENT (ENABLE_IRQ)!\n");
	}
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	uint8_t temp = 1;
	if(irq_num >= 0 && irq_num < 8)  //master irq values
	{
		master_mask = master_mask | (temp << irq_num);
		outb(master_mask, MASTER_DATA);	//send OCW1
	}
	else if(irq_num >= 8 && irq_num < 16)			 //slave irq values
	{
		slave_mask = slave_mask | (temp << (irq_num - 8));	//8 to offset to correct irq slave value
		outb(slave_mask, SLAVE_DATA);	//send OCW1
	}
	else
	{
		printf("INVALID IRQ WAS SENT (DISABLE_IRQ)!\n");
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	uint8_t eoi;
	if(irq_num >= 0 && irq_num < 8)  				//master irq values
	{
		//write to MASTER_8259_PORT "0x20"
		eoi = EOI | irq_num;
		outb(eoi, MASTER_8259_PORT);
	}
	else if(irq_num >= 8 && irq_num < 16)			 //slave irq values
	{
		//write to SLAVE_8259_PORT "0xA0"
		outb(EOI | (irq_num-8), SLAVE_8259_PORT);
		outb((EOI | 2), MASTER_8259_PORT);
	}
	else
	{
		printf("INVALID IRQ WAS SENT (DISABLE_IRQ)!\n");
	}
	return;
}
