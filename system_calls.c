//***************FOR CHECKPOINT 2********

#include "system_calls.h"

//defines for RTC
#define PORTVAL_RTC         0x70
#define DATAPORT_RTC        0x71

#define MAXBUFFERSIZE       128
#define ERROR_CODE			-1
#define SUCCESS_CODE 		0
#define MAXFREQ             32768
#define REGAVAL             0x8A
#define MINRATE             6
#define MAXRATE             15
#define MINFD               0
#define MAXFD               7
#define pageSIZE            4096

#define REG_FILE			2
#define RTC_ACCESS 			0 
#define DIRECTORY 			1
#define EXE_CHK_SIZE		4
#define ENTRY_PT_SIZE 		4
#define ENTRY_PT_BEGIN 		24
#define BYTE_SIZE			8
#define KERN_PG_BOT 		0x00800000
#define KERN_SEG_SIZE		0x00002000
#define VIRT_PRGM_ADDR		0x08000000
#define VIRT_PRGM_END       0x08400000
#define USERTASK_PDE_SETTINGS  0x87
#define VIRT_PRGM_EXE_ADDR 0x08048000
#define ORIG_PROCESS 		-1
#define MAX_ARGS			3 				// three arguments can be passed into execute along with the main command
#define DIR_FLAG			2
#define MAX_PROCESSES 		4				// allow only six processes to run per terminal
#define MAXTERMS            3
#define MAXPROGPTERM        4
#define TERM1               0
#define TERM2               1
#define TERM3               2

extern unsigned char termReadBuffer[MAXBUFFERSIZE];
extern volatile int enter_flag[MAXTERMS];
extern volatile int rtcFlag[MAXTERMS];
extern int term_x[MAXTERMS];
extern int term_y[MAXTERMS];
extern char* video_memory;
extern tss_t tss;
extern int process_num;
extern int curTerm;
extern uint32_t working_Term;
extern uint8_t* term1ptr;
extern uint8_t* term2ptr;
extern uint8_t* term3ptr;
extern int keyBoardFlag;


int curEsp;
int curEbp;
int initial_pcb[MAXTERMS] = {1, 1, 1};
int term_processes[MAXTERMS] = {0, 0 ,0};

struct PCB* recentPCB1;
struct PCB* recentPCB2;
struct PCB* recentPCB3;


/*
*   int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes)
*   Inputs: file descriptor of where to read input
			0 = standard input
			1 = standard output
			2 = standard error
			...or use the file desciptor # obtained from 
					the "open" system call
		buf = character array where read content is stored
		nbytes = number of bytes to read before truncating 
					data. If read data is smaller than "nbytes",
					then all data is saved in the buffer
*   Return Value: number of bytes read, -1 on failure
*	Function: wait until next interrupt occurs, then return 0
*/
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes)
{
    sti();
    rtcFlag[working_Term] = 1;

    while(rtcFlag[working_Term]);

    cli();
    return SUCCESS_CODE;
}

/*
*   int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd =  file descriptor of where to read input
				0 = standard input
				1 = standard output
				2 = standard error
				...or use the file desciptor # obtained from 
					the "open" system call
			freq = (in Hz) value to set the RTC's frequency to
			nbytes = number of bytes to write, if smaller than
					provided buffer, output is truncated
*   Return Value: number of bytes written, -1 on failure
*	Function: writes frequency (buffer) to the RTC 
*/
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes)
{

    //get value to input into rtc register
    if(buf == 0)
        return ERROR_CODE;
    

    int * freq = (int *) buf;
    if(MAXFREQ % *freq)
        return ERROR_CODE;

    uint32_t val = MAXFREQ / *freq;
    uint32_t rate = 1;
    while(val >= 2)                                 //Correct stopping point of where to stop shifting right
    {
        rate += 1;
        val = val >> 1;
    }
    if(rate < MINRATE || rate >  MAXRATE)           //invalid rate: 2 is the lowest value rate can be and 15 is the highest
        return ERROR_CODE;          

    char prev;

    outb(REGAVAL, PORTVAL_RTC);                     //0x8A represents masking NMI and register A
    prev = inb(DATAPORT_RTC);
    outb(REGAVAL, PORTVAL_RTC);
    outb((prev & 0xF0) | rate, DATAPORT_RTC);       //0xF0 clears lower 4 bits and replaces it with 15, the value of the rate  

    return SUCCESS_CODE;
}

/*
*   int32_t rtc_open (const uint8_t* filename)
*   Inputs: name of file to open
*   Return Value: returns file descriptor # of opened file. 
				   This value is always the smallest integer 
				   greater than zero that is still available.
				   OTHERWISE, return -1 on non-existant file 
				   or no descriptor to free
*	Function: initilalizes and sets RTC freq to 2Hz
*/
int32_t rtc_open ()
{
    init_rtc(); //will initialize rtc and set interrupt freq. to 2Hz
    return SUCCESS_CODE;
}

/*
*   int32_t rtc_close (int32_t fd)
*   Inputs: file descriptor number to close
*   Return Value: 0 on success, -1 on invalid close
*	Function: "good to have RTC stop or reset, but not necessary"
*/
int32_t rtc_close (int32_t fd)
{
    return ERROR_CODE;
}







//FILE SYSTEM STUFF 

/*
*   int32_t file_read (int32_t fd, void* buf, int32_t nbytes)
*   Inputs: file descriptor of where to read input
*            0 = standard input
*            1 = standard output
*            2 = standard error
*            ...or use the file desciptor # obtained from 
*                    the "open" system call
*        buf = character array where read content is stored
*        nbytes = number of bytes to read before truncating 
*                    data. If read data is smaller than "nbytes",
*                    then all data is saved in the buffer
*   Return Value: number of bytes read, -1 on failure
*   Function: reads data from the keyboard, a file, or directory 
*/
int32_t file_read (int32_t fd, void* buf, int32_t nbytes)
{		
	// check to see if what's being read is a file or a file directory
	if(current_PCB->file_desc[fd].flags == (IS_USED | DIR_FLAG))	// file directory
	{
		int32_t ret = read_directory(fd, buf);						// read directory
		return ret;
															// returns 0 upon reaching end of directory
															// else returns the number of bytes read (32 for file name)
	}	
	
	// read file normally
	int32_t ret = read_data(*current_PCB->file_desc[fd].inode, 0, buf, nbytes); // user cannot specify an offset
	return ret;
}


/*
*   int32_t file_write (int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd =  file descriptor of where to read input
*                0 = standard input
*                1 = standard output
*                2 = standard error
*                ...or use the file desciptor # obtained from 
*                    the "open" system call
*            buf = null terminated character string to write
*            nbytes = number of bytes to write, if smaller than
*                    provided buffer, output is truncated
*   Return Value: number of bytes written, -1 on failure
*   Function: writes data to the terminal or to a device (RTC) 
*/
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes)
{
    return ERROR_CODE;		// read only file system
}


/*
*   int32_t file_open (const uint8_t* filename)
*   Inputs: name of file to open
*   Return Value: returns file descriptor # of opened file. 
*                   This value is always the smallest integer 
*                   greater than zero that is still available.
*
*                   OTHERWISE, return -1 on non-existant file 
*                   or no descriptor to free
*   Function: provides access to the file system. Can use this
*               function to obtain the file descriptor # used for
*               read/write operations (the above functions).
*/
int32_t file_open(const uint8_t* filename)
{
	int i;
	struct dir_entry e;
	int32_t read_error = read_dentry_by_name(filename, &e); // gather file information into diretory entry e & return error is unsuccessful
	if(read_error == ERROR_CODE)
		return ERROR_CODE;

    int BBindex = ERROR_CODE;						// check every file in order to see if the inode num passed in matches one of the files
    for(i = 0; i < tot_num_files; i++)                                
    {
        if(e.inode_num == bb->files[i].inode_num)	// if a match, copy the file index
        {
            BBindex = i;
            break;
        }
    }
    if(BBindex < 0)
        return ERROR_CODE;

	for(i = 0; i < NUM_FILES; i++)		
	{
		if((current_PCB->file_desc[i].flags & FD_USAGE) == NOT_USED) // if the fd is not in use
		{
			if(e.file_type == REG_FILE) // treat the file differently if it is a regularly readable file than if it were an executable, directory, or rtc
			{
				current_PCB->file_desc[i].fop_table_pointer = f_table;	// set the file operation jump table
				current_PCB->file_desc[i].inode = &bb->files[BBindex].inode_num; 
				current_PCB->file_desc[i].file_position = FILE_START;               
			}
			else if(e.file_type == DIRECTORY)
			{
				current_PCB->file_desc[i].fop_table_pointer = f_table;	// set the file operation jump table
				current_PCB->file_desc[i].inode = &bb->files[BBindex].inode_num; 
				current_PCB->file_desc[i].file_position = FILE_START;  
				current_PCB->file_desc[i].flags |= DIR_FLAG;	// mark this fd as a directory
			}
			else
			{
                current_PCB->file_desc[i].fop_table_pointer = r_table;
				current_PCB->file_desc[i].inode = NULL;
				current_PCB->file_desc[i].file_position = ERROR_CODE;
				if(rtc_open())
					return ERROR_CODE;
			}
			current_PCB->file_desc[i].flags |= IS_USED; // mark fd as in use
			return i;
		}
	}
    return ERROR_CODE;	// if the file is never found or no fd's are available, return error
}


/*
*   int32_t file_close (int32_t fd)
*   Inputs: file descriptor number to close
*   Return Value: 0 on success, -1 on invalid close
*   Function: closes specified file descriptor
*/
int32_t file_close (int32_t fd)
{
	if(fd == STDIN_FD || fd == STDOUT_FD || fd < 0 || fd >= NUM_FILES)	// -check to see if fd is in bounds
		return ERROR_CODE;												// -cannot close STDIN or STDOUT

	current_PCB->file_desc[fd].flags = 0; 					// preserves all other bit flags and zeroes the 0th bit
																		// to signify that the FD is no longer in use
	return SUCCESS_CODE;
}







//KEYBOARD/TERMINAL STUFF
//Essentially reading data from terminal/keyboard input

/*
*   int32_t term_read (int32_t fd, void* buf, int32_t nbytes)
*   Inputs: file descriptor of where to read input
*            0 = standard input
*            1 = standard output
*            2 = standard error
*            ...or use the file desciptor # obtained from 
*                    the "open" system call
*        buf = character array where read content is stored
*        nbytes = number of bytes to read before truncating 
*                    data. If read data is smaller than "nbytes",
*                    then all data is saved in the buffer
*   Return Value: number of bytes read, -1 on failure
*   Function: reads data from the keyboard, a file, or directory 
*/
int32_t term_read (int32_t fd, void* buf, int32_t nbytes)
{
    int i = 0;
	enter_flag[working_Term] = 0;
    unsigned char * buffer = (unsigned char *) buf;

    struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 
    (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);

    if(nbytes > MAXBUFFERSIZE)
        nbytes = MAXBUFFERSIZE;
    if(buf == 0 || nbytes < 0)
    {    
        return ERROR_CODE;
    }
    if(nbytes == 0)
    {
        return 0;
    }
    sti();
    if(cur_pcb_loc->term == TERM1)
        while(enter_flag[TERM1] != 1 || working_Term != curTerm);
    else if(cur_pcb_loc->term == TERM2)
        while(enter_flag[TERM2] != 1 || working_Term != curTerm);
    else if(cur_pcb_loc->term == TERM3)
        while(enter_flag[TERM3] != 1 || working_Term != curTerm);
    cli();
    while(termReadBuffer[i] != '\n' && (i != nbytes))     //10 represents the Enter key here
    {
        *buffer = termReadBuffer[i];
        buffer++;
        i++;
    }
    *buffer = termReadBuffer[i];                            //place '\n' into final place
    
 
    return i + 1;   //number of bytes read
}

/* int32_t myprint(const void* buf)
*	Purpose: Helper function to abstract away from the system call that needs a file descriptor and length.
*	 Inputs: buf - array of chars to print to terminal
*	 Return: -1 on error
* 			 how many bytes were written on success
*/
int32_t myprint(const void* buf)
{
	int32_t length = strlen(buf);
	return term_write(0, buf, length); 
}

/*
*   int32_t term_write (int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd =  file descriptor of where to read input
*                0 = standard input
*                1 = standard output
*                2 = standard error
*                ...or use the file desciptor # obtained from 
*                    the "open" system call
*            buf = null terminated character string to write
*            nbytes = number of bytes to write, if smaller than
*                    provided buffer, output is truncated
*   Return Value: number of bytes written, -1 on failure
*   Function: writes data to the terminal or to a device (RTC) 
*/
int32_t term_write (int32_t fd, const void* buf, int32_t nbytes)
{
    int countBytes = 0;     //counter for number of bytes written to the terminal
    int i = 0;              //loop over the buffer
    unsigned char * buffer = (unsigned char *)buf;

    if(buffer == 0 || nbytes < 0)
    {
        return ERROR_CODE;          //if the buffer is null then return -1
    }
    while(countBytes != nbytes)
    {
		if(buffer[i] == '\n')
		{
			goto NewLine1;
		}
		switch(working_Term)
		{
			case TERM1:
				*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[TERM1] + term_x[TERM1]) << 1)) = buffer[i];
				*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[TERM1] + term_x[TERM1]) << 1) + 1) = ATTRIB;
                if(TERM1==curTerm) memcpy(video_memory, term1ptr, pageSIZE);
				term_x[TERM1]++;
				break;
			case TERM2:
				*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[TERM2] + term_x[TERM2]) << 1)) = buffer[i];
				*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[TERM2] + term_x[TERM2]) << 1) + 1) = ATTRIB;
                if(TERM2==curTerm) memcpy(video_memory, term2ptr, pageSIZE);
				term_x[TERM2]++;
				break;
			case TERM3:
				*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[TERM3] + term_x[TERM3]) << 1)) = buffer[i];
				*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[TERM3] + term_x[TERM3]) << 1) + 1) = ATTRIB;
                if(TERM3==curTerm) memcpy(video_memory, term3ptr, pageSIZE);
				term_x[TERM3]++;
				break;
			default:
				break;
		}
		if(term_x[working_Term] == NUM_COLS)
		{
			NewLine1:
			term_x[working_Term] = 0;
			newLineFunc(working_Term);
		}
        i++;
        countBytes++;
    }
	if(working_Term == curTerm)
		updateCursor(term_x[curTerm], term_y[curTerm]);

    return countBytes;
}


/*
*   int32_t term_open (const uint8_t* filename)
*   Inputs: name of file to open
*   Return Value: returns file descriptor # of opened file. 
*                   This value is always the smallest integer 
*                   greater than zero that is still available.
*
*                   OTHERWISE, return -1 on non-existant file 
*                   or no descriptor to free
*   Function: provides access to the file system. Can use this
*               function to obtain the file descriptor # used for
*               read/write operations (the above functions).
*/
int32_t term_open (const uint8_t* filename)
{
    init_keyboard();
    return SUCCESS_CODE;
}


/*
*   int32_t term_close (int32_t fd)
*   Inputs: file descriptor number to close
*   Return Value: 0 on success, -1 on invalid close
*   Function: closes specified file descriptor
*/
int32_t term_close (int32_t fd)
{
    return ERROR_CODE;
}



//***********SYSTEM CALLS FOR CHECKPOINT 3***********

/*
*   int32_t execute (const uint8_t* command)
*   Inputs: file name of the program to be executed
*   Return Value:  -1 if the command can't be executed
*                        (i.e) program doesn't exist/file not an executable
*                    256 if the commad dies by an exception
*                    [0, 255] if program executes a "halt" system call
*                        (then the return value is specified by call to "halt")
*   Function: attempts to load and execute a new program by hading the processor
*                to a new program until its termination (via exception/"halt" system call).
*
*              Kernel also needs to create a new virtual space for the process
*                via set up new page directory with entries (look at "Appendix C")                
*/
int32_t execute (const uint8_t* command)
{
	uint32_t i, j, entry_point;
	struct dir_entry e;
																// parse the commands for the file executable and the optional argument
	uint8_t buf[file_name_size];
	uint8_t arg[MAXBUFFERSIZE];
	uint8_t exe_check_buf[EXE_CHK_SIZE];						// size four because we only need to check for the ELF at the beginning of the file
	uint8_t entry_pt_buf[ENTRY_PT_SIZE];						// entry size is four bytes 24-27 in the elf header
    
  	struct PCB* cur_pcb_loc = (struct PCB*)(KERN_PG_BOT - 
    (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);	    // set the current PCB using virtual memory to the top of the current kernel stack segment
																			// bottom of the kernel page is at 8MB and each kernel stack segment is 8KB, so we subtract
																			// 8KB for each process to get to the correct pcb address
																			// divide by 4 because these pointers are 4 bytes and we want to subtract bytes

	*cur_pcb_loc = *current_PCB;	        // copy over current_PCB data into this address (top of KSS)
	current_PCB = cur_pcb_loc;		    // set this address equal to the current PBC address

	if(initial_pcb[working_Term] <= 0)				// the initial PCB for shell is created outside of execute, so we set a flag to skip the PCB creation for the initial run
	{
		term_processes[working_Term]++;				// new process, so add to the total processes and make a new PCB for the new process
        process_num = term_processes[TERM1] + term_processes[TERM2] + term_processes[TERM3];
		if(process_num >= MAX_PROCESSES) // check to see if max processes are already running; subtract one because this is zero indexed
		{
			term_processes[working_Term]--;
			return ERROR_CODE;
		}
		struct PCB new_pcb;		
		init_pcb(&new_pcb);
		new_pcb.PID = current_PCB;	// set parent of the new PCB to the current program calling this new program
		current_PCB = &new_pcb;
		
		struct PCB* new_pcb_loc = (struct PCB*)(KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);
		*new_pcb_loc = *current_PCB;	// process control block for the shell
		current_PCB = new_pcb_loc;
	}
	
	current_PCB->term = working_Term;
    initial_pcb[working_Term]--;				// initial pass is over, create new pcb here on out

    for(i = 0; i < file_name_size; i++)		// clear buffers to avoid potential errors
        buf[i] = 0;

    for(i = 0; i < EXE_CHK_SIZE; i++)
		exe_check_buf[i] = 0;
	
	
	for(i = 0; i < MAXBUFFERSIZE; i++)
		arg[i] = 0;
	
	
    for(i = 0; i < ENTRY_PT_SIZE; i++)
        entry_pt_buf[i] = 0;
	
	i = 0;
    j = 0;
    entry_point = 0;
	
    while(command[i] == ' ')                                    // strip leading spaces
           i++;
	
	while(command[i] != '\0' && command[i] != ' ')      		// file name is separated by a space from the argument
	{                                                           // or it ends with null character since argument is optional
        if(i == file_name_size)
        {
			current_PCB = cur_pcb_loc;
			term_processes[working_Term]--;											// process failed to create so make sure to take one away from total processes
            return ERROR_CODE;
        }
		buf[j] = command[i];
        i++;
        j++;
	}
	
	while(command[i] == ' ')                                    // strip leading spaces
        i++;

	j = 0;
	while(command[i] != '\0' && command[i] != ' ')
	{
		arg[j] = command[i];
		current_PCB->command[j] = command[i];
		j++;
		i++;
	}
	
	int32_t fd = file_open(buf);                              
	int32_t read_error_check = read_dentry_by_name(buf, &e);	// retrieve the information for the exe file & error check
	if(read_error_check == ERROR_CODE)
	{
		file_close(fd);											// on any errors, close the opened fd, set the current PCB back to the calling program 
		current_PCB = cur_pcb_loc;
		term_processes[working_Term]--;											// process failed to create so make sure to take one away from total processes
		return ERROR_CODE;
	}

	current_PCB->file_desc[fd].file_position = 0;				// make sure that the file position is the the beginning because we will now check for the ELF

	int32_t read_error_check2 = read_data(e.inode_num, 0, exe_check_buf, EXE_CHK_SIZE);
	if(read_error_check2 == ERROR_CODE)
	{
		current_PCB->file_desc[fd].file_position = 0;			// along with the other standard error operations, set the file position to zero for the next time we open it
		current_PCB = cur_pcb_loc;
		term_processes[working_Term]--;
		file_close(fd);
		return ERROR_CODE;
	}
	
	// these numbers are used only once
	if(exe_check_buf[0] != 0x7f || exe_check_buf[1] != 0x45 || exe_check_buf[2] != 0x4c || exe_check_buf[3] != 0x46)	// check read info against what it should have as an exe "ELF"
	{
		current_PCB->file_desc[fd].file_position = 0;
		current_PCB = cur_pcb_loc;
		term_processes[working_Term]--;
		file_close(fd);
		return ERROR_CODE;
	}
	
	current_PCB->file_desc[fd].file_position = ENTRY_PT_BEGIN;		// confirmed exe file, so set file in position to read the entry data
	
	// read the four bytes of entry data
	int32_t read_error_check3 = read_data(e.inode_num, 0, entry_pt_buf, ENTRY_PT_SIZE);
	if(read_error_check3 == ERROR_CODE)
	{
		current_PCB->file_desc[fd].file_position = 0;
		current_PCB = cur_pcb_loc;
		term_processes[working_Term]--;
		file_close(fd);
		return ERROR_CODE;
	}
	
	// little endian so need to reverse the order, but take entry point address and set it to the entry point var
	for(i = 1; i <= ENTRY_PT_SIZE; i++)
	{
		entry_point += (uint32_t)entry_pt_buf[ENTRY_PT_SIZE - i];
        if(i != 4)
		  entry_point <<= BYTE_SIZE;
	}
	
	// just read some data, so reset file position for copying
	current_PCB->file_desc[fd].file_position = 0;
   
   	uint32_t* inode_ptr = (uint32_t*)((uint8_t*)bb + ((e.inode_num + 1) * BLOCK_SIZE)); // -calculate pointer to the inode using the inode index; add 1 because index 0 inode is not the boot block
	
	insert_4MBPage(VIRT_PRGM_ADDR, USERTASK_PDE_SETTINGS, term_processes[working_Term] + working_Term*MAXPROGPTERM + 2);				
                                                    // set up paging for this task; all tasks will share virtual address 128MB & are set as user level; 
													// the process_num tells the function how far down in physical memory to map the page to
													// the last parameter is specifying where in physical memory to map the page, so the 2 comes from the fact
												    // that 2 4MB sections have already been allocated
    
	// read the entire file into the newly created task page as to copy it into user program space
	int32_t read_error_check4 = read_data(e.inode_num, 0, (uint8_t*) VIRT_PRGM_EXE_ADDR, *inode_ptr);
    if(read_error_check4 == ERROR_CODE)
    {
        current_PCB->file_desc[fd].file_position = 0;
		current_PCB = cur_pcb_loc;
		term_processes[working_Term]--;
        file_close(fd);	
        insert_4MBPage(VIRT_PRGM_ADDR, USERTASK_PDE_SETTINGS, term_processes[working_Term] + working_Term*MAXPROGPTERM + 2);		
        return ERROR_CODE;
    }
	
	if(working_Term == TERM1)
        recentPCB1 = current_PCB;
    else if(working_Term == TERM2)
        recentPCB2 = current_PCB;
    else if(working_Term == TERM3)
        recentPCB3 = current_PCB;
	
	file_close(fd);															// close the file because we are now done with it	

	tss.ss0 = KERNEL_DS;													// switch the stack segment to the kernel_ds (never really changes)
    tss.esp0 = KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4);              	// place kernel stack pointer at the bottom of the kernel stack segement for that individual task
																			// need to divide by four because esp0 is a 4 byte variable, and we want to subtract bytes, not 4 bytes at a time

	current_PCB->currentESP = getESP();			// set the esp and bsp for the current PCB for the child program to reference back to
    current_PCB->currentEBP = getEBP();

    user_mode(entry_point, 0x083FFFFC); 		// call assembly function to put on context for program, then iret to user space
	
	asm volatile(			// execute finishes here once halt is finished
	"halt_ret:;"
    "leave;"
    "ret;"	
	);

    return ERROR_CODE;
}

/*
*   int32_t halt (uint8_t status)
*   Inputs: 8-bit arg from "BL" (lower 8 bits from %EBX)
*   Return Value: 8-bit value to the parent process.
                    DO NOT USE ALL 32-BITS OF %EBX
*   Function: terminates current process, return to parent process
*/
int32_t halt (uint8_t status)
{
    struct PCB* pcb_loc = (struct PCB*)(KERN_PG_BOT - (term_processes[working_Term] + working_Term*MAXPROGPTERM+1)*KERN_SEG_SIZE);	// gather the address of the current PCB that is sitting 8KB above the stack pointer
	current_PCB = pcb_loc;																	// add 1 because the zeroth program will need to have a PCB above its stack

    int safe_num;	
	int i;
	for(i = 2; i < NUM_FILES; i++)	// start at 2 because we do not close the STDIN or STDOUT
	{
		file_close(i);				// close all files opened in PCB because we are terminating the program
	}
	
	curEsp = current_PCB->currentESP;	// set the ebp and esp registers with current pcb's values for when we jump back to execute
	curEbp = current_PCB->currentEBP;	// could be wrong
	
	if(current_PCB->PID != NULL)			// if current PCB is the initial PCB, dont set it to its parent (it has none)
    {
		current_PCB = current_PCB->PID;
        if(working_Term == TERM1)
            recentPCB1 = current_PCB;
        else if(working_Term == TERM2)
            recentPCB2 = current_PCB;
        else if(working_Term == TERM3)
            recentPCB3 = current_PCB;
    }


	asm volatile(			// inline assembly to push values to registers to set up stack
	"movl curEsp, %esp;"
	"movl curEbp, %ebp;"
	);
	
	term_processes[working_Term]--;										// subtract one from the current total processes because we are terminating a process
	safe_num = term_processes[working_Term];							// i want process_num to be negative, however, i need a zero/positive value for paging
	if(term_processes[working_Term] == ORIG_PROCESS)	// so i set safe_num (for paging) to process_num and make sure it isn't negative
		safe_num = 0;									// process_num of -1 signifies that we are attempting to close the original shell
	
	insert_4MBPage(VIRT_PRGM_ADDR, USERTASK_PDE_SETTINGS, safe_num + working_Term*MAXPROGPTERM + 2);	// fix paging and kernel stack for the parent process we are going back to
	
	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERN_PG_BOT - (safe_num + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 
	//tss.ebp = KERN_PG_BOT - (safe_num + working_Term*MAXPROGPTERM)*((KERN_SEG_SIZE)/4); 

	if(term_processes[working_Term] == ORIG_PROCESS)	// if we try to close the original shell, reset the values that the initial shell needs
	{
		initial_pcb[working_Term] = 1;	// needed to signal the original shell so that execute doesnt create another pcb
		term_processes[working_Term] = 0;	// processes will be equal to zero, (don't count the original shell)
		execute((uint8_t*)"shell");	// call execute shell so that the original shell can't be closed 
	}
	
	
    setEBX(status);    // move the status value that is in %bl into %eax; this will be execute's return value

	asm volatile(
	"jmp halt_ret;"		// return to execute
	);
	
	return ERROR_CODE; // will never get here but return error if it does
}


/*
*   int32_t getargs (uint8_t* buf, int32_t nbytes)
*   Inputs: buf = buffer to hold arguments
            nbytes = size of arguments stored in buffer
*   Return Value: return -1 if the the arguments and the terminal NULL byte
                     doesn't fit into the given buffer
                  return 0 on success otherwise
*   Function: grabs programs arugments (from PCB) and stores it into
                the buffer "buf". Also stores total size of arguments 
                in bytes
*/
int32_t getargs (uint8_t* buf, int32_t nbytes)
{
    if(buf == NULL)
    {
        return ERROR_CODE;
    }
	buf = (uint8_t*)strcpy((int8_t*)buf, (int8_t*)current_PCB->command);
	return SUCCESS_CODE;
}


/*
*   int32_t vidmap (uint8_t** screen_start)
*   Inputs: location to write video-memory. This is the pointer to the
*               page to be allocated in user-space
*   Return Value: return -1 on invalid location
*                  return 0 on success
*   Function: maps the text-mode video memory (already predefined in phyiscal memory) 
*                into user space (onto a newly allocated page)
*/
int32_t vidmap (uint8_t** screen_start)
{
    if(screen_start < (uint8_t**)VIRT_PRGM_ADDR || screen_start > (uint8_t**)VIRT_PRGM_END)   //Determines if the passed in pointer is from user space
        return ERROR_CODE;
    //insert_PageTable(0x0F);                 //Sets U/S bit, R/W bit, present bit, and pwt bit
    switch(curTerm)
    {
        case TERM1:
            *screen_start = term1ptr;
            break;
        case TERM2:
            *screen_start = term2ptr;
            break;
        case TERM3:
            *screen_start = term3ptr;
            break;
        default:
            *screen_start = term1ptr;
            break;
    }
    //*screen_start = (uint8_t*)0x00800000;             //The virtual address corresponding to 2nd page directory entry which points to video memory
    return SUCCESS_CODE;            
}



/*
*   int32_t read (int32_t fd, void* buf, int32_t nbytes)
*   Inputs: fd: the file descriptor that needs to be read
*           buf: This can represent the file to read, or a buffer for term read to copy its contents into
*           nbytes: The number of bytes to read from a file or from the keyboard buffer
*   Return Value: return -1 on invalid value
*                  return 0 on success
*   Function: This function takes in a file descriptor, buf pointer, and number of bytes to read. Passes the 
*             parameters to the correct function determined by the function operation table in the current PCB.
*/
int32_t read (int32_t fd, void* buf, int32_t nbytes)
{
    if(fd == STDOUT_FD || fd < MINFD || fd > MAXFD || buf == NULL)
    {
        return ERROR_CODE;
    }
    if(current_PCB->file_desc[fd].flags & FD_USAGE)
    {
        int32_t ret = (*current_PCB->file_desc[fd].fop_table_pointer.read)(fd, buf, nbytes);
		return ret;
    }
    return ERROR_CODE;
}

/*
*   int32_t write (int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd: the file descriptor that needs to be written to 
*           buf: This can represent the file to write, or a buffer for term write to copy its contents into
*           nbytes: The number of bytes to write from a file or from the keyboard buffer
*   Return Value: return -1 on invalid value
*                  return 0 on success
*   Function: This function takes in a file descriptor, buf pointer, and number of bytes to write. Passes the 
*             parameters to the correct function determined by the function operation table in the current PCB.
*/
int32_t write (int32_t fd, const void* buf, int32_t nbytes)
{
    if(fd <= MINFD || fd > MAXFD || buf == NULL)
    {
        return ERROR_CODE;
    }
    
    if(current_PCB->file_desc[fd].flags & FD_USAGE)
    {
        int32_t ret = (*current_PCB->file_desc[fd].fop_table_pointer.write)(fd, buf, nbytes);
        return ret;
    }
    return ERROR_CODE;
}

/*
*   int32_t open (const uint8_t* filename)
*   Inputs: filename: The file name to be opened
*   Return Value: return a File descriptor
*                  return -1 on failure
*   Function: Opens a file.
*/
int32_t open (const uint8_t* filename)
{
    if(filename == NULL || (*filename == '\0'))
        return ERROR_CODE;
    int32_t ret = file_open(filename);
    return ret;
}

/*
*   int32_t close (int32_t fd)
*   Inputs: fd: File descriptor to be closed.
*   Return Value: return -1 on invalid value
*                  return 0 on success
*   Function: Closes a file.
*/
int32_t close (int32_t fd)
{
    if(current_PCB->file_desc[fd].flags & FD_USAGE)
    {
        int32_t ret = file_close(fd);
        return ret;
    }
    return ERROR_CODE;
}


