/*
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

#ifndef UC_AVR_GRAPHICS_GRAPHICS_H_
#define UC_AVR_GRAPHICS_GRAPHICS_H_

// TODO: check if all actions perform set pixel from left to right!!! -> this can save setColumn commands MASSIVELY

void uc_graphics_draw_line_left_top_right_bottom(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel) {
	if ( x1 == x2 && y1 == y2 ) LCD_API_SET_PIXEL(x1, y1, pixel);
	else if ( x1 == x2 ) {
		for ( uint8_t y = y1; y < y2+1; y++ ) {
			LCD_API_SET_PIXEL(x1, y, pixel);
		}
	} else if ( y1 == y2 ) {
		for ( uint8_t x = x1; x < x2+1; x++ ) {
			LCD_API_SET_PIXEL(x, y1, pixel);
		}
	} else {
		int16_t x1_16 = (int16_t)x1;
		int16_t x2_16 = (int16_t)x2;
		int16_t y1_16 = (int16_t)y1;
		int16_t y2_16 = (int16_t)y2;
		if ( y2-y1 > x2-x1 ) {
			for ( int16_t y = y1; y < y2+1; y++ ) {
				int16_t x = ((x2_16-x1_16)*(y-y1_16))/(y2_16-y1_16)+x1_16;
				LCD_API_SET_PIXEL(x, y, pixel);
			}
		} else {
			for ( int16_t x = x1; x < x2+1; x++ ) {
				int16_t y = ((y2_16-y1_16)*(x-x1_16))/(x2_16-x1_16)+y1_16;
				LCD_API_SET_PIXEL(x, y, pixel);
			}
		}
	}
}

void uc_graphics_draw_line_left_bottom_right_top(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel) {
	if ( x1 == x2 && y1 == y2 ) LCD_API_SET_PIXEL(x1, y1, pixel);
	else if ( x1 == x2 ) {
		for ( uint8_t y = y2; y < y1+1; y++ ) {
			LCD_API_SET_PIXEL(x1, y, pixel);
		}
	} else if ( y1 == y2 ) {
		for ( uint8_t x = x1; x < x2+1; x++ ) {
			LCD_API_SET_PIXEL(x, y1, pixel);
		}
	} else {
		int16_t x1_16 = (int16_t)x1;
		int16_t x2_16 = (int16_t)x2;
		int16_t y1_16 = (int16_t)y1;
		int16_t y2_16 = (int16_t)y2;
		if ( x2-x1 < y1-y2 ) {
			for ( int16_t y = y2; y < y1+1; y++ ) {
				int16_t x = ((x2_16-x1_16)*(y-y1_16))/(y2_16-y1_16)+x1_16;
				LCD_API_SET_PIXEL(x, y, pixel);
			}
		} else {
			for ( int16_t x = x1; x < x2+1; x++ ) {
				int16_t y = ((y2_16-y1_16)*(x-x1_16))/(x2_16-x1_16)+y1_16;
				LCD_API_SET_PIXEL(x, y, pixel);
			}
		}
	}
}

void uc_graphics_draw_line_right_top_left_bottom(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel) {
	uc_graphics_draw_line_left_bottom_right_top(x2, y2, x1, y1, pixel);
}

void uc_graphics_draw_line_right_bottom_left_top(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel) {
	uc_graphics_draw_line_left_top_right_bottom(x2, y2, x1, y1, pixel);
}

void uc_graphics_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixel) {
	if ( x1 < x2 ) {
		//Left to right
		if ( y1 < y2 ) {
			//Top to bottom
			uc_graphics_draw_line_left_top_right_bottom(x1, y1, x2, y2, pixel);
		} else {
			//Bottom to top
			uc_graphics_draw_line_left_bottom_right_top(x1, y1, x2, y2, pixel);
		}
	} else {
		//Right to left
		if ( y1 < y2 ) {
			//Top to bottom
			uc_graphics_draw_line_right_top_left_bottom(x1, y1, x2, y2, pixel);
		} else {
			//Bottom to top
			uc_graphics_draw_line_right_bottom_left_top(x1, y1, x2, y2, pixel);
		}
	}
}

void uc_graphics_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t pixel) {
	if ( width > 1 && height > 1 ) {
		uint8_t right_x = x+width-1;
		uint8_t bottom_y = y+height-1;

		for ( uint8_t i = 0; i < width; i++ ) {
			LCD_API_SET_PIXEL(x+i, y, pixel);
		}
		for ( uint8_t i = 0; i < height; i++ ) {
			LCD_API_SET_PIXEL(x, y+i, pixel);
		}
		for ( uint8_t i = 0; i < height; i++ ) {
			LCD_API_SET_PIXEL(right_x, y+i, pixel);
		}
		for ( uint8_t i = 0; i < width; i++ ) {
			LCD_API_SET_PIXEL(x+i, bottom_y, pixel);
		}
	} else if ( width == 1 && height > 1 ) {
		for ( uint8_t i = 0; i < height; i++ ) {
			LCD_API_SET_PIXEL(x, y+i, pixel);
		}
	} else if ( height == 1 && width > 1 ) {
		for ( uint8_t i = 0; i < width; i++ ) {
			LCD_API_SET_PIXEL(x+i, y, pixel);
		}
	} else if ( width == 1 && height == 1 ) {
		LCD_API_SET_PIXEL(x, y, pixel);
	}
}

void uc_graphics_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t pixel) {
	if ( width > 1 && height > 1 ) {
		for ( uint8_t j = 0; j < height; j++ ) {
			for ( uint8_t i = 0; i < width; i++ ) {
				LCD_API_SET_PIXEL(x+i, y+j, pixel);
			}
		}
	} else if ( width == 1 && height > 1 ) {
		for ( uint8_t i = 0; i < height; i++ ) {
			LCD_API_SET_PIXEL(x, y+i, pixel);
		}
	} else if ( height == 1 && width > 1 ) {
		for ( uint8_t i = 0; i < width; i++ ) {
			LCD_API_SET_PIXEL(x+i, y, pixel);
		}
	} else if ( width == 1 && height == 1 ) {
		LCD_API_SET_PIXEL(x, y, pixel);
	}
}

#endif /* SRC_GRAPHICS_H_ */
