/*****************************************************************************
 *
 * ps2_term.h - Main module for ps2_term app. This program will allow the use
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
 * recompile. Echo and LF Add are variables. 
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
 * To implement this PS2 Keyboard library, The Keyboard Clock line must be 
 * connected to INT1 (PD3), and the data to PD4
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "lcd_norw.h"
#include "uart.h"
#include "ps2kbd.h"
#include "ascii.h"


#ifndef __PS2_TERM_H__
#define __PS2_TERM_H__

#define ATtiny4313

#define BAUD BR9600

#define LINE_SZ 40

#define LF_AFTER_CR

#define KBD 1
#define COM 0

#define ON 1
#define OFF 0

void clr_buf(void);
void send_id(void);
void send_signon(void);
void process_char(uint8_t source, unsigned char c);

#endif // __PS2_TERM_H__