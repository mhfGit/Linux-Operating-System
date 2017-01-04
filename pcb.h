#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "system_calls.h"


#define NUM_FILES 8 // max number of open files per task
#define STDIN_FD 0  // default file descriptor for stdin
#define STDOUT_FD 1 // default file descriptor for stdout

#define ERROR_CODE -1
#define SUCCESS_CODE 0
#define MAXBUFFERSIZE 128
#define FD_USAGE			0x00000001
#define NOT_USED			0
#define IS_USED 			1
#define FILE_START 			0
#define MAXPROCESSES		6
#define MAX_ARGS			3 				// max of three arguments can be passed into the command line

/*	FLAGS
31									  0
0000 0000 0000 0000 0000 0000 0000 0000

bit 0 -> 1 - file descriptor in use
		 0 - file descriptor not in use

*/

// the informational blocks inside of the file descriptor array
struct fop_table
{
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*open)(const uint8_t* filename);
	int32_t (*close)(int32_t fd);
};

struct file_descriptor
{
	struct fop_table fop_table_pointer;		//	pointer to the file operations table
	uint32_t* inode;					//	inode number corresponding to the file
	int32_t file_position;				//	current file position in the file so the place is not lost
	uint32_t flags;						//	marks file descriptor as in use among other things
};

// the process control block which consists of NUM_FILES info blocks
struct PCB
{
	struct file_descriptor file_desc[NUM_FILES];		// a file descriptor per open file
	struct PCB* PID;									// pointer to the parent PCB
	uint32_t currentESP;
	uint32_t currentEBP;
	uint32_t kernelESP;
	uint32_t kernelEBP;
	uint32_t term;
	uint8_t command[MAXBUFFERSIZE];
};

struct PCB* current_PCB;

struct fop_table f_table;
struct fop_table t_table;
struct fop_table r_table;
void init_pcb(struct PCB* pcb);
void init_ttable();
void init_ftable();
void init_rtable();

#endif