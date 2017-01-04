#include "devices.h"

//defines for KEYBOARD
#define STATUSPORT 			0x64
#define DATAPORT_KEYBRD		0x60
#define BREAKKEYFLAG		128
#define NUMKEYBOARDENTRY 	90

#define VOIDVAL				0xFF
#define CURSORPORT			0x3D4
#define CURSORDATA			0x3D5

//defines for RTC
#define PORTVAL_RTC			0x70
#define DATAPORT_RTC		0x71
#define PORTVALREGB			0x8B
#define PORTVALREGA			0x8A
//defines for the flags
#define MAKESHIFTL 			42
#define BREAKSHIFTL			170
#define MAKESHIFTR			54
#define BREAKSHIFTR			182
#define MAKECTRL			29
#define BREAKCTRL			157
#define BREAKCAPS			186
#define MAKEKEYL			38
#define MAKEALT				56
#define BREAKALT			184
#define FLAGSET				1
#define NOFLAGSET			0
//Values for swapping non alphabet keys with their shift representation. Ex: 1 --> !
#define ASCIIZERO			48
#define ASCIIONE			49
#define ASCIITWO			50
#define ASCIITHREE			51
#define ASCIIFOUR			52
#define ASCIIFIVE			53
#define ASCIISIX			54
#define ASCIISEVEN			55		
#define ASCIIEIGHT			56			
#define ASCIININE			57
#define GRAVEACCENT			96
#define HYPHEN				45
#define EQUALSSIGN			61
#define LBRACKET			91
#define BLASH				92
#define RBRACKET			93
#define SEMICOLON			59
#define SINGLEQUOTE			39
#define COMMA				44
#define PERIOD				46
#define FLASH				47
#define RPARANTH 			41
#define EXCLPOINT 			33
#define ATSYMB				64
#define POUND 				35
#define MONEY 				36
#define PERCENT 			37
#define CARROT 				94
#define AMPERSAND 			38
#define MULT				42
#define LPARANTH 			40
#define TILDE 				126
#define UNDERSCORE 			95
#define PLUS 				43
#define LDRAGON 			123
#define PARALLEL 			124
#define RDRAGON 			125
#define COLON 				58
#define DBLQUOTE 			34
#define LANGLE 				60
#define RANGLE 				62
#define QUESTION 			63
#define LOWALPHA			97
#define HIGHALPHA			122
#define ALPHAOFFSET			32
#define ENTERKEY			10
//Values for checkpoint 5, extending to multiple terminals
#define MAXTERMS			3
#define MAKEF1				59
#define MAKEF2				60
#define MAKEF3				61
#define TERM1				0
#define TERM2				1
#define TERM3				2
#define pageSIZE			4096
#define KERN_PG_BOT 		0x00800000
#define KERN_SEG_SIZE		0x00002000
#define MAXPROGPTERM        4
#define PIT_MAX_HZ 			100
#define PIT_MIN_HZ			20
//defines for PIT
#define CHANPORT0			0x40
#define CHANPORT1			0x41
#define CHANPORT2			0x42
#define COMMANDPORT			0x43
#define CMD_PORT_VAL 		0x36
#define MAGIC_PIT			11931
#define PIT_IRQ				0
#define TERMCONST			0x1000

extern int term_processes[MAXTERMS];
extern int initial_pcb[MAXTERMS];
int term_x[MAXTERMS];
int term_y[MAXTERMS];
char* video_memory = (char *)VIDEO;

uint8_t* term1ptr = (uint8_t*)0x00801000;
uint8_t* term2ptr = (uint8_t*)0x00802000;
uint8_t* term3ptr = (uint8_t*)0x00803000;
extern struct PCB* recentPCB1;
extern struct PCB* recentPCB2;
extern struct PCB* recentPCB3;
extern int process_num;

int ctrl = 0;
int capsflag = 0;
int shift_flag = 0;
int alt = 0;
int curTerm = 0;
int termIndex[MAXTERMS];
volatile int enter_flag[MAXTERMS] = {0,0,0};
volatile int rtcFlag[MAXTERMS] = {1, 1 ,1};
int setFlags(unsigned char keypress);
int changeToAlphaNumeric(int keypress);
void clearKeybrdBuffer(int);
static uint8_t sCode[NUMKEYBOARDENTRY] = {
	VOIDVAL, VOIDVAL, 
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 	//2-13
	'\b', VOIDVAL,															//14 backspace = '\b'
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',		//16-27
	ENTERKEY, VOIDVAL, 															//28 enter = 10 = '\n'
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 		//30-40
	'`', VOIDVAL, '\\', 
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',               //44-53
	VOIDVAL, VOIDVAL, VOIDVAL, ' ', VOIDVAL, VOIDVAL, VOIDVAL,		//54-60
	VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL,	//61-70
	VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL,	//71-80`
	VOIDVAL, VOIDVAL, VOIDVAL,										//83 
	 VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL, VOIDVAL 			//81-89
};


unsigned char keyboardBuffer[MAXTERMS][MAXBUFFERSIZE];
unsigned char termReadBuffer[MAXBUFFERSIZE];

/*
* void init_keyboard(void)
*   Inputs: void
*   Return Value: void
*	Function: Initializes keyboard, sets the cursor to the top left corner, and clears
*			  the screen and keyboard buffer.
*/
void init_keyboard(void)	
{
	curTerm = TERM1;							//current Term is 1
	clear();								//Clear the screen
	clearKeybrdBuffer(TERM1);					//clear the keyboard
	clearKeybrdBuffer(TERM2);					//clear the keyboard
	clearKeybrdBuffer(TERM3);					//clear the keyboard
	term_y[TERM1] = 0;
	term_y[TERM2] = 0;
	term_y[TERM3] = 0;
	updateCursor(0,0);						//Place cursor in top left corner
	enable_irq(1);							//1 represents the IRQ line to be enabled in the pic
}

/*
* void init_rtc(void)
*   Inputs: void
*   Return Value: void
*	Function: enables real time clock
*/
void init_rtc(void)
{
	char prev;

	outb(PORTVALREGB,PORTVAL_RTC);					//0x8B represents masking NMI and selecting register B
	prev = inb(DATAPORT_RTC);						//store values of register B
	outb(PORTVALREGB,PORTVAL_RTC);			
	outb(prev | 0x40, DATAPORT_RTC);				//set bit 6 to a 1 

	outb(PORTVALREGA,PORTVAL_RTC);					//0x8A represents masking NMI and selecting register A
	prev = inb(DATAPORT_RTC);
	outb(PORTVALREGA,PORTVAL_RTC);
	outb((prev & 0xF0) | 15, DATAPORT_RTC);			//0xF0 clears lower 4 bits and replaces it with 15, the value of the rate

	enable_irq(8);									//8 represents the IRQ line to be enabled in the pic
}

/*
* void rtc_redo(void)
*   Inputs: void
*   Return Value: void
*	Function: Outputs register C just so another interrupt can happen from the RTC
*/
void rtc_redo(void)
{
	rtcFlag[TERM1] = 0;
	rtcFlag[TERM2] = 0;
	rtcFlag[TERM3] = 0;
	outb(0x0C,PORTVAL_RTC);					//0x0C represents register C in RTC
	inb(DATAPORT_RTC);
}

/*
* void getChar(void)
*   Inputs: void
*   Return Value: void
*	Function: If the keyboard buffer is full, then we read the data from the specified
*			  data port to obtain the key code that will be turned into a character and 
*			  output to the keyboard buffer.
*/
unsigned char getChar(void)
{
	unsigned char press = inb(STATUSPORT);
	if(press & 1)										//checks to see if we can read from dataport
	{
		press = inb(DATAPORT_KEYBRD);

		if(setFlags(press))								//If any flags were set don't print that character
		{
			return VOIDVAL;
		}
		if((press & BREAKKEYFLAG) == 0)					//Don't allow the release keycodes to print
		{
			unsigned char newPress = sCode[(int)press];	//grab the correct ascii represenation
			int oneFlagCheck = shift_flag + capsflag;	//This checks to see if caps is on and shift is pressed
														//the reason for this is if both caps and shift are on
														//the resulting characters should be lowercase
			if(newPress >= LOWALPHA && newPress <= HIGHALPHA && (oneFlagCheck == 1))	//change lowercase to uppercase
			{																
				return newPress - ALPHAOFFSET;
			}
			int calcRange = newPress >= COMMA && newPress <= ASCIININE;		//calcRange checks to see if the
			calcRange += (newPress >= LBRACKET && newPress <= RBRACKET);	//button pressed is one of the
																			//strange non alphabet keys
			calcRange += newPress == SEMICOLON || newPress == SINGLEQUOTE;
			calcRange += newPress == EQUALSSIGN || newPress == GRAVEACCENT;
			if(calcRange && shift_flag )
			{
																			//addresses the non alphabet keys
				return changeToAlphaNumeric(newPress);						//to the correct shift representation
			}				
			return newPress;							//Outputs character normally if no flags were set
		}	
	}
	return VOIDVAL;
}

/*
* void updateBuffer(unsigned char keypress)
*   Inputs: unsigned char keypress - the most recent key that was pressed, in ASCII format.
*   Return Value: void
*	Function: This function determines how to update the keyboard buffer from the input. If there is a 
*			  backspace, then it will remove the previous key pressed, an enter will print current buffer
*			  then clear the entire buffer to prepare for next user input.
*			  Other than these two, any standard key press will be stored into the buffer given that
*			  the buffer isn't full, 126 entries- 127 is saved for the newline character.
*/
void updateBuffer(unsigned char keypress)
{
	
	if(keypress == VOIDVAL)							//A 0 keypress represents nothing important from the function
	{											//getChar was returned
		return;
	}
	if(keypress == '\b')							//A '\b' keypress represents a backspace
	{
		if(termIndex[curTerm] > 0)
		{
			termIndex[curTerm]--;
			term_x[curTerm]--;
			switch(curTerm)					//put a space in place of whatever was deleted according to the correct curTerm
			{
				case TERM1:
					*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1)) = ' ';
					*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1) + 1) = ATTRIB;
					break;
				case TERM2:
					*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1)) = ' ';
					*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1) + 1) = ATTRIB;
					break;
				case TERM3:
					*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1)) = ' ';
					*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1) + 1) = ATTRIB;
					break;
				default:
					break;
			}
			if(term_x[curTerm] < 0)
			{
				term_x[curTerm] = NUM_COLS - 1;
				term_y[curTerm]--;
			}
			updateCursor(term_x[curTerm], term_y[curTerm]);
		}
		return;	
	}
	if(keypress == ENTERKEY)							//A 10 keypress represents an enter key
	{
		int i = 0;
		while(i < termIndex[curTerm])
		{
			termReadBuffer[i] = keyboardBuffer[curTerm][i];
			i++;
		}
		termReadBuffer[termIndex[curTerm]] = ENTERKEY;
		enter_flag[curTerm] = 1;		
		clearKeybrdBuffer(curTerm);					//clear the keyboard buffer
		newLineFunc(curTerm);
		updateCursor(term_x[curTerm], term_y[curTerm]);
		return;
	}	

	if(termIndex[curTerm] < (MAXBUFFERSIZE - 1))				//buffer size - 1 for the last entry to be an enter
	{
		keyboardBuffer[curTerm][termIndex[curTerm]] = keypress;
		if(keypress == '\n')
		{
			goto NewLine2;
		}
		switch(curTerm)											//Write the keypress to the correct buffer it passed through all the checks 
		{														//of being an incorrect keypress
			case TERM1:
				*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1)) = keypress;
				*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1) + 1) = ATTRIB;
				break;
			case TERM2:
				*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1)) = keypress;
				*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1) + 1) = ATTRIB;
				break;
			case TERM3:
				*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1)) = keypress;
				*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[curTerm] + term_x[curTerm]) << 1) + 1) = ATTRIB;
				break;
			default:
				break;
		}
		term_x[curTerm]++;
		if(term_x[curTerm] == NUM_COLS)				//check to see if the x position of the current terminal is out of bounds, if it is put it to 0
		{
			NewLine2:
			term_x[curTerm] = 0;
			newLineFunc(curTerm);
		}
		updateCursor(term_x[curTerm], term_y[curTerm]);
		termIndex[curTerm]++;
	}
}

/*
* void newLineFunc(void)
*   Inputs: void
*   Return Value: void
*	Function: Increments the term_y global variable, if it is on the last line copy rows 1-24 into 0-23 then
*			  outputs a blank line into line 24
*/
void newLineFunc(int TermVal)
{
	if(term_y[TermVal] != NUM_ROWS -1)
	{
		term_y[TermVal] += 1;
	}
	else
	{
		int i;
		int j;
		for(i = 1; i < NUM_ROWS; i++)				//Copy lines of 1 - 24 to 0 - 23 in memory of specific buffer
		{											
			for(j = 0; j < NUM_COLS; j++)
			{
				switch(TermVal)
				{
					case TERM1:
						*(uint8_t *)(term1ptr + ((NUM_COLS*(i-1) + j) << 1)) = *(uint8_t *)(term1ptr + ((NUM_COLS*i + j) << 1));
						*(uint8_t *)(term1ptr + ((NUM_COLS*(i-1) + j) << 1) + 1) = ATTRIB;
						break;
					case TERM2:
						*(uint8_t *)(term2ptr + ((NUM_COLS*(i-1) + j) << 1)) = *(uint8_t *)(term2ptr + ((NUM_COLS*i + j) << 1));
						*(uint8_t *)(term2ptr + ((NUM_COLS*(i-1) + j) << 1) + 1) = ATTRIB;
						break;
					case TERM3:
						*(uint8_t *)(term3ptr + ((NUM_COLS*(i-1) + j) << 1)) = *(uint8_t *)(term3ptr + ((NUM_COLS*i + j) << 1));
						*(uint8_t *)(term3ptr + ((NUM_COLS*(i-1) + j) << 1) + 1) = ATTRIB;
						break;
					default:
						break;
				}
			}
		}
		
		for(i = 0; i < NUM_COLS; i++)					//output a blank line since the new line button was pressed.
		{	
			switch(TermVal)
			{
				case TERM1:
					*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[TermVal] + i) << 1)) = ' ';
					*(uint8_t *)(term1ptr + ((NUM_COLS*term_y[TermVal] + i) << 1) + 1) = ATTRIB;
					break;
				case TERM2:
					*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[TermVal] + i) << 1)) = ' ';
					*(uint8_t *)(term2ptr + ((NUM_COLS*term_y[TermVal] + i) << 1) + 1) = ATTRIB;
					break;
				case TERM3:
					*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[TermVal] + i) << 1)) = ' ';
					*(uint8_t *)(term3ptr + ((NUM_COLS*term_y[TermVal] + i) << 1) + 1) = ATTRIB;
					break;
				default:
					break;
			}
		}
	}
}

/*
* void clearKeybrdBuffer(void)
*   Inputs: void
*   Return Value: void
*	Function: Sets all the keyboard buffer values to NULL. Then places cursor value to 0 and index to 0
*			  in order to prepare for new key input.
*/
void clearKeybrdBuffer(int termVal)
{
	term_x[termVal] = 0;
	termIndex[termVal] = 0;
	int i;
	for(i = 0; i < MAXBUFFERSIZE; i++)
	{
		keyboardBuffer[termVal][i] = VOIDVAL;				//sets all entries to null
	}

}

/*
* void updateCursor(int s_x, int s_y)
*   Inputs: two position integers, one for the x position and the other for the y
*   Return Value: void
*	Function: Finds the position on the screen based off the x and y positions then updates the cursor position
*			  by writing to the specific ports.
*/
void updateCursor(int s_x, int s_y)
{
	int position = s_x + s_y * NUM_COLS;

	outb(0x0F, CURSORPORT);											//0x0F required for the low port
	outb((unsigned char)(position & 0xFF), CURSORDATA);

	outb(0x0E, CURSORPORT);											//0x0E required for the high port
	outb((unsigned char)((position>>8) & 0xFF), CURSORDATA);
}

/*
* int setFlags(unsigned char keypress)
*   Inputs: keypress - the most recent key that was pressed
*   Return Value: 1 if there was a flag that was changed
*				  0 otherwise
*	Function: Based on the most recent key that was pressed, certain global variables will be changed
*			  to show that these specific keys have been pressed. The keys in consideration are caps, shift,
*			  control, and alt.
*/
int setFlags(unsigned char keypress)
{
	switch(keypress)
	{
		case MAKESHIFTL:
			shift_flag = 1;
			return FLAGSET;
		case MAKESHIFTR:
			shift_flag = 1;
			return FLAGSET;	
		case BREAKSHIFTL:
			shift_flag = 0;
			return FLAGSET;
		case BREAKSHIFTR:
			shift_flag = 0;
			return FLAGSET;
		case MAKECTRL:
			ctrl = 1;
			return FLAGSET;
		case BREAKCTRL:
			ctrl = 0;
			return FLAGSET;
		case MAKEALT:
			alt = 1;
			return FLAGSET;
		case BREAKALT:
			alt = 0;
			return FLAGSET;
		case MAKEKEYL:
			if(ctrl)							//this is the function of CRTL-L where the memory of the terminal will be cleared
			{									//with spaces and the cursor should be updated
				clear();
				term_y[curTerm] = 0;
				clearKeybrdBuffer(curTerm);
				updateCursor(0,0);
				return FLAGSET;
			}
			return NOFLAGSET;
		case BREAKCAPS:							
			if(capsflag)
				capsflag = 0;
			else
				capsflag = 1;
			return FLAGSET;	
		case MAKEF1:							//This is the function of ALT-F1 where the terminal is switching to the term 1
			if(alt)
			{																			
				curTerm = TERM1;
				updateCursor(term_x[curTerm], term_y[curTerm]);
			}
			return FLAGSET;
		case MAKEF2:							//This is the function of ALT-F2 where the terminal is switching to the term 2
			if(alt)
			{
				curTerm = TERM2;
				updateCursor(term_x[curTerm], term_y[curTerm]);
				
			}
			return FLAGSET;
		case MAKEF3:							//This is the function of ALT-F3 where the terminal is switching to the term 3
			if(alt)
			{
				curTerm = TERM3;
				updateCursor(term_x[curTerm], term_y[curTerm]);
			}
			return FLAGSET;
	}
	if(ctrl)
		return FLAGSET;
	if(alt)
		return FLAGSET;

	return NOFLAGSET;
}


/*
* int changeToAlphaNumeric(int keypress)
*   Inputs: keypress - the most recent key that was pressed
*   Return Value: positive number if a key was changed 
*				  0 otherwise
*	Function: These keys will be changed from their current representation to their corrseponding
*			  shift representation. For example, a 1 will be changed to a !, 1 --> !, 2 --> @, 3 --> #, etc.
*/
int changeToAlphaNumeric(int keypress)
{
	switch(keypress)
	{
		case ASCIIZERO:
			return RPARANTH;
		case ASCIIONE:
			return EXCLPOINT;
		case ASCIITWO:
			return ATSYMB;
		case ASCIITHREE:
			return POUND;
		case ASCIIFOUR:
			return MONEY;
		case ASCIIFIVE:
			return PERCENT;
		case ASCIISIX:
			return CARROT;
		case ASCIISEVEN:
			return AMPERSAND;
		case ASCIIEIGHT:
			return MULT;
		case ASCIININE:
			return LPARANTH;
		case GRAVEACCENT:
			return TILDE;
		case HYPHEN:
			return UNDERSCORE;
		case EQUALSSIGN:
			return PLUS;
		case LBRACKET:
			return LDRAGON;
		case BLASH:
			return PARALLEL;
		case RBRACKET:
			return RDRAGON;
		case SEMICOLON:
			return COLON;
		case SINGLEQUOTE:
			return DBLQUOTE;
		case COMMA:
			return LANGLE;
		case PERIOD:
			return RANGLE;
		case FLASH:
			return QUESTION;
	}

	return 0;
}

//**********CHECKPOINT 5 STUFF****************

/*
* int32_t init_pit(void)
*   Inputs: None
*   Return Value: SUCCESS_CODE or ERROR_CODE
*	Function: This function will set the frequency of the PIT (programmable
				interrupt timer) so the OS scheduler can keep track of all tasks.

				We want the frequency to be between 20-100 Hz
*/
int32_t init_pit()
{

	outb(CMD_PORT_VAL, COMMANDPORT);
	outb(MAGIC_PIT & 0xFF, CHANPORT0);
	outb(MAGIC_PIT >> 8, CHANPORT0);

	enable_irq(PIT_IRQ);

	return SUCCESS_CODE;
}
