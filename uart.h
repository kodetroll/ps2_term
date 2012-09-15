/**************************************************************************
 *
 * UART.H - ET-JRAVR UART Code definitions
 * This is a library module that provides UART/USART functions that mimic
 * some LCD library functions. It can be used to initialize the UART/USART
 * and to send characters via the USART/UART. Note, the RX ISR is located
 * in the main module as it interacts with globals.
 *
 * (C) 2012 KB4OID Labs, A division of Kodetroll Industries
 * All Rights Reserved
 *
 **************************************************************************/

#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

//extern char tbuf[16];

#define ATtiny4313
// 2400 UBRR= 207 0.2
// 4800 UBRR= 103 0.2
// 9600 UBRR= 51 0.2
// 14400 UBRR= 34 0.8
// 19200 UBRR= 25 0.2
// 28800 UBRR= 16 2.1
// 38400 UBRR= 12 0.2
// 57600 UBRR= 8 3.7
// 76800 UBRR= 6 7.5
// 115200 UBRR= 3 7.8

// These values are for 8 MHz, 2313
#define UBRR_1200 	207
#define UBRR_2400 	207
#define UBRR_4800 	103
#define UBRR_9600 	 51
#define UBRR_14400 	 34
#define UBRR_19200 	 25
#define UBRR_28800 	 16
#define UBRR_38400 	 12
#define UBRR_57600 	  8
#define UBRR_76800 	  6
#define UBRR_115200   3

#define UBRR_1200 	207
#define UBRR_2400 	207
#define UBRR_4800 	103
#define UBRR_9600 	 51
#define UBRR_14400 	 34
#define UBRR_19200 	 25
#define UBRR_28800 	 16
#define UBRR_38400 	 12
#define UBRR_57600 	  8
#define UBRR_76800 	  6
#define UBRR_115200   3

#ifdef ATtiny4313
#define UBRR _SFR_IO8(0x009)
#endif

enum BaudRates {
	BR1200,
	BR2400,
	BR4800,
	BR9600,
	BR14400,
	BR19200,
	BR28800,
	BR38400,
	BR57600,
	BR76800,
	BR115200,
};

void UART_init(const uint8_t baud_rate);
void UART_Send_Char(const char c);
void SendSTR_P(const char *FlashSTR);
void UART_putc(const char c);
void UART_puts(const char *s);

#endif //UART_H