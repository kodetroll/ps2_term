ps2_term
========
Description
----------------
PS2 Keyboard and LCD serial terminal for Atmel AVR ATTiny4313.

How to get
----------------
To clone this repo, type:

git clone https://github.com/kodetroll/ps2_term.git

Discussion
----------------
Allows user to connect a PS2 keyboard and a 24X2 character (HD44780 style) LCD
display to the Futurlec board and use this as a serial terminal. The code will
not fit on a 2313, so a ATTiny4313 was selected instead. This project uses 
modified copies of Peter Fleury's LCD library (Hacked to accomodate no RW pin
on the LCD, a Futurlec derp) and Jurre Hanema's PS2KBD library. 

Hardware
----------------
The PS2 keyboard has four connections that must be dealt with. Power, ground, 
clock and data. Power is supplied to the keyboard from the Futurlec boards VCC connection, as is ground. The PS2 keyboard clock is attached to INT1 (as this 
signal needs to be handled by an interrupt) and the data is attached to a GPIO
pin (usually PB4). These pins can easily be accessed via the EXP20 connector
on the Futurlec board. The LCD is connected via the LCD connector on the 
Futurlec board. Any LCD that normally works with this connector should suffice
for this application. Realize that the futurlec board LCD pin on the header are
right to left swapped. I suspect that the connector was originally intended to
be connected on the bottom side of the board. If you have a Futurlec provided
LCD, then this should work. I used a surplus 16x2 board that was available 
cheap. The wholesaler was practically giving them away as the ribbon cable
connection was on the wrong side of the board, thus reversing the connections.
Funny that it worked for me ;)

        PS2 Keyboard connector          

   Pin  Name   Dir       Description    
  ----------------------------------
    1   DATA   <->   Key Data       
    
    2   n/c    ---   Not connected      

    3   GND    ---   Ground             

    4   VCC    -->   Power , +5 VDC     

    5   CLK    -->   Clock              

    6   n/c    ---   Not connected      


 Note: Direction is Computer relative Keyboard.

To implement this PS2 Keyboard library, The Keyboard Clock line must be 
connected to INT1 (PD3), and the data line to PD4

  CLK - PD3 (7)

  DAT - PD4 (8)


Futurlec EXP20 Connector Pinout to PS2 Keyboard connector pinout
 Pin  Name  Func   PS2 Conn
 --------------------------

  1 - VCC - PWR - 4

 14 - PD3 - CLK - 5

 16 - PD4 - DAT - 1

 20 - GND - GND - 3 

All parts not otherwise so:
(C) 2012 KB4OID Labs, a division of Kodetroll Heavy Industries

Thanks and good luck.....Kodetroll
