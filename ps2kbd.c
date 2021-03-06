/* PS/2 keyboard routines by Jurre Hanema, version 1.0
 * ps2kbd.c
 *
 * Released under "THE BEER-WARE LICENSE" (Revision 42):
 * <kipmans@gmail.com> wrote this file. As long as you retain this notice you can 
 * do whatever you want with this stuff. If we meet some day, and you think this 
 * stuff is worth it, you can buy me a beer in return.
 *
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "ps2kbd.h"
#include "ascii.h"

#define LOGIC_XOR(a, b)	(((a) && !(b)) || ((b) && !(a)))


volatile uint8_t	kbd_bit_n = 1;
volatile uint8_t	kbd_n_bits = 0;
volatile uint8_t	kbd_buffer = 0;
volatile uint8_t	kbd_queue[KBD_BUFSIZE + 1];
volatile uint8_t	kbd_queue_idx = 0;
volatile uint16_t	kbd_status = 0;

const unsigned char lut_normal_keys[] PROGMEM = {

	0x7c, '*',
	0x7b, '-',
	0x79, '+',
	0x70, '0',
	0x69, '1',
	0x7a, '3',
	0x73, '5',
	0x6c, '7',
	0x7d, '9',
	0x71, '.',
	0x75, DC1,	/* Arrow keys */
	0x72, DC2,
	0x6b, DC3,
	0x74, DC4,
	0x1C, 'a',
	0x32, 'b',
	0x21, 'c',
	0x23, 'd',
	0x24, 'e',
	0x2B, 'f',
	0x34, 'g',
	0x33, 'h',
	0x43, 'i',
	0x3B, 'j',
	0x42, 'k',
	0x4B, 'l',
	0x3A, 'm',
	0x31, 'n',
	0x44, 'o',
	0x4D, 'p',
	0x15, 'q',
	0x2D, 'r',
	0x1B, 's',
	0x2C, 't',
	0x3C, 'u',
	0x2A, 'v',
	0x1D, 'w',
	0x22, 'x',
	0x35, 'y',
	0x1A, 'z',
	0x45, '0',
	0x16, '1',
	0x1E, '2',
	0x26, '3',
	0x25, '4',
	0x2E, '5',
	0x36, '6',
	0x3D, '7',
	0x3E, '8',
	0x46, '9',
	0x29, ' ',
	0X5A, CR,
	0X66, BS,
	0x0D, TAB,	
	0X54, '[',
	0X5B, ']',
	0X4E, '-',
	0X55, '=',
	0X4C, ';',
	0X52, 39,
	0X0E, 44,
	0X41, ',',
	0X49, '.',
	0X5D, 92,
	0X4A, '/',
	0, 0, 0
};


// Lookup table for 'normal' scancodes when SHIFT is held down
// If no entry is found in this table, the uppercase equivalent of the result
// from lut_normal_keys will be used.

const unsigned char lut_normal_keys_shift[] PROGMEM = {
	0x1C, 'A',
	0x32, 'B',
	0x21, 'C',
	0x23, 'D',
	0x24, 'E',
	0x2B, 'F',
	0x34, 'G',
	0x33, 'H',
	0x43, 'I',
	0x3B, 'J',
	0x42, 'K',
	0x4B, 'L',
	0x3A, 'M',
	0x31, 'N',
	0x44, 'O',
	0x4D, 'P',
	0x15, 'Q',
	0x2D, 'R',
	0x1B, 'S',
	0x2C, 'T',
	0x3C, 'U',
	0x2A, 'V',
	0x1D, 'W',
	0x22, 'X',
	0x35, 'Y',
	0x1A, 'Z',
	0x45, ')',
	0x16, '!',
	0x1E, '@',
	0x26, '#',
	0x25, '$',
	0x2E, '%',
	0x36, '^',
	0x3D, '&',
	0x3E, '*',
	0x46, '(',
	0x29, ' ',
	0X5A, CR,
	0X66, BS,
	0X54, '{',
	0X5B, '}',
	0X4E, '_',
	0X55, '+',
	0X4C, ':',
	0X52, 34,
	0X0E, '~',
	0X41, '<',
	0X49, '>',
	0X5D, '|',
	0X4A, '?',
	0, 0
};


// Lookup table for 'normal' scancodes when NUMLOCK is on
// Can, for example, be used to change the behaviour of the keypad keys if desired

const unsigned char lut_normal_keys_numlock[] PROGMEM = {
	0x75, '8',
	0x72, '2',
	0x6b, '4',
	0x74, '6',
	0, 0
};


// Lookup table for 'extended' scancodes
// Scancode without e0

const unsigned char lut_extended_keys[] PROGMEM = {
	0x4a, '/',
	0x5a, 13,
	0, 0
};


// Begin actual implementation

void kbd_init(void)
{
	kbd_queue[KBD_BUFSIZE] = 0;
	
	// Set interrupts
	
	KBD_SET_INT();
	KBD_EN_INT();
	
	// Enable pullup on clock
	
	KBD_CLOCK_PORT |= _BV(KBD_CLOCK_BIT);
	
	sei();
}


uint8_t kbd_kbd_queue_scancode(volatile uint8_t p)
{
	if(kbd_queue_idx < KBD_BUFSIZE)
	{
		kbd_queue[kbd_queue_idx] = p;
		kbd_queue_idx++;
	} else
		return 0;

	return 1;
}


uint8_t kbd_get_scancode(void)
{
	uint8_t		i;
	uint8_t		tmp;

	if(kbd_queue_idx > 0)
	{
		tmp = kbd_queue[kbd_queue_idx - 1]; 
		
		for(i = 0; i < KBD_BUFSIZE; i++)
			kbd_queue[i] = kbd_queue[i + 1];
		
		kbd_queue_idx--;
		
		return tmp;
	} else
		return 0;
}


void kbd_send(uint8_t data)
{
	while(kbd_status & KBD_SEND) _delay_ms(5);	// This behaviour isn't the most desirable, but it's the easiest and proved to be reliable.
	
	// Initiate request-to-send, the actual sending of the data
	// is handled in the ISR.
	
	KBD_CLOCK_PORT &= ~_BV(KBD_CLOCK_BIT);
	KBD_CLOCK_DDR |= _BV(KBD_CLOCK_BIT);
	_delay_us(120);
	KBD_DATA_DDR |= _BV(KBD_DATA_BIT);
	KBD_CLOCK_DDR &= ~_BV(KBD_CLOCK_BIT);
	KBD_CLOCK_PORT |= _BV(KBD_CLOCK_BIT);
	
	kbd_bit_n = 1;
	kbd_status |= KBD_SEND;
	kbd_n_bits = 0;
	kbd_buffer = data;
}


void kbd_update_leds(void)
{
	uint8_t	val = 0;

	if(kbd_status & KBD_CAPS) val |= 0x04;
	if(kbd_status & KBD_NUMLOCK) val |= 0x02;
	if(kbd_status & KBD_SCROLL) val |= 0x01;
	
	kbd_send(0xed);
	kbd_send(val);
}


unsigned char kbd_do_lookup(const unsigned char *lut, uint8_t sc)
{
	uint8_t	i;
	
	for(i = 0; pgm_read_byte(&lut[i]); i += 2)
		if(sc == pgm_read_byte(&lut[i]))
			return pgm_read_byte(&lut[i + 1]);
	return 0;
}


unsigned char kbd_getchar(void)
{
	uint8_t		sc = 0;
	unsigned char	c;
	
	while((sc = kbd_get_scancode()))
	{
		if(sc == 0xaa)
			kbd_status |= KBD_BAT_PASSED;
		else if(sc == 0xe0)
			kbd_status |= KBD_EX;
		else if(sc == 0xf0)
			kbd_status |= KBD_BREAK;
		else
		{
			if(kbd_status & KBD_BREAK)
			{
				kbd_status &= ~KBD_BREAK;
				kbd_status &= ~KBD_EX;
				
				if(sc == 0x12 || sc == 0x59)	// Shift
					kbd_status &= ~KBD_SHIFT;
				else if(sc == 0x14)		// Ctrl
					kbd_status &= ~KBD_CTRL;
				else if(sc == 0x11)		// Alt
					kbd_status &= ~KBD_ALT;
				else if(sc == 0x77 || sc == 0x58 || sc == 0x7e)	// Caps lock, num lock or scroll lock
					kbd_status &= ~KBD_LOCKED;
			} else if(kbd_status & KBD_EX)
			{
				kbd_status &= ~KBD_EX;
				
				if(sc == 0x14)			// R ctrl
					kbd_status |= KBD_CTRL;
				else if(sc == 0x11)		// R alt
					kbd_status |= KBD_ALT;
				else if((c = kbd_do_lookup(lut_extended_keys, sc)))
					return c;
				//else
				//	return sc;
			} else
			{
				if(sc == 0x12 || sc == 0x59)	// Shift
					kbd_status |= KBD_SHIFT;
				else if(sc == 0x14)		// L ctrl
					kbd_status |= KBD_CTRL;
				else if(sc == 0x11)		// L alt
					kbd_status |= KBD_ALT;
				else if(sc == 0x77 && !(kbd_status & KBD_LOCKED))		// Num lock
				{
					kbd_status = (kbd_status & KBD_NUMLOCK) ? kbd_status & ~KBD_NUMLOCK : kbd_status | KBD_NUMLOCK;
					kbd_status |= KBD_LOCKED;
					kbd_update_leds();
				} else if(sc == 0x58 && !(kbd_status & KBD_LOCKED))		// Caps lock
				{
					kbd_status = (kbd_status & KBD_CAPS) ? kbd_status & ~KBD_CAPS : kbd_status | KBD_CAPS;
					kbd_status |= KBD_LOCKED;
					kbd_update_leds();
				} else if(sc == 0x7e && !(kbd_status & KBD_LOCKED))		// Scroll lock
				{
					kbd_status = (kbd_status & KBD_SCROLL) ? kbd_status & ~KBD_SCROLL : kbd_status | KBD_SCROLL;
					kbd_status |= KBD_LOCKED;
					kbd_update_leds();
				} else
				{
					if((kbd_status & KBD_SHIFT) && (c = kbd_do_lookup(lut_normal_keys_shift, sc)))
						return c;
					else if((kbd_status & KBD_NUMLOCK) && (c = kbd_do_lookup(lut_normal_keys_numlock, sc)))
						return c;
					else if((c = kbd_do_lookup(lut_normal_keys, sc)))
						return (LOGIC_XOR(kbd_status & KBD_SHIFT, kbd_status & KBD_CAPS) && (c >= 'a' && c <= 'z')) ? c - 32 : c;
				}
			}
		}
	}
	
	return 0;
}


uint16_t kbd_get_status(void)
{
	return kbd_status;
}


ISR(KBD_INT)
{
	if(kbd_status & KBD_SEND)
	{
		// Send data
		
		if(kbd_bit_n == 9)				// Parity bit
		{
			if(kbd_n_bits & 0x01)
				KBD_DATA_DDR |= _BV(KBD_DATA_BIT);
			else
				KBD_DATA_DDR &= ~_BV(KBD_DATA_BIT);
		} else if(kbd_bit_n == 10)			// Stop bit
			KBD_DATA_DDR &= ~_BV(KBD_DATA_BIT);
		else if(kbd_bit_n == 11)			// ACK bit, set by device
		{
			kbd_buffer = 0;
			kbd_bit_n = 0;
			kbd_status &= ~KBD_SEND;
		} else					// Data bits
		{
			if(kbd_buffer & (1 << (kbd_bit_n - 1)))
			{
				KBD_DATA_DDR &= ~_BV(KBD_DATA_BIT);
				kbd_n_bits++;
			} else
				KBD_DATA_DDR |= _BV(KBD_DATA_BIT);
		}
	} else
	{
		// Receive data
		
		if(kbd_bit_n > 0 && kbd_bit_n < 10)		// Ignore start, parity & stop bit
		{
			if(!bit_is_clear(KBD_DATA_PIN, KBD_DATA_BIT))
				kbd_buffer |= (1 << (kbd_bit_n - 2));
		} else if(kbd_bit_n == 11)
		{
			kbd_kbd_queue_scancode(kbd_buffer);
			kbd_buffer = 0;
			kbd_bit_n = 0;
		}
	}
	
	kbd_bit_n++;
	
	KBD_SET_INT();
}
