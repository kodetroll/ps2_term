/*****************************************************************************
 *
 * ps2_term.c - Main module for ps2_term app. This program will allow the use
 * of a standard PS2 keyboard with a Tiny4313 and LCD to form a simple serial
 * terminal using Futurlec style development board. This app uses modified 
 * versions of PFleury's LCD library (hacked to ignore R/W) and Jurre Hanema's
 * PS2KBD libraries. It is configured for FULL Duplex mode, what is typed on 
 * the PS2 keyboard is sent via the USART and what is received on the USART is 
 * displayed to the LCD. This version gets around the slow LCD and no RW by 
 * saving a copy of the incoming line to a buffer, as well as printing it to
 * the LCD. The code is arranged so that whenever a CR is received, the screen
 * is cleared, the line buffer is copied to the first line of the display and 
 * the cursor is moved to the second line to be ready to receive characters. 
 * As the characters are received, they are printed to the second line of the 
 * LCD and saved in the line buffer, where the next CR will cause the process
 * to repeat. Baud rate is currently fixed, but changeable via a define and
 * recompile. Echo and LF Add are variables. Control codes (CTRL-C, etc) are 
 * currently not supported. A different method of defining Scancode to ASCII
 * code conversions needs to be built. Not all PS2 keyboard keys are decoded,
 * mainly, letters, numbers, some punctuation and a few control keys (ENTER, 
 * BACKSPACE). Code space utilization is at ~65% on a Tiny4313.
 * 
 * (C) 2012 KB4OID Labs, a division of Kodetroll Heavy Industries.
 * All respective rights to their owners.
 *
 * Hardware connection info:
 * PS2 Keyboard connector:
 * Pin 	Name 	Dir 	Description
 *-----------------------------------
 * 1 	DATA 	<-> 	Key Data
 * 2 	n/c 	--- 	Not connected
 * 3 	GND 	--- 	Gnd
 * 4 	VCC 	--> 	Power , +5 VDC
 * 5 	CLK 	--> 	Clock
 * 6 	n/c 	--- 	Not connected
 * 
 * Note: Direction is Computer relative Keyboard.
 * 
 * To implement the PS2 Keyboard library, The Keyboard Clock line must be 
 * connected to PD3 (INT1), and the Data to PD4
 * CLK - PD3 (7)
 * DAT - PD4 (8)
 *
 * Futurlec EXP20 Connector Pinout to PS2 Keyboard connector pinout
 *  1 - VCC - PWR - 4
 * 14 - PD3 - CLK - 5
 * 16 - PD4 - DAT - 1
 * 20 - GND - GND - 3
 *
 * The LCD Display is connected via the futurlec LCD Display connector - J8
 *
 ******************************************************************************/

#include "ps2_term.h"

const char SignOnString[] PROGMEM = "PS2 Keybd Term V0.99                    ";
const char CopyrightString[] PROGMEM = "(C) 2012 KB4OID Labs";
const char HexString[] PROGMEM = "0123456789ABCDEF";
const char IDString[] PROGMEM = "@0104:0002:0000";
const char CRLF[] PROGMEM = {0x0D, 0x0A, 0x00};

uint8_t idx = 0;
uint8_t echo = OFF;
uint8_t lfadd = ON;

char linebuf[LINE_SZ];


/*************************************************************************
 * Low-level ISR function to receive a byte from the USART
 *
 * Input:    USART RX Vector
 * Modifies: interrupts
 * Returns:  none
 *************************************************************************/
 
ISR ( USART_RX_vect )
{
	unsigned char ReceivedByte;

	// Disable interrupts
	cli();

	// Copy the received byte value 
	ReceivedByte = UDR ; 

	// Process the received character as type "COM"
	process_char(COM, ReceivedByte);
	
	// re-enable interrupts
	sei();
}


/*************************************************************************
 * Low-level function to clear the contents of the line buffer character
 * array.
 *
 * Input:    none
 * Modifies: global linebuf*
 * Returns:  none
 * 
 *************************************************************************/

void clr_buf(void)
{
	uint8_t i = 0;

	// Start with an empty linebuf
	for (i=0;i<LINE_SZ;i++)
		linebuf[i] = 0x00;

}

/*************************************************************************
 * Function to send pre-defined instrument ID string to USART. This allows
 * a system connected to this device to identify the type of device 
 * attached.  The ID string is stored in PROGMEM.
 *
 * Input:    none
 * Modifies: none
 * Returns:  none
 * 
 *************************************************************************/

void send_id(void)
{
	// copy ID to USART
	SendSTR_P(IDString);

	// copy CR LF to USART
	SendSTR_P(CRLF);
}

/*************************************************************************
 * Function to send pre-defined header and copyright strings to the USART
 * These strings are stored in PROGMEM.
 *
 * Input:    none
 * Modifies: none
 * Returns:  none
 * 
 *************************************************************************/

void send_signon(void)
{
	// copy Welcome Header to USART
	SendSTR_P(SignOnString);
	SendSTR_P(CRLF);

	// copy (C) Header to USART
	SendSTR_P(CopyrightString);
	SendSTR_P(CRLF);

	// Send ID string to USART
	send_id();

    /* initialize display, cursor off */
    lcd_init(LCD_DISP_ON);

	/* clear display and home cursor */
	lcd_clrscr();

	/* put signong string to LCD display (line 1) with linefeed */
	lcd_puts_p(SignOnString);
	
	/* put (C) string to LCD display (line 2) with linefeed */
	lcd_puts_p(CopyrightString);
	
}

/*************************************************************************
 * Function to process a received byte (either keyboard or serial) based
 * on type.
 *
 * Input:	uint8_t source
 *			unsigned char c 
 * Modifies: global linebuf, writes to USART and LCD
 * Returns:  none
 * 
 *************************************************************************/

void process_char(uint8_t source, unsigned char c)
{
	// If C is a Carriage Return
	if (c == CR)
	{
		if (source == COM || (source == KBD && echo == ON))
		{

			// properly terminate the buffer
			linebuf[idx] = 0x00;

			// clear the LCD screen (and cursor to 0,0)
			lcd_clrscr();

			// move the cursor to line 2
			lcd_gotoxy(0,0);

			// copy the current contents of the line
			// buffer to the first line of the LCD 
			lcd_puts(linebuf);
				
			// move the cursor to line 2
			lcd_gotoxy(0,1);
				
			// clear the line buffer to make room for the next line
			clr_buf();

			// reset the current char index to beginning of the buffer
			idx = 0;
		}
		
		// copy the char to the USART 
		if (source == KBD || (source == COM && echo == ON))
		{
			UART_putc(c);

			// Add a LF, if defined
			if (lfadd)
				UART_putc(LF);
		}
		// reset the char to 0x00
		c = 0;

	}
	else			
	{
		// char was NOT a CR, so
		// write char to current cursor position
		// on LCD display
		if (source == COM || (source == KBD && echo == ON))
		{
			lcd_putc(c);

			// add the char to the line buffer 
			// and increment the index pointer
			linebuf[idx++] = c;

			// handle (crudly) index past end of buffer
			if (idx > LINE_SZ-1)
				idx = LINE_SZ-1;
		}

		// echo the char to the USART
		if (source == KBD || (source == COM && echo == ON))
			UART_putc(c);

	}
}

int main(void)
{
	unsigned char c = 0;
	
	echo = OFF;
	lfadd = ON;
	
	// Initialize the PS2 Keyboard queue
	kbd_init();
	
	// Initialize the LCD display
	lcd_init(LCD_DISP_ON);
	
	// Initialize the USART to the specified BAUD rate
	UART_init(BAUD);

	// Initiate Interrupts
	sei ();

	// Send the wordy damn signon message
	send_signon();

	// Show it for 3 seconds
	_delay_ms(3000);
	
	idx = 0;

	// properly terminate linebuf (JIC)
	linebuf[idx] = 0x00;
	
	// Clear the LCD screen
	lcd_clrscr();
	
	// copy the contents of the linebuf to the 
	// first line of the LCD display
	lcd_puts(linebuf);
	
	// put the cursor on the second (bottom) 
	// line of the LCD display.
	lcd_gotoxy(0,1);
	
	// clear the linebuf
	clr_buf();

	// start the terminal loop
	while(1)
	{
		// if c is other than 0x00, then 
		while((c = kbd_getchar()))
			process_char(KBD,c);

	}

	return 0;
}
