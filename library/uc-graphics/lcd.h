/*
 * So this header file contains the whole library
 * for controlling 128x64 LCD displays.
 *
 * The central responsibility of this library is
 * offering a "set_pixel" call. All graphical
 * operations can use that to realize their functions.
 *
 * This library is a write-only one. It does not need
 * to read from the LCD. The pixel status are kept in
 * a buffer in the RAM. This takes up 1024 bytes.
 *
 * LCDs with following specs are supported:
 * 		- Two LCD segment controllers with each 8 pages consisting
 * 		  of 64 columns of 8 pixels (one byte)
 * 		  (KS0107 / KS0108).
 * 		- On data write the column address increments automatically
 * 		  and overflows after 63 to 0.
 * 		- 8 data pins (DB0 - DB7)
 * 		- CSEL1 and CSEL2 pins (HIGH means selected)
 * 		- Enable pin (action initiated on HIGH)
 * 		- Command/Data pin (Command = LOW, Data = HIGH)
 *
 * Following pins are there probably too,
 * but they won't be used by this library. You have
 * to take care of them, more details in the list:
 * 		- Read/Write pin (Write = LOW, Read = HIGH)
 * 			- As this library is write only, connect
 * 			  this pin to GND or pull it LOW by using
 * 			  a dedicated IO pin
 * 		- Reset pin (LOW pulse resets display)
 * 			- This library does not require the
 * 			  possibility of a hardware reset. So
 * 			  please connect this pin to Vcc or pull
 * 			  a dedicated IO pin HIGH.
 *
 * This display offers two drawing modes:
 * 		- Immediate:
 * 			- In immediate mode, every set_pixel action
 * 			  will immediately be reflected on the
 * 			  RAM buffer and on the LCD.
 * 			  If for example a drawing action consists of
 * 			  multiple set_pixel actions exceeding in sum
 * 			  ~1024 operations, it is recommended to group
 * 			  them using stackable draw blocks. After
 * 			  reaching the end of the first block, the whole
 * 			  buffer will be flushed at once. This way
 * 			  the new pixel data is send once to the LCD.
 * 		- Buffered:
 * 			- In buffered mode, set_pixel actions are only
 * 			  applied to the buffer in RAM. In order to
 * 			  reflect the changes on the LCD, a flush is
 * 			  necessary. This mode is highly recommended,
 * 			  if the LCD data changes quite often and quickly.
 * 			  In combination with a repetitive timed task
 * 			  that flushes the buffer, you have a fixed
 * 			  refresh rate setup. If flush is called and
 * 			  no changes have been made, the data will not
 * 			  be sent twice to the LCD.
 *
 * The library also supports two data transmission modes:
 * 		- parallel:
 * 			- In parallel mode at least the data pins are
 * 			  controlled by just one output register.
 * 			  The control ports are either set together using
 * 			  a shared output register or set individual using
 * 			  different output registers. It is up to you.
 * 		- serial:
 * 			- In serial mode, only 3 pins are necessary. This
 * 			  can be achieved by using shift registers like the
 * 			  75HC595. For each pin you can use either the same
 * 			  or different output registers. You just need to
 * 			  specify them. The reset pin of the shift registers
 * 			  must be set HIGH to disable reset. The OE pin
 * 			  must be driven low in order to activate the out pins.
 * 			  The register and shift clock have to be connected
 * 			  together. That's it. Ah, the 3 pins are:
 * 			  	- clock pin for shift registers
 * 			  	- serial pin for shift register data
 * 			  	- enable pin of the LCD
 * 			  The first shift register represents the data byte.
 * 			  Qa represents the LSB and Qh the MSB.
 * 			  Of the second shift register will only be the first
 * 			  three ones used (CSEL1, CSEL2 & Command/Data). You
 * 			  have to define via macros which of these does correspond
 * 			  to Qa, Qb or Qc by defining the numerical pin number
 * 			  (Qa: 0, Qb: 1, Qc: 2).
 *
 *
 *
 *
 *
 * This library is free software:
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

#ifndef UC_AVR_GRAPHICS_LCD_H_
#define UC_AVR_GRAPHICS_LCD_H_

#include <util/delay.h>
#include <stdint.h>



/* --------------------------------------------------------------------

				Copy & paste macros, values must still be set!

   -------------------------------------------------------------------- */


/*
Choose your drawing mode:

	#define LCD_MODE_BUFFERED
	#define LCD_MODE_IMMEDIATE



Next block for parallel data mode
with common control register:

	#define LCD_DATA_MODE_PARALLEL
	#define LCD_DMP_CONTROL_TOGETHER
	#define LCD_DMP_DDR_DATA
	#define LCD_DMP_PORT_DATA
	#define LCD_DMP_DDR_CONTROL
	#define LCD_DMP_PORT_CONTROL
	#define LCD_DMP_PIN_CSEL1
	#define LCD_DMP_PIN_CSEL2
	#define LCD_DMP_PIN_COMMAND_DATA
	#define LCD_DMP_PIN_ENABLE

Next block for parallel data mode
with different control registers:

	#define LCD_DATA_MODE_PARALLEL
	#define LCD_DMP_DDR_DATA
	#define LCD_DMP_PORT_DATA
	#define LCD_DMP_CONTROL_INDIVIDUAL
	#define LCD_DMP_DDR_CSEL1
	#define LCD_DMP_DDR_CSEL2
	#define LCD_DMP_DDR_COMMAND_DATA
	#define LCD_DMP_DDR_ENABLE
	#define LCD_DMP_PORT_CSEL1
	#define LCD_DMP_PORT_CSEL2
	#define LCD_DMP_PORT_COMMAND_DATA
	#define LCD_DMP_PORT_ENABLE
	#define LCD_DMP_PIN_CSEL1
	#define LCD_DMP_PIN_CSEL2
	#define LCD_DMP_PIN_COMMAND_DATA
	#define LCD_DMP_PIN_ENABLE



Next block for serial data mode:

	#define LCD_DATA_MODE_SERIAL
	#define LCD_DMS_DDR_CLOCK
	#define LCD_DMS_DDR_SERIAL
	#define LCD_DMS_DDR_ENABLE
	#define LCD_DMS_PORT_CLOCK
	#define LCD_DMS_PORT_SERIAL
	#define LCD_DMS_PORT_ENABLE
	#define LCD_DMS_PIN_CLOCK
	#define LCD_DMS_PIN_SERIAL
	#define LCD_DMS_PIN_ENABLE
	#define LCD_DMS_SR_BIT_CSEL1
	#define LCD_DMS_SR_BIT_CSEL2
	#define LCD_DMS_SR_BIT_COMMAND_DATA



These two macros for defining the enable latch times:
	#define LCD_ENABLE_PRE_DELAY_US
	#define LCD_ENABLE_HOLD_DELAY_US
	#define LCD_ENABLE_POST_DELAY_US
*/


/* --------------------------------------------------------------------

						Macro validation section

   -------------------------------------------------------------------- */


/* Check for necessary macros */
#if !defined(LCD_MODE_BUFFERED) && !defined(LCD_MODE_IMMEDIATE)
#error No drawing mode has been specified!
#endif

#if !defined(LCD_DATA_MODE_PARALLEL) && !defined(LCD_DATA_MODE_SERIAL)
#error No data mode has been specified!
#endif

#if defined(LCD_MODE_BUFFERED) && defined(LCD_MODE_IMMEDIATE)
#error Both immediate and buffered drawing mode have been specified, but only one at a time is allowed.

#endif

#if defined(LCD_DATA_MODE_SERIAL) && defined(LCD_DATA_MODE_PARALLEL)
#error Both serial and parallel data mode have been specified, but only one at a time is allowed.
#endif

/* Validate configuration of parallel data mode */
#ifdef LCD_DATA_MODE_PARALLEL
	/*
	 * You need to specify following macros:
	 * 		- LCD_DMP_DDR_DATA
	 * 		- LCD_DMP_PORT_DATA
	 */

	#ifndef LCD_DMP_DDR_DATA
	#error In data mode parallel the macro LCD_DMP_ is required.
	#endif

	#ifndef LCD_DMP_PORT_DATA
	#error In data mode parallel the macro LCD_DMP_PORT_DATA is required.
	#endif

	/*
	 * If you want to control the control pins the same output register,
	 * define the macro LCD_DMP_CONTROL_TOGETHER, otherwise use
	 * LCD_DMP_CONTROL_INDIVIDUAL.
	 */
	#if !defined(LCD_DMP_CONTROL_TOGETHER) && !defined(LCD_DMP_CONTROL_INDIVIDUAL)
	#error Neither LCD_DMP_CONTROL_TOGETHER nor LCD_DMP_CONTROL_INDIVIDUAL have been specified.
	#endif

	#ifdef LCD_DMP_CONTROL_TOGETHER
		/*
		 * If control pins shall be set together using on output register
		 * you must specify following macros:
		 * 		- LCD_DMP_DDR_CONTROL
		 * 		- LCD_DMP_PORT_CONTROL
		 * 		- LCD_DMP_PIN_CSEL1
		 * 		- LCD_DMP_PIN_CSEL2
		 * 		- LCD_DMP_PIN_COMMAND_DATA
		 * 		- LCD_DMP_PIN_ENABLE
		 */

		#ifndef LCD_DMP_DDR_CONTROL
		#error In data mode parallel the macro LCD_DMP_DDR_CONTROL is required.
		#endif

		#ifndef LCD_DMP_PORT_CONTROL
		#error In data mode parallel the macro LCD_DMP_PORT_CONTROL is required.
		#endif

		#ifndef LCD_DMP_PIN_CSEL1
		#error In data mode parallel the macro LCD_DMP_PIN_CSEL1 is required.
		#endif

		#ifndef LCD_DMP_PIN_CSEL2
		#error In data mode parallel the macro LCD_DMP_PIN_CSEL2 is required.
		#endif

		#ifndef LCD_DMP_PIN_COMMAND_DATA
		#error In data mode parallel the macro LCD_DMP_PIN_COMMAND_DATA is required.
		#endif

		#ifndef LCD_DMP_PIN_ENABLE
		#error In data mode parallel the macro LCD_DMP_PIN_ENABLE is required.
		#endif
	#endif //End of LCD_DMP_CONTROL_TOGETHER

	#ifdef LCD_DMP_CONTROL_INDIVIDUAL
		/*
		 * But if you want to control the data pins using different output
		 * registers, specify following macros:
		 * 		- LCD_DMP_DDR_CSEL1
		 * 		- LCD_DMP_DDR_CSEL2
		 * 		- LCD_DMP_DDR_COMMAND_DATA
		 * 		- LCD_DMP_DDR_ENABLE
		 *
		 * 		- LCD_DMP_PORT_CSEL1
		 * 		- LCD_DMP_PORT_CSEL2
		 * 		- LCD_DMP_PORT_COMMAND_DATA
		 * 		- LCD_DMP_PORT_ENABLE
		 *
		 * 		- LCD_DMP_PIN_CSEL1
		 * 		- LCD_DMP_PIN_CSEL2
		 * 		- LCD_DMP_PIN_COMMAND_DATA
		 * 		- LCD_DMP_PIN_ENABLE
		 */

		#ifndef LCD_DMP_DDR_CSEL1
		#error In data mode parallel the macro LCD_DMP_DDR_CSEL1 is required.
		#endif

		#ifndef LCD_DMP_DDR_CSEL2
		#error In data mode parallel the macro LCD_DMP_DDR_CSEL2 is required.
		#endif

		#ifndef LCD_DMP_DDR_COMMAND_DATA
		#error In data mode parallel the macro LCD_DMP_DDR_COMMAND_DATA is required.
		#endif

		#ifndef LCD_DMP_DDR_ENABLE
		#error In data mode parallel the macro LCD_DMP_DDR_ENABLE is required.
		#endif

		#ifndef LCD_DMP_PORT_CSEL1
		#error In data mode parallel the macro LCD_DMP_PORT_CSEL1 is required.
		#endif

		#ifndef LCD_DMP_PORT_CSEL2
		#error In data mode parallel the macro LCD_DMP_PORT_CSEL2 is required.
		#endif

		#ifndef LCD_DMP_PORT_COMMAND_DATA
		#error In data mode parallel the macro LCD_DMP_PORT_COMMAND_DATA is required.
		#endif

		#ifndef LCD_DMP_PORT_ENABLE
		#error In data mode parallel the macro LCD_DMP_PORT_ENABLE is required.
		#endif

		#ifndef LCD_DMP_PIN_CSEL1
		#error In data mode parallel the macro LCD_DMP_PIN_CSEL1 is required.
		#endif

		#ifndef LCD_DMP_PIN_CSEL2
		#error In data mode parallel the macro LCD_DMP_PIN_CSEL2 is required.
		#endif

		#ifndef LCD_DMP_PIN_COMMAND_DATA
		#error In data mode parallel the macro LCD_DMP_PIN_COMMAND_DATA is required.
		#endif

		#ifndef LCD_DMP_PIN_ENABLE
		#error In data mode parallel the macro LCD_DMP_PIN_ENABLE is required.
		#endif

	#endif //End of LCD_DMP_CONTROL_INDIVIDUAL
#endif //End of LCD_MODE_PARALLEL


/* Validate configuration for serial data mode */
#ifdef LCD_DATA_MODE_SERIAL
	/*
	 * You need to specify following macros:
	 * 		- LCD_DMS_DDR_CLOCK
	 * 		- LCD_DMS_DDR_SERIAL
	 * 		- LCD_DMS_DDR_ENABLE
	 *
	 * 		- LCD_DMS_PORT_CLOCK
	 * 		- LCD_DMS_PORT_SERIAL
	 * 		- LCD_DMS_PORT ENABLE
	 *
	 * 		- LCD_DMS_PIN_CLOCK
	 * 		- LCD_DMS_PIN_SERIAL
	 * 		- LCD_DMS_PIN_ENABLE
	 *
	 * 		- LCD_DMS_SR_BIT_CSEL1: bit/pin of 2nd shift register to use for CSEL1 (0-3)
	 * 		- LCD_DMS_SR_BIT_CSEL2: bit/pin of 2nd shift register to use for CSEL2 (0-3)
	 * 		- LCD_DMS_SR_BIT_COMMAND_DATA: bit/pin of 2nd shift register to use for Command/Data (0-3)
	 */
	#ifndef LCD_DMS_DDR_CLOCK
	#error In serial mode parallel the macro LCD_DMS_DDR_CLOCK is required.
	#endif

	#ifndef LCD_DMS_DDR_SERIAL
	#error In data mode serial the macro LCD_DMS_DDR_SERIAL is required.
	#endif

	#ifndef LCD_DMS_DDR_ENABLE
	#error In data mode serial the macro LCD_DMS_DDR_ENABLE is required.
	#endif

	#ifndef LCD_DMS_PORT_CLOCK
	#error In data mode serial the macro LCD_DMS_PORT_CLOCK is required.
	#endif

	#ifndef LCD_DMS_PORT_SERIAL
	#error In data mode serial the macro LCD_DMS_PORT_SERIAL is required.
	#endif

	#ifndef LCD_DMS_PORT_ENABLE
	#error In data mode serial the macro LCD_DMS_PORT_ENABLE is required.
	#endif

	#ifndef LCD_DMS_PIN_CLOCK
	#error In data mode serial the macro LCD_DMS_PIN_CLOCK is required.
	#endif

	#ifndef LCD_DMS_PIN_SERIAL
	#error In data mode serial the macro LCD_DMS_PIN_SERIAL is required.
	#endif

	#ifndef LCD_DMS_PIN_ENABLE
	#error In data mode serial the macro LCD_DMS_PIN_ENABLE is required.
	#endif

	#ifndef LCD_DMS_SR_BIT_CSEL1
	#error In data mode serial the macro LCD_DMS_SR_BIT_CSEL1 is required.
	#endif

	#ifndef LCD_DMS_SR_BIT_CSEL2
	#error In data mode serial the macro LCD_DMS_SR_BIT_CSEL2 is required.
	#endif

	#ifndef LCD_DMS_SR_BIT_COMMAND_DATA
	#error In data mode serial the macro LCD_DMS_SR_BIT_COMMAND_DATA is required.
	#endif

	#if defined(LCD_DMS_SR_BIT_COMMAND_DATA) && defined(LCD_DMS_SR_BIT_CSEL2) && defined(LCD_DMS_SR_BIT_COMMAND_DATA)
		#if !(3 == (LCD_DMS_SR_BIT_CSEL1 + LCD_DMS_SR_BIT_CSEL2 + LCD_DMS_SR_BIT_COMMAND_DATA))
		#error LCD_DMS_SR_BIT_ macros are not defined properly. There must be one having the value 0, one having the value 1 and the last one must have the value 2 (sum 3). Please check your configuration.
		#endif
	#endif

	#define LCD_TURN_ON_CLOCK LCD_DMS_PORT_CLOCK |= (1 << LCD_DMS_PIN_CLOCK);
	#define LCD_TURN_OFF_CLOCK LCD_DMS_PORT_CLOCK &= ~(1 << LCD_DMS_PIN_CLOCK);

	#define LCD_TURN_ON_SERIAL LCD_DMS_PORT_SERIAL |= (1 << LCD_DMS_PIN_SERIAL);
	#define LCD_TURN_OFF_SERIAL LCD_DMS_PORT_SERIAL &= ~(1 << LCD_DMS_PIN_SERIAL);

	#define LCD_TURN_ON_ENABLE LCD_DMS_PORT_ENABLE |= (1 << LCD_DMS_PIN_ENABLE);
	#define LCD_TURN_OFF_ENABLE LCD_DMS_PORT_ENABLE &= ~(1 << LCD_DMS_PIN_ENABLE);

	#define LCD_TOGGLE_CLOCK LCD_TURN_ON_CLOCK; LCD_TURN_OFF_CLOCK;
#endif


/*
 * Check for enable delay macros, otherwise introduce default values.
 *
 * The post delay is often the critical point. That's why here a default
 * will be defined if not done before.
 */
#ifndef LCD_ENABLE_POST_DELAY_US
#define LCD_ENABLE_POST_DELAY_US 1.0
#endif


/* --------------------------------------------------------------------

							Variables section

   -------------------------------------------------------------------- */



/* Shared variables */

/* Buffer for LCD/pixel data */
uint8_t uc_lcd_buffer[1024];
uint8_t uc_lcd_inverted = 0;


/* Introduce variables for immediate drawing mode */
#ifdef LCD_MODE_IMMEDIATE
/* They are used to avoid setting column or page multiple time,
 * even if drawing action is going from left to right. */
uint8_t uc_lcd_current_column_chip1;
uint8_t uc_lcd_current_column_chip2;

uint8_t uc_lcd_current_page_chip1;
uint8_t uc_lcd_current_page_chip2;

uint8_t uc_lcd_grouped_pixel_actions_level;
#endif


/* Introduce variables for buffered drawing mode */
#ifdef LCD_MODE_BUFFERED
/* Flag indicating if a data changed occurred (if a flush is really necessary) */
uint8_t uc_lcd_data_changed;
#endif



/* --------------------------------------------------------------------

							Section for methods

   -------------------------------------------------------------------- */

#ifdef LCD_DATA_MODE_SERIAL
void uc_lcd_shift_out(uint8_t instructions, uint8_t data) {
	//Three instruction bits
	if ( instructions & 0b00000100 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	instructions <<= 1;
	if ( instructions & 0b00000100 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	instructions <<= 1;
	if ( instructions & 0b00000100 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK

	//8 data bits
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK
	data <<= 1;
	if ( data & 0b10000000 ) LCD_TURN_ON_SERIAL else LCD_TURN_OFF_SERIAL
	LCD_TOGGLE_CLOCK

	//Additional clock pulse to flush levels from shift register to out register
	LCD_TOGGLE_CLOCK
}
#endif

void uc_lcd_send(uint8_t csel1, uint8_t csel2, uint8_t command_data, uint8_t data) {
	#ifdef LCD_DATA_MODE_PARALLEL
		#ifdef LCD_DMP_CONTROL_TOGETHER
			LCD_DMP_PORT_DATA = data;
			LCD_DMP_PORT_CONTROL |= (csel1 << LCD_DMP_PIN_CSEL1) | (csel2 << LCD_DMP_PIN_CSEL2) | (command_data << LCD_DMP_PIN_COMMAND_DATA);

			#if LCD_ENABLE_PRE_DELAY_US
				_delay_us(LCD_ENABLE_PRE_DELAY_US);
			#endif

			LCD_DMP_PORT_CONTROL |= (1 << LCD_DMP_PIN_ENABLE);

			#if LCD_ENABLE_HOLD_DELAY_US
				_delay_us(LCD_ENABLE_HOLD_DELAY_US);
			#endif

			LCD_DMP_PORT_CONTROL &= ~(1 << LCD_DMP_PIN_ENABLE);
			_delay_us(LCD_ENABLE_POST_DELAY_US);

		#endif //End of control together parallel mode

		#ifdef LCD_DMP_CONTROL_INDIVIDUAL
			LCD_DMP_PORT_DATA = data;
			LCD_DMP_PORT_CSEL1 |= (csel1 << LCD_DMP_PIN_CSEL1);
			LCD_DMP_PORT_CSEL2 |= (csel1 << LCD_DMP_PIN_CSEL2);
			LCD_DMP_PORT_COMMAND_DATA |= (csel1 << LCD_DMP_PIN_COMMAND_DATA);

			#if LCD_ENABLE_PRE_DELAY_US
				_delay_us(LCD_ENABLE_PRE_DELAY_US);
			#endif

			LCD_DMP_PORT_ENABLE |= (1 << LCD_DMP_PIN_ENABLE);

			#if LCD_ENABLE_HOLD_DELAY_US
				_delay_us(LCD_ENABLE_HOLD_DELAY_US);
			#endif

			LCD_DMP_PORT_ENABLE &= ~(1 << LCD_DMP_PIN_ENABLE);
			_delay_us(LCD_ENABLE_POST_DELAY_US);

		#endif //End of individual control parallel mode

	#endif //End of parallel data mode


	#ifdef LCD_DATA_MODE_SERIAL
		uc_lcd_shift_out(((csel1 << LCD_DMS_SR_BIT_CSEL1) |
						  (csel2 << LCD_DMS_SR_BIT_CSEL2) |
						  (command_data << LCD_DMS_SR_BIT_COMMAND_DATA)),
						 data);

		#if LCD_ENABLE_PRE_DELAY_US
			_delay_us(LCD_ENABLE_PRE_DELAY_US);
		#endif

		LCD_DMS_PORT_ENABLE |= (1 << LCD_DMS_PIN_ENABLE);

		#if LCD_ENABLE_HOLD_DELAY_US
			_delay_us(LCD_ENABLE_HOLD_DELAY_US);
		#endif

		LCD_DMS_PORT_ENABLE &= ~(1 << LCD_DMS_PIN_ENABLE);
		_delay_us(LCD_ENABLE_POST_DELAY_US);
	#endif //End of serial data mode
}

/*
 * Turns both segments on.
 */
void uc_lcd_turn_on() {
	uc_lcd_send(1, 1, 0, 0x3F);
}

/*
 * Turns both segments off.
 */
void uc_lcd_turn_off() {
	uc_lcd_send(1, 1, 0, 0x3E);
}

/*
 * Sets the start line of both segments.
 *
 * Params:
 * 		- uint8_t startline: y coordinate which represents the first bit of page 0 and column 0.
 */
void uc_lcd_set_startline(uint8_t startline) {
	uc_lcd_send(1, 1, 0, 0b11000000 | startline);
}

/*
 * Sets the page of the first chip. In immediate mode the new page will be saved
 * in a separate variable.
 *
 * Params:
 * 		- uint8_t page: index of vertical set of 8 pixels.
 */
void uc_lcd_set_page_chip_1(uint8_t page) {
	#ifdef LCD_MODE_IMMEDIATE
	if ( page != uc_lcd_current_page_chip1 ) {
	#endif

		uc_lcd_send(1, 0, 0, 0b10111000 | (page & 0b00000111));

		#ifdef LCD_MODE_IMMEDIATE
			uc_lcd_current_page_chip1 = page;
		#endif

	#ifdef LCD_MODE_IMMEDIATE
	}
	#endif
}

/*
 * Sets the page of the second chip. In immediate mode the new page will be saved
 * in a separate variable.
 *
 * Params:
 * 		- uint8_t page: index of vertical set of 8 pixels.
 */
void uc_lcd_set_page_chip_2(uint8_t page) {
	#ifdef LCD_MODE_IMMEDIATE
	if ( page != uc_lcd_current_page_chip2 ) {
	#endif

		uc_lcd_send(0, 1, 0, 0b10111000 | (page & 0b00000111));

		#ifdef LCD_MODE_IMMEDIATE
			uc_lcd_current_page_chip2 = page;
		#endif

	#ifdef LCD_MODE_IMMEDIATE
	}
	#endif
}

/*
 * Sets the column of the first chip. In immediate mode the new column will be saved
 * in a separate variable.
 *
 * Params:
 * 		- uint8_t column: index of horizontal coordinate.
 */
void uc_lcd_set_column_chip_1(uint8_t column) {
	#ifdef LCD_MODE_IMMEDIATE
	if ( column != uc_lcd_current_page_chip1 ) {
	#endif

		uc_lcd_send(1, 0, 0, 0b01000000 | (column & 0b00111111));

		#ifdef LCD_MODE_IMMEDIATE
			uc_lcd_current_column_chip1 = column;
		#endif

	#ifdef LCD_MODE_IMMEDIATE
	}
	#endif
}

/*
 * Sets the column of the second chip. In immediate mode the new column will be saved
 * in a separate variable.
 *
 * Params:
 * 		- uint8_t column: index of horizontal coordinate.
 */
void uc_lcd_set_column_chip_2(uint8_t column) {
	#ifdef LCD_MODE_IMMEDIATE
	if ( column != uc_lcd_current_page_chip2 ) {
	#endif

		uc_lcd_send(0, 1, 0, 0b01000000 | (column & 0b00111111));

		#ifdef LCD_MODE_IMMEDIATE
			uc_lcd_current_column_chip2 = column;
		#endif

	#ifdef LCD_MODE_IMMEDIATE
	}
	#endif
}

/*
 * Writes a data byte using the display data mode of the LCD.
 * Only writes to chip 1.
 *
 * Params:
 * 		- uint8_t data: 8 pixels to be written at location identified by page and column.
 */
void uc_lcd_write_chip1(uint8_t data) {
	uc_lcd_send(1, 0, 1, data);

	#ifdef LCD_MODE_IMMEDIATE
		uc_lcd_current_column_chip1 = (uc_lcd_current_column_chip1 + 1) % 64;
	#endif
}

/*
 * Writes a data byte using the display data mode of the LCD.
 *
 * Params:
 * 		- uint8_t data: 8 pixels to be written at location identified by page and column.
 */
void uc_lcd_write_chip2(uint8_t data) {
	uc_lcd_send(0, 1, 1, data);

	#ifdef LCD_MODE_IMMEDIATE
		uc_lcd_current_column_chip2 = (uc_lcd_current_column_chip2 + 1) % 64;
	#endif
}

/*
 * Writes a data byte using the display data mode.
 *
 * Params:
 * 		- uint8_t csel1: 1 if data shall be sent to chip1
 * 		- uint8_t csel2: 1 if data shall be sent to chip2
 * 		- uint8_t data: 8 pixels to be written at location identified by page and column.
 */
void uc_lcd_write(uint8_t csel1, uint8_t csel2, uint8_t data) {
	uc_lcd_send(csel1, csel2, 1, data);

	#ifdef LCD_MODE_IMMEDIATE
		if ( csel1 ) uc_lcd_current_column_chip1 = (uc_lcd_current_column_chip1 + 1) % 64;
		if ( csel2 ) uc_lcd_current_column_chip2 = (uc_lcd_current_column_chip2 + 1) % 64;
	#endif
}


/*
 * Sends the complete buffer to the LCD. Rewrites the whole screen.
 * Does not check for changes. It's recommended to rather use flush function
 * in buffered mode.
 */
void uc_lcd_send_buffer_to_lcd() {

	//I use uc_lcd_send() instead of write functions to avoid keeping track of
	//of the current columns because after all write commands that follow,
	//the column register of the LCD points to 0 again ... so keeping
	//track of column changes is unnecessary.

	uint16_t index = 0;

	uc_lcd_set_column_chip_1(0);
	for ( uint8_t page = 0; page < 8; page++ ) {
		uc_lcd_set_page_chip_1(page);
		for ( uint8_t column = 0; column < 64; column++ ) {
			uc_lcd_send(1, 0, 1, uc_lcd_buffer[index++]);
		}
	}

	uc_lcd_set_column_chip_2(0);
	for ( uint8_t page = 0; page < 8; page++ ) {
		uc_lcd_set_page_chip_2(page);
		for ( uint8_t column = 0; column < 64; column++ ) {
			uc_lcd_send(0, 1, 1, uc_lcd_buffer[index++]);
		}
	}

	//Reset page
	//Columns should be at 0 again
	uc_lcd_set_page_chip_1(0);
	uc_lcd_set_page_chip_2(0);
}

#ifdef LCD_MODE_BUFFERED
/*
 * Flushes the buffer: if changes have been made on the buffer, the complete
 * buffer will be sent to the LCD, otherwise nothing happens.
 */
void uc_lcd_flush() {
	if ( uc_lcd_data_changed ) {
		uc_lcd_send_buffer_to_lcd();

		uc_lcd_data_changed = 0;
	}
}
#endif

/*
 * Retrieves the inverted status of the LCD graphics.
 *
 * Returns:
 * 		- uint8_t:	0 if not inverted, 1 if inverted.
 */
uint8_t uc_lcd_is_inverted() {
	return uc_lcd_inverted;
}

/*
 * Set the status of the inverted mode. If mode changed the necessary buffer
 * inverting takes place. In drawing mode immediate the changes will be instantly
 * flushed to the LCD.
 *
 * Params:
 * 		- uint8_t invert:	0 if graphics shall not be inverted, 1 to invert.
 */
void uc_lcd_set_inverted(uint8_t invert) {
	if ( invert != uc_lcd_inverted ) {
		for ( uint16_t i = 0; i < 1024; i++ ) {
			uc_lcd_buffer[i] = ~uc_lcd_buffer[i];
		}

		uc_lcd_inverted = invert;

		#ifdef LCD_MODE_IMMEDIATE
			uc_lcd_send_buffer_to_lcd();
		#endif

		#ifdef LCD_MODE_BUFFERED
			uc_lcd_data_changed = 1;
		#endif
	}
}

/*
 * Clears the display buffer (sets every bit to zero). In drawing mode
 * immediate the buffer will afterwards be flushed to the LCD.
 */
void uc_lcd_clear() {
	if ( uc_lcd_inverted ) {
		for ( uint16_t i = 0; i < 1024; i++ ) {
			uc_lcd_buffer[i] = 0xFF;
		}
	} else {
		for ( uint16_t i = 0; i < 1024; i++ ) {
			uc_lcd_buffer[i] = 0x00;
		}
	}

	#ifdef LCD_MODE_BUFFERED
		uc_lcd_data_changed = 1;
	#endif

	#ifdef LCD_MODE_IMMEDIATE
		uc_lcd_send_buffer_to_lcd();
	#endif
}

/*
 * Fill the display buffer (sets every bit to one). In drawing mode
 * immediate the buffer will afterwards be flushed to the LCD.
 */
void uc_lcd_fill() {
	if ( uc_lcd_inverted ) {
		for ( uint16_t i = 0; i < 1024; i++ ) {
			uc_lcd_buffer[i] = 0x00;
		}
	} else {
		for ( uint16_t i = 0; i < 1024; i++ ) {
			uc_lcd_buffer[i] = 0xFF;
		}
	}
	#ifdef LCD_MODE_BUFFERED
		uc_lcd_data_changed = 1;
	#endif

	#ifdef LCD_MODE_IMMEDIATE
		uc_lcd_send_buffer_to_lcd();
	#endif
}

#ifdef LCD_MODE_IMMEDIATE
/*
 * Used to enter a section of multiple set_pixel function calls in
 * immediate drawing mode without to send data at each pixel change.
 * This function can be called mutliple times. It increases a counter
 * and blocks by this data transmission within the set_pixel function.
 *
 * If the counter reaches zero again while calling the counterpart function
 * "leave", the complete display buffer will be send to the LCD.
 */
void uc_lcd_enter_grouped_pixel_changes() {
	uc_lcd_grouped_pixel_actions_level++;
}

/*
 * Used to leave grouped pixel change sections. It decreases an internal counter.
 * On zero, the complete display buffer will be send to the LCD. For more details,
 * see "enter"-function.
 */
void uc_lcd_leave_grouped_pixel_changes() {
	uc_lcd_grouped_pixel_actions_level--;

	if ( uc_lcd_grouped_pixel_actions_level == 0 ) {
		uc_lcd_send_buffer_to_lcd();
	}
}
#endif

/**
 * Set a certain pixel value.
 *
 * Params:
 * 		- x: x coordinate of pixel.
 * 		- y: y coordinate of pixel.
 * 		- pixel: pixel value (0 or 1).
 */
void uc_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t pixel) {
	if ( x > 127 ) return;
	if ( y > 63 ) return;

	uint8_t chip_1 = x < 64;
	uint8_t column = x % 64;
	uint8_t page = y/8;
	uint8_t bit = y%8;

	uint16_t buffer_index = column + (page * 64);
	if ( !chip_1 ) buffer_index += 512;

	//Retrieve data, modify pixel, apply inverting if necessary,
	//send data in immediate mode, update changed flag in buffered mode.
	uint8_t data = uc_lcd_buffer[buffer_index];

	if ( uc_lcd_inverted ) {
		if ( pixel ) data &= ~(1 << (bit));
		else data |= (1 << (bit));
	} else {
		if ( pixel ) data |= (1 << (bit));
		else data &= ~(1 << (bit));
	}

	uc_lcd_buffer[buffer_index] = data;


	#ifdef LCD_MODE_BUFFERED
		uc_lcd_data_changed = 1;
	#endif

	#ifdef LCD_MODE_IMMEDIATE
		if ( uc_lcd_grouped_pixel_actions_level == 0 ) {
			if ( chip_1 ) {
				uc_lcd_set_page_chip_1(page);
				uc_lcd_set_column_chip_1(column);
				uc_lcd_write_chip1(data);
			} else {
				uc_lcd_set_page_chip_2(page);
				uc_lcd_set_column_chip_2(column);
				uc_lcd_write_chip2(data);
			}
		}
	#endif
}

/*
 * Resets LCD setup. Resets startline, page and column to zero and clears screen.
 */
void uc_lcd_reset() {
	uc_lcd_set_inverted(0);
	uc_lcd_set_startline(0);
	uc_lcd_set_page_chip_1(0);
	uc_lcd_set_page_chip_2(0);
	uc_lcd_set_column_chip_1(0);
	uc_lcd_set_column_chip_2(0);

	#ifdef LCD_MODE_IMMEDIATE
		uc_lcd_grouped_pixel_actions_level = 0;
	#endif

	uc_lcd_clear();

	#ifdef LCD_MODE_BUFFERED
		uc_lcd_flush();
	#endif
}

/*
 * Initializes the data directions registers, resets (clears) screen,
 * sets startline, page and column to zero. In buffered mode, buffer will
 * be reset.
 */
void uc_lcd_init() {
	/* Set data direction registers */
	#ifdef LCD_DATA_MODE_PARALLEL
		#ifdef LCD_DMP_CONTROL_TOGETHER
			LCD_DMP_DDR_DATA 	= 0xFF;
			LCD_DMP_DDR_CONTROL |= (1 << LCD_DMP_PIN_CSEL1) |
								   (1 << LCD_DMP_PIN_CSEL2) |
								   (1 << LCD_DMP_PIN_COMMAND_DATA) |
								   (1 << LCD_DMP_PIN_ENABLE);
		#endif

		#ifdef LCD_DMP_CONTROL_INDIVIDUAL
			LCD_DMP_DDR_DATA 			= 0xFF;
			LCD_DMP_DDR_CSEL1 			|= (1 << LCD_DMP_PIN_CSEL1);
			LCD_DMP_DDR_CSEL2 			|= (1 << LCD_DMP_PIN_CSEL2);
			LCD_DMP_DDR_COMMAND_DATA	|= (1 << LCD_DMP_PIN_COMMAND_DATA);
			LCD_DMP_DDR_ENABLE 			|= (1 << LCD_DMP_PIN_ENABLE);
		#endif
	#endif

	#ifdef LCD_DATA_MODE_SERIAL
		LCD_DMS_DDR_CLOCK	|= (1 << LCD_DMS_PIN_CLOCK);
		LCD_DMS_DDR_SERIAL	|= (1 << LCD_DMS_PIN_SERIAL);
		LCD_DMS_DDR_ENABLE	|= (1 << LCD_DMS_PIN_ENABLE);
	#endif

	//Set output all output levels to LOW
	#ifdef LCD_DATA_MODE_PARALLEL
		LCD_DMP_PORT_DATA = 0x00;

		#ifdef LCD_DMP_CONTROL_TOGETHER
			LCD_DMP_PORT_CONTROL &= ~((1 << LCD_DMP_PIN_CSEL1) |
									  (1 << LCD_DMP_PIN_CSEL2) |
									  (1 << LCD_DMP_PIN_COMMAND_DATA) |
									  (1 << LCD_DMP_PIN_ENABLE)
									 );
		#endif

		#ifdef LCD_DMP_CONTROL_INDIVIDUAL

			LCD_DMP_PORT_CSEL1 &= ~(1 << LCD_DMP_PIN_CSEL1);
			LCD_DMP_PORT_CSEL2 &= ~(1 << LCD_DMP_PIN_CSEL2);
			LCD_DMP_PORT_COMMAND_DATA &= ~(1 << LCD_DMP_PIN_COMMAND_DATA);
			LCD_DMP_PORT_ENABLE &= ~(1 << LCD_DMP_PIN_ENABLE);
		#endif
	#endif

	#ifdef LCD_DATA_MODE_SERIAL
		LCD_DMS_PORT_CLOCK &= ~(1 << LCD_DMS_PIN_CLOCK);
		LCD_DMS_PORT_SERIAL &= ~(1 << LCD_DMS_PIN_SERIAL);
		LCD_DMS_PORT_ENABLE &= ~(1 << LCD_DMS_PIN_ENABLE);
	#endif

	uc_lcd_reset();
}

/*
 * Mandatory API calls:
 * 		- void		LCD_API_INIT()
 * 		- void		LCD_API_RESET()
 * 		- void		LCD_API_CLEAR()
 * 		- void		LCD_API_FILL()
 * 		- uint8_t	LCD_API_IS_INVERTED()
 * 		- void		LCD_API_SET_INVERTED(uint8_t inverted) --> good for blue/white displays
 * 		- void		LCD_API_SET_PIXEL(uint8_t x, uint8_t y, uint8_t pixel)
 */

#define LCD_API_WIDTH					128
#define LCD_API_HEIGHT					64
#define LCD_API_INIT() 					uc_lcd_init()
#define LCD_API_RESET() 				uc_lcd_reset()
#define LCD_API_CLEAR() 				uc_lcd_clear()
#define LCD_API_FILL() 					uc_lcd_fill()
#define LCD_API_IS_INVERTED() 			uc_lcd_is_inverted()
#define LCD_API_SET_INVERTED(invert) 	uc_lcd_set_inverted(invert)
#define LCD_API_SET_PIXEL(x, y, pixel) 	uc_lcd_set_pixel(x, y, pixel)

#ifdef LCD_MODE_BUFFERED
	#define LCD_API_FLUSH() 			uc_lcd_flush()
#endif

#ifdef LCD_MODE_IMMEDIATE
	#define LCD_API_ENTER_GROUPED_PIXEL_CHANGES() uc_lcd_enter_grouped_pixel_changes()
	#define LCD_API_LEAVE_GROUPED_PIXEL_CHANGES() uc_lcd_leave_grouped_pixel_changes()
#endif



#endif /* UC_GRAPHICS_LCD_H_ */
