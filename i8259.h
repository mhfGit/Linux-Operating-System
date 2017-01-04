/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT 0x20
#define MASTER_DATA 0x21
#define SLAVE_8259_PORT  0xA0
#define SLAVE_DATA 0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1    	  0x11 //0001 0001	
#define ICW2_MASTER   0x20 //0010 0000
#define ICW2_SLAVE    0x28 //0010 1000
#define ICW3_MASTER   0x04 //0000 0100
#define ICW3_SLAVE    0x02 //0000 0010
#define ICW4          0x01 //0000 0001

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI             0x60 //0110 0000

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
extern void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */