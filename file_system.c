#include "file_system.h"

/* int32_t read_dentry_by_name(const uint8_t* fname, struct dir_entry* dentry)
*	Purpose: Given a directory entry struct and a filename, this function locates
*            the file in the file directory and copies its name, inode number, and
*			 file type to the directory entry.
*	 Inputs: fname - file name in the file directory
* 			 dentry - directory entry struct to be written to 
*	 Return: 0 on copy success
* 			 -1 on copy failure
*/
int32_t read_dentry_by_name(const uint8_t* fname, struct dir_entry* dentry)
{
	int32_t i, j, pass;
	const uint8_t* temp_name;
	
	pass = 0; 																// -file name is not found initially
	
	for(i = 0; i < tot_num_files; i++)										// -iterate over all possible files in file system
	{
		j = 0;
		temp_name = fname;													// -use a temp variable for name because name is iterated over and needs to be reset each check
		while(bb->files[i].filename[j] == *temp_name)						// check to see if letters match
		{
			if(*temp_name == NULL || j == file_name_size - 1) 				// -assuming that a blank filename will not be passed in, so we can check here for a null character
			{																// to see if string is finished being read
				pass = 1;													// -alternatively, some filenames will be too long to have a null terminator, so check to make sure that
				break;														// we are not checking beyond the max file length
			}																// -if while loop has not broken at this point, file names must match thus set filename to found
			else
			{
				j++;														// -iterate to next character if current one matches
				temp_name++;
			}
		}
		
		if(pass == 1)														// -if the file names match, perform copy; else go to next file
		{
			for(j = 0; j < file_name_size; j++)
			{
				dentry->filename[j] = bb->files[i].filename[j];				// -copy name
			}
			dentry->file_type = bb->files[i].file_type;						// -copy file type
			dentry->inode_num = bb->files[i].inode_num;						// -copy inode num
			return SUCCESS_CODE;														
		}
	}
	
	return ERROR_CODE;						
}

/* int32_t read_dentry_by_index(uint32_t index, struct dir_entry* dentry)
*	Purpose: Given a directory entry struct and an index number, this function locates
*            the file in the file directory and copies its name, inode number, and
*			 file type to the directory entry.
*	 Inputs: index - index in the file directory
* 			 dentry - directory entry struct to be written to 
*	 Return: 0 on copy success
* 			 -1 on copy failure
*/
int32_t read_dentry_by_index(uint32_t index, struct dir_entry* dentry)
{
	int32_t i;
	if(index > bb->dir_entries - 1 || index < 0)
		return ERROR_CODE;
	
	for(i = 0; i < file_name_size; i++)
		dentry->filename[i] = bb->files[index].filename[i];
	dentry->file_type = bb->files[index].file_type;
	dentry->inode_num = bb->files[index].inode_num;
	
	return SUCCESS_CODE;
}

/* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
*	Purpose: Given the inode, an offset into the file, and a length, copy data equal 
*			 to the number of bytes given by length from a file specified by the inode 
* 			 into a passed in buffer.
*	 Inputs: inode - inode of the specified file you want to copy
* 			 offset - how many bytes into the file you want to begin copying from
*			 buf - a char buffer that is written to
* 			 length - how many bytes to read from the file
*	 Return: 0 if the end of file was reached
* 			 bytes_read - how many bytes were copied into the buffer if the EOF was not reached
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	int32_t i, db_num, offset_blks, new_offset, read_limit, fd, bytes_read;
	uint32_t* db_ptr;
	
	fd = ERROR_CODE;
	bytes_read = 0;								 																// -initialize bytes read to no bytes yet
	
	for(i = 0; i < PCB_MAX_FILES; i++)																			// -find the file descriptor for this file
	{								
		uint32_t* node = current_PCB->file_desc[i].inode;														// if the fd isn't found, file isn't open so return error
		if(node != NULL && *node == inode && current_PCB->file_desc[i].flags != (DIR_FLAG | USED_FLAG))			// this also confirms that the inode is valid
		{
			fd = i;																								// "." shares inode with frame0.txt so need to differentiate
			break;
		}
	}
	
	if(fd == ERROR_CODE)
		return ERROR_CODE;

	uint32_t* inode_ptr = (uint32_t*)((uint8_t*)bb + ((inode + 1) * BLOCK_SIZE)); 								// -calculate pointer to the inode using the inode index; add 1 because index 0 inode is not the boot block
	int32_t file_size = *inode_ptr;																				// -grab file length from inode structure																
	
	if(current_PCB->file_desc[fd].file_position >= file_size - 1 && current_PCB->file_desc[fd].flags != (DIR_FLAG | USED_FLAG))  // make sure we are not resetting a directory
	{
		current_PCB->file_desc[fd].file_position = 0;															// -reset file position for the file because it has been fully read
		return 0;																								// -end of file reached, return 0 to signify this
	}

	current_PCB->file_desc[fd].file_position += offset;															// -start at file position plus any offset

	if(length + current_PCB->file_desc[fd].file_position > file_size)											// -set read_limit to be the file size if the length is longer than the file size so we do not go out of bounds
		read_limit = file_size - current_PCB->file_desc[fd].file_position;
	else
		read_limit = length;
	
	offset_blks = current_PCB->file_desc[fd].file_position / BLOCK_SIZE;										// -calculate how many data blocks need to be skipped given the offset
	new_offset = current_PCB->file_desc[fd].file_position - (offset_blks * BLOCK_SIZE);							// -now subtract the data blocks from the offset to have new accurate offset
	inode_ptr += 1 + offset_blks;																				// -inode ptr starts at length so it needs incremented once to get to first data block
																												// -increment it more for the offset if needed
	if(*inode_ptr < 0 || *inode_ptr > bb->data_blocks - 1)														// -error check the data block by making sure it is in bounds
		return ERROR_CODE;
	
	db_num = *inode_ptr;																						// -extract the initial data block number from inode
	
	db_ptr = (uint32_t*)((uint8_t*)bb + (bb->inodes + 1) * BLOCK_SIZE + db_num * BLOCK_SIZE + new_offset);		// -fix datablock ptr to the correct data block
	
	for(i = 0; i < read_limit; i++)																				// -read only length bytes from file
	{
		*buf = *db_ptr;																							// -copy character into buffer and then increment bytes read
		bytes_read++;
		
		if((int32_t)db_ptr % BLOCK_SIZE == BLOCK_SIZE - 1)														// -see if the datablock ptr is on the boundary of another block
		{																										// and if it is, increment the inode ptr to get next block
			inode_ptr += 1; 
			if(*inode_ptr < 0 || *inode_ptr > bb->data_blocks - 1)
				return ERROR_CODE;
			
			db_num = *inode_ptr;
			db_ptr = (uint32_t*)((uint8_t*)bb + (bb->inodes + 1) * BLOCK_SIZE + db_num * BLOCK_SIZE);
			buf++;
		}
		else
		{
			buf++;																								// -if in same block, just increment to next byte in file
			db_ptr = (uint32_t*)((uint8_t*)db_ptr + 1);
		}
	}
	
	current_PCB->file_desc[fd].file_position += bytes_read;
	return bytes_read;	
}

/* int32_t read_directory(int32_t fd, uint8_t* buf)
*	Purpose: Copies into the buf the filename of the index specified.
*	 Inputs: buf - buffer to be copied to
*	 Return: length of the filename on success
* 	 		 SUCCESS_CODE if end of directory reached or error occurred upon reading dir entry
*/
int32_t read_directory(int32_t fd, uint8_t* buf)
{
	struct dir_entry e;										// dummy struct to be extorted
	
	int success = read_dentry_by_index(current_PCB->file_desc[fd].file_position, &e);		// get filename

	if(success == SUCCESS_CODE)
	{				
		current_PCB->file_desc[fd].file_position += 1;					// ready to read next entry
		buf = (uint8_t*) strcpy((int8_t*)buf, (int8_t*)e.filename);		// copy over the filename into buffer
		return strlen((int8_t*)buf); 											// return length of the buffer
	}

	current_PCB->file_desc[fd].file_position = 0; // reset the file position as every file entry has been read
	return SUCCESS_CODE;
	
}

/* int32_t test_file_read()
*	Purpose: Helper function that asks for a filename, reads the file, then prints it to the terminal.
*	 Inputs: none
*	 Return: 0 for success
* 			 -1 for an error
*/
int32_t my_file_read(uint32_t offset, uint32_t length)
{
	clear();
	uint8_t filename_buffer[file_name_size];	
	myprint("Enter the name of the file you wish to read followed by the ENTER key: \n");
	int32_t error_check4 = term_read(0, filename_buffer, file_name_size);				// -gather desired filename from user
	if(error_check4 == ERROR_CODE)
		return ERROR_CODE;
	struct dir_entry e;																	// -dummy struct to be extorted
	int32_t error_check1 = read_dentry_by_name(filename_buffer, &e);					// -gather information for file
	if(error_check1 == ERROR_CODE)
		return ERROR_CODE;
	
	uint8_t buffer[PRACTICAL_SIZE];			

	int32_t bytes_read = read_data(e.inode_num, offset, buffer, length);				// read file into buffer with no offset and full file size
	if(bytes_read == ERROR_CODE)
		return ERROR_CODE;

	while(bytes_read)
	{
		int32_t error_check3 = term_write(0, buffer, length);							// write file to terminal
		if(error_check3 == ERROR_CODE)
			return ERROR_CODE;

		bytes_read = read_data(e.inode_num, 0, buffer, length);					// read file into buffer with no offset and full file size
		if(bytes_read == ERROR_CODE)
			return ERROR_CODE;

	}

	return SUCCESS_CODE;
}

/* int32_t test_file_size()
*	Purpose: Helper function that asks for a filename and then prints the file's size.
*	 Inputs: none
*	 Return: none	 
*/
int32_t test_file_size()
{
	clear();
	uint8_t filename_buffer[file_name_size];	
	
	myprint("Enter name of file whose size you wish to know followed by the ENTER key: ");
	
	int32_t error_check4 = term_read(0, filename_buffer, file_name_size);				// -gather desired filename from user & error check
	if(error_check4 == ERROR_CODE)
		return ERROR_CODE;

	struct dir_entry e;																	// -dummy struct to be extorted
	int32_t error_check1 = read_dentry_by_name(filename_buffer, &e);					// -gather information for file & error check
	if(error_check1 == ERROR_CODE)
		return ERROR_CODE;
	
	uint32_t* inode_ptr = (uint32_t*)((uint8_t*)bb + ((e.inode_num + 1) * BLOCK_SIZE)); // -calculate pointer to the inode using the inode index; add 1 because index 0 inode is not the boot block
	int32_t file_size = *inode_ptr;	
	
	printf("File size: %d\n", file_size); 												// print file size to terminal
	
	return SUCCESS_CODE;
	
}