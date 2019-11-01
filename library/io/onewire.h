/**
 * This tiny software is used to communicate with
 * peripherals over the OneWire protocol.
 *
 * Options for this library:
 *
 * 		- control via macros the timing:
 * 			- #define OW_TIMING_PRECISE ... during each execution interrupts will be deactivated
 * 			- #define OW_TIMING_APPROXIMATELY ... only time critical sections are immune to interrupts
 * 				-> Use the latter one if you have multiple time critical interrupts!
 * 				-> In case no macro has been defined, OW_TIMING_PRECISE will be defined here!
 *
 *
 * This is free software:
 *
 * 		- You are allowed to execute the code how
 * 		  you want to.
 * 		- This code is open source, you are allowed
 * 		  to inspect what the code does.
 * 		- You are allowed to share/redistribute the
 * 		  code on your own to make it accessible to
 * 		  others or for any reason.
 * 		- You are allowed to modify the code and
 * 		  improve it. Hopefully you share your
 * 		  modifications to make it available to all.
 *
 * Check out my website:	https://hedgehogs-mind.com
 * My github account:	  	https://github.com/hedgehogs-mind
 * Contact me:				peter@hedgehogs-mind.com
 *
 *
 * Yours sincerely,
 *
 * Peter Kuhmann
 */

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#define OW_RESET_DELAY_US				480
#define OW_RESET_POST_DELAY_US			70
#define OW_PRESENCE_POST_DELAY_US		410
#define OW_WRITE_0_LOW_DELAY_US			60
#define OW_WRITE_0_LOW_POST_DELAY_US	10
#define OW_WRITE_1_LOW_DELAY_US			6
#define OW_WRITE_1_LOW_POST_DELAY_US	64
#define OW_READ_DEALY_US				9
#define OW_READ_POST_DEALY_US			55

#define OW_CONFIGURE_DATA_LINE_OUTPUT(DDRx, Pxn)	{ DDRx |= (1 << Pxn); }
#define OW_CONFIGURE_DATA_LINE_INPUT(DDRx, Pxn)	 	{ DDRx &= ~(1 << Pxn); }
#define OW_DATA_LINE_HIGH(PORTx, Pxn)				{ PORTx |= (1 << Pxn); }
#define OW_DATA_LINE_LOW(PORTx, Pxn)				{ PORTx &= ~(1 << Pxn); }

#define OW_PULL_BUS_LOW(DDRx, PORTx, Pxn) 			{ OW_CONFIGURE_DATA_LINE_OUTPUT(DDRx, Pxn) OW_DATA_LINE_LOW(PORTx, Pxn) }
#define OW_RELEASE_BUS(DDRx, PORTx, Pxn)			{ OW_DATA_LINE_HIGH(PORTx, Pxn) OW_CONFIGURE_DATA_LINE_INPUT(DDRx, Pxn) }

#if !defined(OW_TIMING_PRECISE) && !defined(OW_TIMING_APPROXIMATELY)
	#define OW_TIMING_PRECISE
#endif

/*
 * This functions emits a one wire reset signal.
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- PINx: Input I/O register Pin X (e.g. PINB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 *
 * Returns:
 * 		- 0 if presence signal has been detected.
 * 		- 1 if no presence signal has been detected. Error handling shall be done.
 */
uint8_t ow_reset(volatile uint8_t *DDRx,
				 volatile uint8_t *PORTx,
				 volatile uint8_t *PINx,
				 volatile uint8_t Pxn) {

	#ifdef OW_TIMING_PRECISE
		uint8_t ie = SREG & 0b10000000; cli();
	#endif

	uint8_t presence = 0;

	OW_PULL_BUS_LOW(*DDRx, *PORTx, Pxn)
	_delay_us(OW_RESET_DELAY_US);
	OW_RELEASE_BUS(*DDRx, *PORTx, Pxn)

	#ifdef OW_TIMING_APPROXIMATELY
		uint8_t ie = SREG & 0b10000000; cli();
	#endif

	_delay_us(OW_RESET_POST_DELAY_US);
	presence = *PINx;

	#ifdef OW_TIMING_APPROXIMATELY
		if ( ie ) sei();
	#endif

	_delay_us(OW_PRESENCE_POST_DELAY_US);

	#ifdef OW_TIMING_PRECISE
		if ( ie ) sei();
	#endif

	return ((presence >> Pxn) & 0x01);
}

/*
 * This functions emits a one wire write 0-bit signal.
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 */
void ow_write_0(volatile uint8_t *DDRx,
				volatile uint8_t *PORTx,
				uint8_t Pxn) {

	#ifdef OW_TIMING_PRECISE
		uint8_t ie = SREG & 0b10000000; cli();
	#endif

	OW_PULL_BUS_LOW(*DDRx, *PORTx, Pxn)
	_delay_us(OW_WRITE_0_LOW_DELAY_US);
	OW_RELEASE_BUS(*DDRx, *PORTx, Pxn)
	_delay_us(OW_WRITE_0_LOW_POST_DELAY_US);

	#ifdef OW_TIMING_PRECISE
		if ( ie ) sei();
	#endif
}

/*
 * This functions emits a one wire write 1-bit signal.
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 */
void ow_write_1(volatile uint8_t *DDRx,
				volatile uint8_t *PORTx,
				uint8_t Pxn) {

	#if defined(OW_TIMING_APPROXIMATELY) || defined(OW_TIMING_PRECISE)
		uint8_t ie = SREG & 0b10000000; cli();
	#endif

	OW_PULL_BUS_LOW(*DDRx, *PORTx, Pxn)
	_delay_us(OW_WRITE_1_LOW_DELAY_US);
	OW_RELEASE_BUS(*DDRx, *PORTx, Pxn)

	#ifdef OW_TIMING_APPROXIMATELY
		if ( ie ) sei();
	#endif

	_delay_us(OW_WRITE_1_LOW_POST_DELAY_US);

	#ifdef OW_TIMING_PRECISE
		if ( ie ) sei();
	#endif
}

/*
 * This functions emits a one wire read bit signal and records the bit emitted by the slave.
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- PINx: Input I/O register Pin X (e.g. PINB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 *
 * Returns:
 * 		- 0 if the slave sent a 0-bit.
 * 		- 1 if the slave sent a 1-bit.
 */
uint8_t ow_read_bit(volatile uint8_t *DDRx,
					volatile uint8_t *PORTx,
					volatile uint8_t *PINx,
					uint8_t Pxn) {

	uint8_t read = 0;

	#if defined(OW_TIMING_PRECISE) || defined(OW_TIMING_APPROXIMATELY)
		uint8_t ie = SREG & 0b10000000; cli();
	#endif

	OW_PULL_BUS_LOW(*DDRx, *PORTx, Pxn)
	_delay_us(OW_WRITE_1_LOW_DELAY_US);
	OW_RELEASE_BUS(*DDRx, *PORTx, Pxn)

	_delay_us(OW_READ_DEALY_US);
	read = *PINx;

	#ifdef OW_TIMING_APPROXIMATELY
		if ( ie ) sei();
	#endif

	_delay_us(OW_READ_POST_DEALY_US);

	#ifdef OW_TIMING_PRECISE
		if ( ie ) sei();
	#endif

	return ((read >> Pxn) & 0x01);
}

/*
 * This functions sends a complete byte via one wire. LSB first.
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- PINx: Input I/O register Pin X (e.g. PINB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 */
void ow_send_byte(uint8_t byte,
				  volatile uint8_t *DDRx,
				  volatile uint8_t *PORTx,
				  uint8_t Pxn) {

	for ( uint8_t i = 0; i < 8; i++ ) {
		if ( byte & 0x01 ) ow_write_1(DDRx, PORTx, Pxn);
		else ow_write_0(DDRx, PORTx, Pxn);
		byte >>= 1;
	}
}

/*
 * This functions reads a sequence of eight bits.
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- PINx: Input I/O register Pin X (e.g. PINB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 *
 * Returns:
 * 		- Read bits as a byte. First received bit = LSB, last received bit = MSB.
 */
uint8_t ow_read_byte(volatile uint8_t *DDRx,
					 volatile uint8_t *PORTx,
					 volatile uint8_t *PINx,
					 uint8_t Pxn) {

	uint8_t read = 0;
	for ( uint8_t i = 0; i < 8; i++ ) {
		if ( i > 0 ) read >>= 1;
		if ( ow_read_bit(DDRx, PORTx, PINx, Pxn) ) read |= 0b10000000;
	}
	return read;
}

/*
 * Convenience function: sends ROM command "Skip ROM" (0xCC).
 *
 * Parameters:
 * 		- DDRx: Data Direction Register X (e.g. DDRB) where slave is connected.
 * 		- PORTx: Output I/O register Port X (e.g. PORTB) where slave is connected.
 * 		- Pxn: Numerical pin index of port (e.g. PB1) where slave is connected.
 */
void ow_send_rom_skip(volatile uint8_t *DDRx,
					  volatile uint8_t *PORTx,
					  uint8_t Pxn) {

	ow_send_byte(0xCC, DDRx, PORTx, Pxn);
}
