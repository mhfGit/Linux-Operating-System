#include "scheduler.h"

#define TERM1 0
#define TERM2 1
#define TERM3 2
#define VIRT_PRGM_ADDR		0x08000000		// 128MB virtual space for the user program
#define USERTASK_PDE_SETTINGS 0x87
#define MAXPROGPTERM        4 				// most number of programs a single terminal can run
#define MAXTERMS            3 				// total number of terminals
#define KERN_SEG_SIZE		0x00002000 	    // size of each user program's kernel stack
#define KERN_PG_BOT 		0x00800000 		// bottom of the kernel page (virtual and physical)
#define pageSIZE			4096 			// 4kb pages

uint32_t working_Term;
uint32_t kernEsp = 0;
uint32_t kernEbp = 0;

struct PCB shell1;
struct PCB shell2;
struct PCB shell3;

extern struct PCB* recentPCB1;
extern struct PCB* recentPCB2;
extern struct PCB* recentPCB3;
extern int term_processes[MAXTERMS];
extern int curTerm;
extern uint8_t* term1ptr;
extern uint8_t* term2ptr;
extern uint8_t* term3ptr;
extern char* video_memory;
extern int initial_pcb[MAXTERMS];

/*
*   void schedule ()
*   Inputs: None
*   Return: None
*   Function: Switches the current working task from the current one to the next one. This happens on the firing of the PIT.
* 		      This function is also in charge of swapping the video buffers for each terminal into the actual video memory to be displayed.
*/
void schedule()
{
	// depending on the currently displayed terminal, swap the curTerms video memory into actual video memory

	if(initial_pcb[TERM1] == 1)
	{
		init_pcb(&shell1);
		recentPCB1 = &shell1;
		current_PCB = recentPCB1;
		working_Term = TERM1;
		tss.ss0 = KERNEL_DS;													
		tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 
		send_eoi(0);
		sti();
		execute((uint8_t*)"shell");
	}
	if(initial_pcb[TERM2] == 1)
	{
		recentPCB1->kernelESP = kernEsp;	// save the ESP & EBP of this program so that it can be switched back to later
		recentPCB1->kernelEBP = kernEbp;
		init_pcb(&shell2);
		recentPCB2 = &shell2;
		current_PCB = recentPCB2;	
		working_Term = TERM2;
		tss.ss0 = KERNEL_DS;													
		tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 
		send_eoi(0);
		sti();
		execute((uint8_t*)"shell");
	}
	if(initial_pcb[TERM3] == 1)
	{
		recentPCB2->kernelESP = kernEsp;	// save the ESP & EBP of this program so that it can be switched back to later
		recentPCB2->kernelEBP = kernEbp;
		init_pcb(&shell3);
		recentPCB3 = &shell3;
		current_PCB = recentPCB3;
		working_Term = TERM3;
		tss.ss0 = KERNEL_DS;													
		tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 
		send_eoi(0);
		sti();
		execute((uint8_t*)"shell");
	}

	switch(curTerm)
	{
		case TERM1:
			memcpy(video_memory, term1ptr, pageSIZE);	// termptr to video memory, copy the entire page
			break;
		case TERM2:
			memcpy(video_memory, term2ptr, pageSIZE);
			break;
		case TERM3:
			memcpy(video_memory, term3ptr, pageSIZE);
			break;
		default:
			break;
	}

	// check the current working_Term so that we know which terminal we are going from
	if(working_Term == TERM1)
	{
		struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 							// make 100% sure that the current_PCB is the one in the kernel stack
  	    (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);	    

		recentPCB1 = cur_pcb_loc;														// set the most recentPCB in terminal one to this program

		recentPCB1->kernelESP = kernEsp;												// save the ESP & EBP of this program so that it can be switched back to later
		recentPCB1->kernelEBP = kernEbp;

		// check to see if another program in a different terminal exists before we switch to it
		// this saves resources

		switch_to_term2(); // switch to usual next task
	}
	// same commenting logic applies to these next two checks
	else if(working_Term == TERM2)
	{
		struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 
  	    (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);	    
																				
		recentPCB2 = cur_pcb_loc;		

		recentPCB2->kernelESP = kernEsp;
		recentPCB2->kernelEBP = kernEbp;

		switch_to_term3();
	}
	else if(working_Term == TERM3)
	{
		struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 
  	    (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);	    

		recentPCB3 = cur_pcb_loc;		

		recentPCB3->kernelESP = kernEsp;
		recentPCB3->kernelEBP = kernEbp;

		switch_to_term1();
	}

	return;
}

/*
*   void switch_to_term1 ()
*   Inputs: None
*   Return: None
*   Function: Switch settings (PCB, ESP, EBP, TSS) to the program in terminal one so that it is able to run for a time.
*/
void switch_to_term1()
{
	working_Term = TERM1;	// this is now the working term

	struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 							// calculate proper PCB in kernel stack to be safe
  	(term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);

	recentPCB1 = cur_pcb_loc;														
	current_PCB = recentPCB1;
	
	
	kernEsp = recentPCB1->kernelESP;												// retrieve the ESP and EBP values for this terminal's program
	kernEbp = recentPCB1->kernelEBP;

	insert_4MBPage(VIRT_PRGM_ADDR, USERTASK_PDE_SETTINGS, term_processes[working_Term] + working_Term*MAXPROGPTERM + 2); // change page to terminal one's program
	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); // change TSS so that interrupts switch to correct kernel stack

	return;
}

/*
*   void schedule ()
*   Inputs: None
*   Return: None
*   Function: Switch settings (PCB, ESP, EBP, TSS) to the program in terminal two so that it is able to run for a time.
*/
void switch_to_term2()
{
	working_Term = TERM2;														// refer to switch_to_term1() for same logic comments

	struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 
  	(term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);

	recentPCB2 = cur_pcb_loc;
	current_PCB = recentPCB2;
	kernEsp = recentPCB2->kernelESP;
	kernEbp = recentPCB2->kernelEBP;

	insert_4MBPage(VIRT_PRGM_ADDR, USERTASK_PDE_SETTINGS, term_processes[working_Term] + working_Term*MAXPROGPTERM + 2);
	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 

	return;
}

/*
*   void schedule ()
*   Inputs: None
*   Return: None
*   Function: Switch settings (PCB, ESP, EBP, TSS) to the program in terminal three so that it is able to run for a time.
*/
void switch_to_term3()
{
	working_Term = TERM3;														// refer to switch_to_term1() for same logic comments
	struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 
  	(term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);

	recentPCB3 = cur_pcb_loc;
	current_PCB = recentPCB3;
	kernEsp = recentPCB3->kernelESP;
	kernEbp = recentPCB3->kernelEBP;

	insert_4MBPage(VIRT_PRGM_ADDR, USERTASK_PDE_SETTINGS, term_processes[working_Term] + working_Term*MAXPROGPTERM + 2);
	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 

	return;
}





