#include "pcb.h"

int process_num = 0;

/*	void init_pcb(PCB pcb)
*	Purpose: executed each time a new PCB is created so that the PCB has the stdin and stdout
* 			 ready to go.
*	Input: the PCB to be initialized
*	Return: none
*/
void init_pcb(struct PCB* pcb)
{
	
	int i;
	for(i = 0; i < NUM_FILES; i++)											// -initialize entire PCB
	{
		pcb->file_desc[i].fop_table_pointer = f_table;
		pcb->file_desc[i].inode = NULL;										// -set the inode # and file positions of these to error codes because
		pcb->file_desc[i].file_position = ERROR_CODE;						// these values are meaningless and should not be utilized
		pcb->file_desc[i].flags = NOT_USED;									// -files are not in use
	}
	
	pcb->PID = NULL;														// no parent initally, no set stack/base pointer yet
	
	pcb->currentESP = 0;
	pcb->currentEBP = 0;
	pcb->term = 0;
	
	for(i = 0; i < MAXBUFFERSIZE; i++)
	{
		pcb->command[i] = 0;												// clear the buffer out to be safe
	}
	
	// stdin/stdout specific
	pcb->file_desc[STDIN_FD].fop_table_pointer = t_table;					// -set their file operations pointers to point to the terminal table
	pcb->file_desc[STDOUT_FD].fop_table_pointer = t_table;					// because they will be using term_read/term_write respectively
	
	pcb->file_desc[STDIN_FD].flags = IS_USED; 								// -mark each file as in use by setting bit 0 to a one
	pcb->file_desc[STDOUT_FD].flags = IS_USED;
}

/*
* 	void init_ftable()
*   Inputs: none
*   Return Value: none
*   Function: initializes function pointers for the file fop table
*/
void init_ftable()
{
	f_table.read = &file_read;
	f_table.write = &file_write;
	f_table.open = &file_open;
	f_table.close = &file_close;
}

/*
* 	void init_ttable()
*   Inputs: none
*   Return Value: none
*   Function: initializes function pointers for the terminal fop table
*/
void init_ttable()
{									// initialize function pointers to the terminal driver functions
	t_table.read = &term_read;
	t_table.write = &term_write;
	t_table.open = &term_open;
	t_table.close = &term_close;
}

/*
* 	void init_ttable()
*   Inputs: none
*   Return Value: none
*   Function: initializes function pointers for the rtc fop table
*/
void init_rtable()
{
	r_table.read = &rtc_read;
	r_table.write = &rtc_write;
	r_table.open = &rtc_open;
	r_table.close = &rtc_close;
}