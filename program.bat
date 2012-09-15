avrdude -c futurlec -P lpt1 -p t4313 -U flash:w:ps2_term.hex:i
avrdude -c futurlec -P lpt1 -p t4313 -U eeprom:w:ps2_term.eep:i
