#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "types.h"
#include "system_calls.h"
#include "pcb.h"

#define bb_reserve_blks 13						// reserved space for boot block
#define de_reserve_blks 6 						// reserved space for each directory entry
#define file_name_size 32						// max size of filename
#define tot_num_files 64						// max files that can fit in file directory including boot block
#define ERROR_CODE -1
#define SUCCESS_CODE 0
#define BLOCK_SIZE 4096 						// each "absolute" block in file system is 4KB in size

#define MAX_FILE_SIZE 4190208					// -an inode is 4KB, each data block descriptor being 4 bytes
												// so (2^12 / 4) - 4 data block descriptors because the first 
												// 4 bytes is the file length; each data block holds 2^12 bytes
												// so multiply by that to find total possible file length
												
#define PRACTICAL_SIZE 40000 					// none of our files are this big but the above value seems too large to make array

#define	MAX_OFFSET_CHARS 7	 					// offset cannot be more than 4177920 which is 7 ascii characters
#define PCB_MAX_FILES 8
#define DIR_DONE	2 							// directory has fully been read	
#define DIR_FLAG 2
#define USED_FLAG 1	

struct dir_entry	// directory entry struct aka file
{
	uint8_t filename[file_name_size];		// each file has a name, type, associated inode number, and reserved space
	uint32_t file_type;
	uint32_t inode_num;
	uint32_t de_reserve[de_reserve_blks];
};

struct boot_block 	// boot block struct
{
	uint32_t dir_entries;	// the boot block is a stats block for the file directory that has total inodes, data blocks, reserved space, and total files
	uint32_t inodes;
	uint32_t data_blocks;
	uint32_t bb_reserve[bb_reserve_blks];
	struct dir_entry files[tot_num_files - 1];   // file stats takes one file block
};
struct boot_block* bb;	// global boot block pointer that is set to the boot module in kernel.c; serves to extract data in an organized manner

// file system functions
int32_t read_dentry_by_name(const uint8_t* fname, struct dir_entry* dentry);
int32_t read_dentry_by_index(uint32_t index, struct dir_entry* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// test functions
int32_t read_directory(int32_t fd, uint8_t* buf);
int32_t my_file_read(uint32_t offset, uint32_t length);
int32_t test_file_size();

#endif