#include "paging.h"

static uint32_t page_directory[PDE] __attribute__((aligned(PAGE_SIZE))); // global page directory table
static uint32_t kernel_page_table __attribute__((aligned(PAGE_SIZE))); // PDE for the kernel
static uint32_t video_page_table[PTE] __attribute__((aligned(PAGE_SIZE))); // PDE for the empty space ahead of the kernel in memory (sans the video mem)
static uint32_t user_video_table[PTE] __attribute__((aligned(PAGE_SIZE)));


/*
* void init_paging();
*   Inputs: none
*   Return Value: void
*	Function: Initializes paging by creating a page directory, and inserting PDE's for the video memory,
*			  and the 4MB kernel page. Then loads correct values in Control Registers for paging such as CR3,
*			  CR4, and CR0.
*/
void init_paging()
{
	init_PDT();
	init_PT();
	page_directory[0] = ((unsigned int) video_page_table) | 3;	// make the PDE present and r/w enabled
	page_directory[1] = ((unsigned int) kernel_page_table);
	Load_PDBR(page_directory);
	Do_Paging();
}

/*
* void init_PDT();
*   Inputs: none
*   Return Value: void
*	Function: Initializes Page Directory Table, initially everything is set to r/w.
*/
void init_PDT()
{
	int i;
	for(i = 0; i < PDE; i++)
	{
		page_directory[i] = 0x00000002; // enable r/w for this entry, but nothing more
	}
}

/*
* void init_PT();
*   Inputs: none
*   Return Value: void
*	Function: Initializes Page Table for video memory, only the video page is set to present.
*/
void init_PT()
{
	kernel_page_table = KERN_ADDR | 0x183;		// sets the PDE to present, r/w enabled, 4MB in
												// page size, and global
												
	int i;
	for(i = 0; i < PTE; i++)
	{
		if(i == VID_PAGE)
			video_page_table[i] = (i * PAGE_SIZE) | 3;		// make only this entry present and r/w enabled
		else
			video_page_table[i] = (i * PAGE_SIZE) | 2;		// rest of entries are not present but r/w enabled
	}
}

/*
* void insert_4MBPage(unsigned int address, unsigned int flags, unsigned int process);
*   Inputs: address: Linear address to be decomposed into a physical address.
*			flags:	 The page's flags that need to be set
*			process: Process number, this is needed in order to index to a different phyiscal address
*   Return Value: void
*	Function: Takes in a linear address then sets up a 4MB page determined by the process number.
*/
void insert_4MBPage(unsigned int address, unsigned int flags, unsigned int process)
{
	unsigned int index = address >> PAGEOFFSET;
	page_directory[index] = (process << PAGEOFFSET) | flags;
	Load_PDBR(page_directory);									//Flushes the TLB by rewriting to cr3
}

/*
* void insert_PageTable(unsigned int flags);
*   Inputs: flags: The page's flags that need to be set.
*   Return Value: void
*	Function: Inserts a PageTable for the terminal video memory buffers, and the for the function vidmap.
*/
void insert_PageTable(unsigned int flags)
{
	int i;
	for(i = 0; i < PTE; i++)
	{
		if(i == 0)
			user_video_table[i] = VIDEO | flags;		// make only this entry present and r/w enabled
		else if(i == 1)
			user_video_table[i] = SHELL1VID | flags;	//term1
		else if(i == 2)
			user_video_table[i] = SHELL2VID | flags;	//term2
		else if(i == 3)
			user_video_table[i] = SHELL3VID | flags;	//term3
		else
			user_video_table[i] = NULL | 2;				// rest of entries are not present but r/w enabled
	}
	page_directory[2] = ((unsigned int) user_video_table) | flags;
	Load_PDBR(page_directory);									//Flushes the TLB by rewriting to cr3
}