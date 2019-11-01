/**
 * This library can draw fonts encoded as byte arrays
 * by the µC-Graphics-Tools.
 *
 * The first byte is a settings byte.
 *  - bit 7 == 1: format of byte array: simple byte chain with fixed with (bc)
 *  - bit 6 == 1: format of byte array: byte chain with search due to emtpy chars (bcs)
 * 	- bit 5 == 1: left to right, top to bottom (hv)
 * 	- bit 4 == 1: top to bottom, left to right (vh)
 *
 * 	The second and third byte store the width and
 * 	height of an font character.
 *
 *
 * 	Byte 3 & up store the pixels stored LSB and zero padded consecutively.
 * 	For more details or to encode your own font, visit the
 * 	µC-Graphics-Tools project:
 *
 * 	https://github.com/hedgehogs-mind/uc-graphics-tools
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
 */

#ifndef UC_AVR_GRAPHICS_FONTS_H_
#define UC_AVR_GRAPHICS_FONTS_H_

#ifndef LCD_API_SET_PIXEL
#error "µC-Graphics fonts library does require LCD_API_SET_PIXEL makro to be set, in order to draw an image."
#endif

#include <avr/pgmspace.h>
#include "../graphics/graphics.h"

#define UC_FONTS_SETTINGS_BIT_BC_MASK	(1 << 7)
#define UC_FONTS_SETTINGS_BIT_BCS_MASK 	(1 << 6)
#define UC_FONTS_SETTINGS_BIT_HV_MASK	(1 << 5)
#define UC_FONTS_SETTINGS_BIT_VH_MASK	(1 << 4)

/**
 * Retrieves settings byte of font stored in progmem.
 *
 * params:
 * 		- progmem_font: font stored in progmem
 *
 * returns: settings byte of font.
 */
uint8_t uc_fonts_get_settings(const uint8_t *progmem_font) {
	return pgm_read_byte(&progmem_font[0]);
}

/**
 * Retrieves height of characters of font stored in progmem.
 *
 * params:
 * 		- progmem_font: font stored in progmem
 *
 * returns: width of characters of font.
 */
uint8_t uc_fonts_get_char_width(const uint8_t *progmem_font) {
	return pgm_read_byte(&progmem_font[1]);
}

/**
 * Retrieves width of characters of font stored in progmem.
 *
 * params:
 * 		- progmem_font: font stored in progmem
 *
 * returns: height of characters of font.
 */
uint8_t uc_fonts_get_char_height(const uint8_t *progmem_font) {
	return pgm_read_byte(&progmem_font[2]);
}

/**
 * Retrieve the amount of byte a character which is not empty occupies
 * in a byte array/font stored in progmem.
 *
 * params:
 * 		- progmem_font: font stored in progmem
 *
 * returns: amount of bytes including hasPixelsFlag-byte a character occupies.
 */
uint16_t uc_fonts_get_bytes_per_non_empty_char(const uint8_t *progmem_font) {
	uint8_t width = pgm_read_byte(&progmem_font[1]);
	uint8_t height = pgm_read_byte(&progmem_font[2]);
	uint16_t wh = width*height;
	return 1 + (wh/8) + ((wh%8) > 0 ? 1 : 0);
}

/**
 * Retrieve the index of the first character byte in a font stored in progmem
 * and of format 'bc'.
 *
 * params:
 * 		- char_code: code of character to get index for
 * 		- progmem_font: font stored in progmem
 *
 * returns: index of first byte that belongs to a character (hasPixels-flag byte).
 */
uint16_t uc_fonts_bc_get_char_index(uint8_t char_code,
									const uint8_t *progmem_font) {

	return 3 + (char_code * uc_fonts_get_bytes_per_non_empty_char(progmem_font));
}

/**
 * Retrieve the index of the first character byte in a font stored in progmem
 * and of format 'bcs'.
 *
 * params:
 * 		- char_code: code of character to get index for
 * 		- progmem_font: font stored in progmem
 *
 * returns: index of first byte that belongs to a character (hasPixels-flag byte).
 */
uint16_t uc_fonts_bcs_get_char_index(uint8_t char_code,
									 const uint8_t *progmem_font) {

	uint8_t bytes_per_non_empty_char = uc_fonts_get_bytes_per_non_empty_char(progmem_font);

	uint16_t byte_index = 3; //First byte after height byte

	for ( uint8_t i = 0; i < char_code; i++ ) {
		if ( pgm_read_byte(&progmem_font[byte_index]) ) {
			//Char is not empty -> increase index by bytes_per_non_empty_char
			byte_index += bytes_per_non_empty_char;
		} else {
			//Char is empty -> increase just by one because pixel bytes are missing
			byte_index++;
		}
	}

	return byte_index;
}


/**
 * Draws a font.
 *
 * params:
 * 		- char_code: code of character
 * 		- x: x coordinate to start drawing
 * 		- y: y coordinate to start drawing
 * 		- draw_white_pixels: if 1, also white pixels will be drawn, if 0 only black pixels will be drawn
 * 		- progmem_font: font in progmem
 */
void uc_fonts_draw_char(uint8_t char_code,
						uint8_t x,
						uint8_t y,
						uint8_t draw_white_pixels,
						const uint8_t *progmem_font) {

	uint8_t settings = pgm_read_byte(&progmem_font[0]);
	uint8_t width = pgm_read_byte(&progmem_font[1]);
	uint8_t height = pgm_read_byte(&progmem_font[2]);
	uint8_t wh = width * height;

	if ( char_code == 32 ) {
		if ( draw_white_pixels ) {
			for ( uint8_t cy = y; cy < y+height; cy++ ) {
				for ( uint8_t cx = x; cx < x+width; cx++ ) {
					LCD_API_SET_PIXEL(cx, cy, 0);
				}
			}
		}
		return;
	}

	uint16_t char_byte_index = 0;
	if ( settings & UC_FONTS_SETTINGS_BIT_BC_MASK ) {
		char_byte_index = uc_fonts_bc_get_char_index(char_code, progmem_font);
	} else if ( settings & UC_FONTS_SETTINGS_BIT_BCS_MASK ) {
		char_byte_index = uc_fonts_bcs_get_char_index(char_code, progmem_font);
	} else {
		return;
	}

	//If char is empty, display null char
	if ( char_code != 0 && pgm_read_byte(&progmem_font[char_byte_index]) == 0 ) char_byte_index = 3;

	if ( pgm_read_byte(&progmem_font[char_byte_index++]) ) {
		//Char not empty!

		uint8_t current_x = x;
		uint8_t current_y = y;
		uint8_t current_byte = 0;

		if ( settings & UC_FONTS_SETTINGS_BIT_HV_MASK ) {
			//Left to right, top to bottom

			for ( uint16_t i = 0; i < wh; i++ ) {
				if ( i % 8 == 0 ) current_byte = pgm_read_byte(&progmem_font[char_byte_index++]);

				uint8_t pixel = current_byte & 0x01;
				if ( pixel ) LCD_API_SET_PIXEL(current_x, current_y, 1);
				else if ( draw_white_pixels ) LCD_API_SET_PIXEL(current_x, current_y, 0);

				current_byte >>= 1;
				current_x++;

				if ( current_x - x == width ) {
					current_x = x;
					current_y++;
				}
			}
		} else if ( settings & UC_FONTS_SETTINGS_BIT_VH_MASK ) {
			//Top to bottom, left to right

			for ( uint16_t i = 0; i < wh; i++ ) {
				if ( i % 8 == 0 ) current_byte = pgm_read_byte(&progmem_font[char_byte_index++]);

				uint8_t pixel = current_byte & 0x01;
				if ( pixel ) LCD_API_SET_PIXEL(current_x, current_y, 1);
				else if ( draw_white_pixels ) LCD_API_SET_PIXEL(current_x, current_y, 0);

				current_byte >>= 1;
				current_y++;

				if ( current_y - y == height ) {
					current_y = y;
					current_x++;
				}
			}
		}
	}
}

/**
 * Draws a series of characters.
 *
 * params:
 * 		- string: string to draw
 * 		- x: x coordinate to start drawing
 * 		- y: y coordinate to start drawing
 * 		- draw_white_pixels: if 1, white pixels of characters will be drawn too, if 0 only black pixels will be drawn
 * 		- fill_char_gips: if 1, gaps between chars are overridden by white pixels
 * 		- progmem_font: font in progmem
 */
void uc_fonts_draw_string(char *string,
						  uint8_t x,
						  uint8_t y,
						  uint8_t draw_white_pixels,
						  uint8_t fill_char_gaps,
						  const uint8_t *progmem_font) {

	uint8_t x_advance = uc_fonts_get_char_width(progmem_font) + 1;
	uint8_t font_height = uc_fonts_get_char_height(progmem_font);

	uint8_t string_index = 0;
	uint8_t current_x = x;
	uint8_t current_char_code = 0;

	while ( (current_char_code = string[string_index++]) ) {
		if ( string_index > 1 ) { // greater than 1 because index got already incremented in while condition
			current_x += x_advance;
			if ( fill_char_gaps )
				uc_graphics_draw_line_left_top_right_bottom(current_x-1, y, current_x-1, y+font_height-1, 0);
		}
		uc_fonts_draw_char(current_char_code, current_x, y, draw_white_pixels, progmem_font);
	}
}

/**
 * Draws a series of characters from progmem.
 *
 * params:
 * 		- progmeme_string: string stored in progmem to draw
 * 		- x: x coordinate to start drawing
 * 		- y: y coordinate to start drawing
 * 		- draw_white_pixels: if 1, white pixels of characters will be drawn too, if 0 only black pixels will be drawn
 * 		- fill_char_gips: if 1, gaps between chars are overridden by white pixels
 * 		- progmem_font: font in progmem
 */
void uc_fonts_draw_string_progmem(const char *progmem_string,
								  uint8_t x,
								  uint8_t y,
								  uint8_t draw_white_pixels,
								  uint8_t fill_char_gaps,
								  const uint8_t *progmem_font) {

	uint8_t x_advance = uc_fonts_get_char_width(progmem_font) + 1;
	uint8_t font_height = uc_fonts_get_char_height(progmem_font);

	uint8_t string_index = 0;
	uint8_t current_x = x;
	uint8_t current_char_code = 0;

	while ( (current_char_code = pgm_read_byte(&progmem_string[string_index++])) ) {
		if ( string_index > 1 ) { // greater than 1 because index got already incremented in while condition
			current_x += x_advance;
			if ( fill_char_gaps )
				uc_graphics_draw_line_left_top_right_bottom(current_x-1, y, current_x-1, y+font_height-1, 0);
		}
		uc_fonts_draw_char(current_char_code, current_x, y, draw_white_pixels, progmem_font);
	}
}


/**
 * Draws a string and creates line breaks if necessary.
 *
 * params:
 * 		- text: string of characters to draw
 * 		- x: x coordinate to start drawing
 * 		- y: y coordinate to start drawing
 * 		- line_spacing: amount of pixels between lines
 * 		- max_x: last x coordinate where it is allowed to draw character pixels to
 * 		- max_y: last y coordinate where it is allowed to draw character pixels to
 * 		- draw_white_pixels: if 1, white pixels of characters will be drawn too, if 0 only the black ones
 * 		- fill_gaps: if 1, gaps between characters and lines will be filled with white pixels
 * 		- progmem_font: font stored in progmem
 */
void uc_fonts_draw_text(char *text,
						uint8_t x,
						uint8_t y,
						uint8_t line_spacing,
						uint8_t max_x,
						uint8_t max_y,
						uint8_t draw_white_pixels,
						uint8_t fill_char_gaps,
						const uint8_t *progmem_font) {

	uint8_t font_width = uc_fonts_get_char_width(progmem_font);
	uint8_t font_height = uc_fonts_get_char_height(progmem_font);

	uint8_t x_advance = font_width + 1;
	uint8_t x_advance_plus_width_minus_one = x_advance+font_width-1;
	uint8_t y_advance = font_height + line_spacing;
	uint8_t y_advance_plus_width_minus_one = y_advance+font_height-1;

	uint8_t string_index = 0;
	uint8_t current_x = x;
	uint8_t current_y = y;
	uint8_t current_char_code = 0;
	uint8_t line_beginning = 1;

	if ( x+font_width-1 > max_x ) return;
	if ( y+font_height-1 > max_y ) return;

	while ( (current_char_code = text[string_index++]) ) {
		if ( !line_beginning && fill_char_gaps ) {
			uc_graphics_draw_line_left_top_right_bottom(current_x-1, current_y, current_x-1, current_y+font_height-1, 0);
		}
		line_beginning = 0;

		uc_fonts_draw_char(current_char_code, current_x, current_y, draw_white_pixels, progmem_font);

		if ( current_x + x_advance_plus_width_minus_one > max_x ) {
			if ( current_y + y_advance_plus_width_minus_one > max_y ) {
				break;
			} else {


				if ( fill_char_gaps ) {
					uc_graphics_fill_rect(x, current_y+font_height, current_x+font_width-x, line_spacing, 0);
				}

				current_x = x;
				current_y += y_advance;
				line_beginning = 1;
			}
		} else {
			current_x += x_advance;
		}
	}
}

/**
 * Draws a string stored in progmem and creates line breaks if necessary.
 *
 * params:
 * 		- progmem_text: string of characters stored in progmem to draw
 * 		- x: x coordinate to start drawing
 * 		- y: y coordinate to start drawing
 * 		- line_spacing: amount of pixels between lines
 * 		- max_x: last x coordinate where it is allowed to draw character pixels to
 * 		- max_y: last y coordinate where it is allowed to draw character pixels to
 * 		- draw_white_pixels: if 1, white pixels of characters will be drawn too, if 0 only the black ones
 * 		- fill_gaps: if 1, gaps between characters and lines will be filled with white pixels
 * 		- progmem_font: font stored in progmem
 */
void uc_fonts_draw_text_progmem(const char *progmem_text,
								uint8_t x,
								uint8_t y,
								uint8_t line_spacing,
								uint8_t max_x,
								uint8_t max_y,
								uint8_t draw_white_pixels,
								uint8_t fill_char_gaps,
								const uint8_t *progmem_font) {

	uint8_t font_width = uc_fonts_get_char_width(progmem_font);
	uint8_t font_height = uc_fonts_get_char_height(progmem_font);

	uint8_t x_advance = font_width + 1;
	uint8_t x_advance_plus_width_minus_one = x_advance+font_width-1;
	uint8_t y_advance = font_height + line_spacing;
	uint8_t y_advance_plus_width_minus_one = y_advance+font_height-1;

	uint8_t string_index = 0;
	uint8_t current_x = x;
	uint8_t current_y = y;
	uint8_t current_char_code = 0;
	uint8_t line_beginning = 1;

	if ( x+font_width-1 > max_x ) return;
	if ( y+font_height-1 > max_y ) return;

	while ( (current_char_code = pgm_read_byte(&progmem_text[string_index++])) ) {
		if ( !line_beginning && fill_char_gaps ) {
			uc_graphics_draw_line_left_top_right_bottom(current_x-1, current_y, current_x-1, current_y+font_height-1, 0);
		}
		line_beginning = 0;

		uc_fonts_draw_char(current_char_code, current_x, current_y, draw_white_pixels, progmem_font);

		if ( current_x + x_advance_plus_width_minus_one > max_x ) {
			if ( current_y + y_advance_plus_width_minus_one > max_y ) {
				break;
			} else {


				if ( fill_char_gaps ) {
					uc_graphics_fill_rect(x, current_y+font_height, current_x+font_width-x, line_spacing, 0);
				}

				current_x = x;
				current_y += y_advance;
				line_beginning = 1;
			}
		} else {
			current_x += x_advance;
		}
	}
}

#endif
