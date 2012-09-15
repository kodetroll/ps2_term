/**************************************************************************
 *
 * UART.C - ET-JRAVR UART Code
 * This is a library module that provides UART/USART functions that mimic
 * some LCD library functions. It can be used to initialize the UART/USART
 * and to send characters via the USART/UART. Note, the RX ISR is located
 * in the main module as it interacts with globals.
 *
 * (C) 2012 KB4OID Labs, A division of Kodetroll Industries
 * All Rights Reserved
 *
 **************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "uart.h"

//char tbuf[16];

void UART_init(const uint8_t baud_rate);
void UART_Send_Char(const char c);
void SendSTR_P(const char *FlashSTR);
void UART_putc(const char c);
void UART_puts(const char *s);

// Initialize the UART
void UART_init(const uint8_t baud_rate)
{
	// Set up UART
	switch (baud_rate)
	{
		case BR1200:
			UBRR = UBRR_1200;		// 1200Baud @ 8MHz
			break;

		case BR2400:
			UBRR = UBRR_2400;		// 2400Baud @ 8MHz
			break;

		case BR4800:
			UBRR = UBRR_4800;		// 2400Baud @ 8MHz
			break;

		case BR9600:
			UBRR = UBRR_9600;		// 2400Baud @ 8MHz
			break;

		case BR14400:
			UBRR = UBRR_14400;		// 2400Baud @ 8MHz
			break;

		case BR19200:
			UBRR = UBRR_19200;		// 2400Baud @ 8MHz
			break;

		case BR28800:
			UBRR = UBRR_28800;		// 2400Baud @ 8MHz
			break;

		case BR38400:
			UBRR = UBRR_38400;		// 2400Baud @ 8MHz
			break;

		case BR57600:
			UBRR = UBRR_57600;		// 2400Baud @ 8MHz
			break;

		case BR76800:
			UBRR = UBRR_76800;		// 2400Baud @ 8MHz
			break;

		case BR115200:
			UBRR = UBRR_115200;		// 2400Baud @ 8MHz
			break;

		default:
			UBRR = UBRR_9600;		// 2400Baud @ 8MHz
			break;
	}
	
#ifdef ATtiny4313
	UBRRH = 0;		// same for all baud rates
#endif

	// Turn on UART TX and RX
#ifdef ATtiny4313
	UCSRB |= _BV(RXEN) | _BV(TXEN);
	UCSRB |= _BV(RXCIE ); // Enable the USART Recieve Complete interrupt ( USART_RXC )
#else
	UCR |= _BV(RXEN) | _BV(TXEN);
	UCR |= _BV(RXCIE ); // Enable the USART Recieve Complete interrupt ( USART_RXC )
#endif
}

// Sends a single char to the serial port
void UART_Send_Char(const char c)
{
	// If previous char is still being sent then wait until done
#ifdef ATtiny4313
	//if (UCSRA & _BV(UDRE))
	if (_BV(UDRE))
#else
	if (_BV(UDRE))
#endif
		UDR = c;

	// Send the char and wait for it to finish
#ifdef ATtiny4313
	while (( UCSRA & _BV(UDRE)) == 0) {};
#else
	while (( USR & _BV(UDRE)) == 0) {};
#endif
}

// Sends a string of text from PGM Memory to the serial port
void SendSTR_P(const char *FlashSTR)
{

	uint8_t i;

	for (i=0; pgm_read_byte(&FlashSTR[i]); i++)
		UART_Send_Char(pgm_read_byte(&FlashSTR[i]));

}

// Sends a single char of text to the serial port
void UART_putc(const char c)
{
	UART_Send_Char(c);
}

// Sends a string of text to the serial port
void UART_puts(const char *s)
{
    register char c;

    while ( (c = *s++) ) {
        UART_Send_Char(c);
    }
	
}
