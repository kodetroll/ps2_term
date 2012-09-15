/**************************************************************************
 *
 * LCD.C - Cut down version of Peter Fleury's LCD Library for HD44780U
 * This library takes the ET-JRAVR LCD derps into account. The R/W line
 * is pinned to ground in WRITE always mode. This means that reads from
 * the LCD are pointless. This implies, of course, that the busy bit is
 * therefore unavailable, hence the horrendously large delays
 *
 * (C) 2012 KB4OID Labs, A division of Kodetroll Industries
 * All Rights Reserved
 *
 **************************************************************************/

/****************************************************************************
 Title	:   HD44780U LCD library
 Author:    Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 File:	    $Id: lcd.c,v 1.13.2.2 2004/02/12 21:08:25 peter Exp $
 Software:  AVR-GCC 3.3
 Target:    any AVR device, memory mapped mode only for AT90S4414/8515/Mega

 DESCRIPTION
       Basic routines for interfacing a HD44780U-based text lcd display

       Originally based on Volker Oth's lcd library,
       changed lcd_init(), added additional constants for lcd_command(),
       added 4-bit I/O mode, improved and optimized code.

       Library can be operated in memory mapped mode (LCD_IO_MODE=0) or in
       4-bit IO port mode (LCD_IO_MODE=1). 8-bit IO port mode not supported.

       Memory mapped mode compatible with Kanda STK200, but supports also
       generation of R/W signal through A8 address line.

 USAGE
       See the C include lcd.h file for a description of each function

*****************************************************************************/

// extended by Martin Thomas 3/2004, removed bugs(?), added functions
// and maybe added new bugs
// 6/2005 - update to be compatible with avr-libc 1.2.3, mth

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd_norw.h"

#include <util/delay.h>

/*
** constants/macros
*/
#define PIN(x) (*(&x - 2))  /* address of data direction register of port x */
#define DDR(x) (*(&x - 1))  /* address of input register of port x          */


//#define lcd_e_delay()   __asm__ __volatile__( "rjmp 1f\n 1:" );
#define lcd_e_high()    LCD_E_PORT  |=  _BV(LCD_E_PIN);
#define lcd_e_low()     LCD_E_PORT  &= ~_BV(LCD_E_PIN);
#define lcd_e_toggle()  toggle_e()
#define lcd_rw_high()   LCD_RW_PORT |=  _BV(LCD_RW_PIN)
#define lcd_rw_low()    LCD_RW_PORT &= ~_BV(LCD_RW_PIN)
#define lcd_rs_high()   LCD_RS_PORT |=  _BV(LCD_RS_PIN)
#define lcd_rs_low()    LCD_RS_PORT &= ~_BV(LCD_RS_PIN)

#if LCD_IO_MODE
#if LCD_LINES==1
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_1LINE
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES
#endif
#else
#if LCD_LINES==1
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_8BIT_1LINE
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_8BIT_2LINES
#endif
#endif


/*
** function prototypes
*/
static void toggle_e(void);

/*
** local functions
*/

/* toggle Enable Pin to initiate write */
static void toggle_e(void)
{
    lcd_e_low();
    _delay_ms(10);
    lcd_e_high();
}

/*************************************************************************
Low-level function to write byte to LCD controller
Input:    data   byte to write to LCD
          rs     1: write data
                 0: write instruction
Returns:  none
*************************************************************************/
static void lcd_write(uint8_t data,uint8_t rs)
{
    //unsigned char dataBits ;

    if (rs) {   /* write data        (RS=1, RW=0) */
       lcd_rs_high();
    } else {    /* write instruction (RS=0, RW=0) */
       lcd_rs_low();
    }

	/* configure data pins as output */
	DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
	DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
	DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
	DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);

	/* output high nibble first */
	LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
	LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
	LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
	LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
	if(data & 0x80) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
	if(data & 0x40) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
	if(data & 0x20) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
	if(data & 0x10) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
	lcd_e_toggle();

	/* output low nibble */
	LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
	LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
	LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
	LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
	if(data & 0x08) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
	if(data & 0x04) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
	if(data & 0x02) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
	if(data & 0x01) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
	lcd_e_toggle();

	/* all data pins high (inactive) */
	LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
	LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
	LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
	LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);

}

/*************************************************************************
Move cursor to the start of next line or to the first line if the cursor
is already on the last line.
*************************************************************************/
static inline void lcd_newline(uint8_t pos)
{
    register uint8_t addressCounter;


#if LCD_LINES==1
    addressCounter = 0;
#endif
#if LCD_LINES==2
    if ( pos < (LCD_START_LINE2) )
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
#endif
#if LCD_LINES==4
    if ( pos < LCD_START_LINE3 )
        addressCounter = LCD_START_LINE2;
    else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4) )
        addressCounter = LCD_START_LINE3;
    else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2) )
        addressCounter = LCD_START_LINE4;
    else {
		// mtmt added autoscroll
		#ifdef LCD_AUTO_SCROLL
		lcd_scrollup();
		_delay_ms(1);
		addressCounter = LCD_START_LINE4;
		#else
	    addressCounter = LCD_START_LINE1;
		#endif
	}
#endif
    lcd_command((1<<LCD_DDRAM)+addressCounter);

}/* lcd_newline */


/*
** PUBLIC FUNCTIONS
*/

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(uint8_t cmd)
{
    lcd_write(cmd,LCD_CMD);
}

/*************************************************************************
Set cursor to specified position
Input:    x  horizontal position  (0: left most position)
          y  vertical position    (0: first line)
Returns:  none
*************************************************************************/
void lcd_gotoxy(uint8_t x, uint8_t y)
{
#if LCD_LINES==1
    lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
#endif
#if LCD_LINES==2
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
#endif
#if LCD_LINES==4
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else if ( y==1)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
    else if ( y==2)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE3+x);
    else /* y==3 */
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE4+x);
#endif

}/* lcd_gotoxy */


/*************************************************************************
Clear display and set cursor to home position
*************************************************************************/
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
}


/*************************************************************************
Set cursor to home position
*************************************************************************/
void lcd_home(void)
{
    lcd_command(1<<LCD_HOME);
}

/*************************************************************************
Display char 
Input:    char to be displayed
Returns:  none
*************************************************************************/
void lcd_putc(const char c)
/* print char on lcd */
{
    lcd_write(c,LCD_DATA);
	
}/* lcd_putc */

/*************************************************************************
Display string without auto linefeed
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(const char *s)
/* print string on lcd (no auto linefeed) */
{
    register char c;
	uint8_t i = 0;

    while ( (c = *s++) ) {
        lcd_write(c,LCD_DATA);
		i++;
    }

	
}/* lcd_puts */


/*************************************************************************
Display string from program memory without auto linefeed
Input:     string from program memory be be displayed
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_write(c,LCD_DATA);
    }

}/* lcd_puts_p */


/*************************************************************************
Initialize display and select type of cursor
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void lcd_init(uint8_t dispAttr)
{
    /*
     *  Initialize LCD to 4 bit I/O mode
     */

	/* configure all port bits as output (LCD data and control lines on different ports */
	DDR(LCD_RS_PORT)    |= _BV(LCD_RS_PIN);
	DDR(LCD_RW_PORT)    |= _BV(LCD_RW_PIN);
	DDR(LCD_E_PORT)     |= _BV(LCD_E_PIN);
	DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
	DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
	DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
	DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);

    _delay_ms(16);        /* wait 16ms or more after power-on       */

    /* initial write to lcd is 8bit */
    LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);  // _BV(LCD_FUNCTION)>>4;
    LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);  // _BV(LCD_FUNCTION_8BIT)>>4;
    lcd_e_toggle();
    _delay_ms(4);         /* delay, busy flag can't be checked here */

    /* repeat last command */
    lcd_e_toggle();
    _delay_ms(1);           /* delay, busy flag can't be checked here */

    /* repeat last command a third time */
    lcd_e_toggle();
    _delay_ms(1);           /* delay, busy flag can't be checked here */

    /* now configure for 4bit mode */
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);   // LCD_FUNCTION_4BIT_1LINE>>4
    lcd_e_toggle();
    _delay_ms(1);           /* some displays need this additional delay */

    /* from now the LCD only accepts 4 bit I/O, we can use lcd_command() */

    lcd_command(LCD_FUNCTION_DEFAULT);      /* function set: display lines  */
    lcd_command(LCD_DISP_OFF);              /* display off                  */
    lcd_clrscr();                           /* display clear                */
    lcd_command(LCD_MODE_DEFAULT);          /* set entry mode               */
    lcd_command(dispAttr);                  /* display/cursor control       */

}/* lcd_init */
