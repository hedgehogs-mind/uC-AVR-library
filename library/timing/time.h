/* This super tiny piece of software is used to keep
 * track of the time that has passed since the startup.
 *
 * Has only been tested on the ATMega328P!
 *
 *
 *
 * DO NOT FORGET calling sei()!
 *
 *
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
 *
 */

#ifndef UC_TIMING_TIME_H_
#define UC_TIMING_TIME_H_

#if !defined (__AVR_ATmega328P__)
#error This software has not been tested on any other AVR except the ATMega328P! Normally every AVR has the 8-bit Timer0. So it should work! You may adapt it to your needs.
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#define US_PER_COMPARE_A (1000000ULL * 64ULL * 128ULL)/F_CPU

volatile uint64_t time_us = 0ULL;

ISR(TIMER0_COMPA_vect) {
	time_us += US_PER_COMPARE_A;
}

inline uint64_t Time_Get() {
	return time_us;
}

/* Calling this method is essential to make it work properly.
 *
 * DO NOT FORGET calling sei() afterwards!
 *
 */
void Time_Init() {
	TIMSK0 |= (1 << OCIE0A); //Enable interrupt on timer count overflow
	OCR0A = 128; //Set compare to 128
	TCCR0A |= (1 << WGM01); //CTC mode -> 128 is max counter

	// now per compare interrupt can be added us:
	//  = 1.000.000 * PRESCALER * COMPARE / F_CPU

	TCCR0B |= (1 << CS01) | (1 << CS00); //prescaler to clk/64, starts timer
}

#endif
