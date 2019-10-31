/**
 * This file can handle bitmap images encoded as byte arrays.
 *
 * The first byte must be the settings byte.
 * It tells which pixel direction is used:
 * 	- bit 5 == 1: left to right, top to bottom (hv)
 * 	- bit 4 == 1: top to bottom, left to right (vh)
 *
 * 	The second and third byte store the width and
 * 	height of an image.
 *
 * 	Byte 3 & up store the pixels stored LSB and zero padded
 * 	at the end.
 *
 *
 *
 * 	Images can be encoded using µC-Graphics-Tools:
 *
 * 	https://github.com/hedgehogs-mind/uc-graphics-tools
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
 */


#ifndef UC_AVR_GRAPHICS_IMAGES_H_
#define UC_AVR_GRAPHICS_IMAGES_H_

#ifndef LCD_API_SET_PIXEL
#error "µC-Graphics image library does require LCD_API_SET_PIXEL makro to be set, in order to draw an image."
#endif

#include <avr/pgmspace.h>

#define UC_IMG_SETTINGS_BIT_HV_MASK (1 << 5)
#define UC_IMG_SETTINGS_BIT_VH_MASK (1 << 4)

/**
 * Retrieves the settings byte of an image stored in PROGMEM.
 *
 * params:
 * 		- progmem_img: image stored in progmem
 *
 * returns: settings byte of image
 */
uint8_t uc_images_get_settings(const uint8_t *progmem_img) {
	return pgm_read_byte(&progmem_img[0]);
}

/**
 * Retrieves the width byte of an image stored in PROGMEM.
 *
 * params:
 * 		- progmem_img: image stored in progmem
 *
 * returns: width of image
 */
uint8_t uc_images_get_width(const uint8_t *progmem_img) {
	return pgm_read_byte(&progmem_img[1]);
}

/**
 * Retrieves the height byte of an image stored in PROGMEM.
 *
 * params:
 * 		- progmem_img: image stored in progmem
 *
 * returns: height of image
 */
uint8_t uc_images_get_height(const uint8_t *progmem_img) {
	return pgm_read_byte(&progmem_img[2]);
}


/**
 * Draws an image stored in PROGMEM. Needs the macro LCD_API_SET_PIXEL(x, y, pixel) in
 * order to draw the image.
 *
 * params:
 * 		- x: x coordinate to start drawing the image
 * 		- y: y coordinate to start drawing the image
 * 		- draw_white_pixels: if 1, white (0) pixels will be drawn, if 0 not
 */
void uc_images_draw(uint8_t x, uint8_t y, uint8_t draw_white_pixels, const uint8_t *progmem_img) {
	uint8_t settings = pgm_read_byte(&progmem_img[0]);
	uint8_t width = pgm_read_byte(&progmem_img[1]);
	uint8_t height = pgm_read_byte(&progmem_img[2]);
	uint16_t wh = width*height;

	uint16_t byte_index = 3;
	uint8_t current_x = x;
	uint8_t current_y = y;
	uint8_t current_byte = 0;
	uint8_t pixel = 0;

	if ( settings & UC_IMG_SETTINGS_BIT_HV_MASK ) {
		for ( uint16_t i = 0; i < wh; i++ ) {
			if ( (i % 8) == 0 ) current_byte = pgm_read_byte(&progmem_img[byte_index++]);

			pixel = (current_byte & 0x01);
			if ( pixel ) LCD_API_SET_PIXEL(current_x, current_y, 1);
			else if ( draw_white_pixels ) LCD_API_SET_PIXEL(current_x, current_y, 0);

			current_byte >>= 1;
			current_x++;

			if ( current_x - x == width ) {
				current_x -= width;
				current_y++;
			}
		}
	} else if ( settings & UC_IMG_SETTINGS_BIT_VH_MASK ) {
		for ( uint16_t i = 0; i < wh; i++ ) {
			if ( (i % 8) == 0 ) current_byte = pgm_read_byte(&progmem_img[byte_index++]);

			pixel = (current_byte & 0x01);
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

#endif
