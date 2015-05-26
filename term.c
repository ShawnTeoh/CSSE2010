/*
 * term.c
 *
 * Author: Thuan Song Teoh
 *
 * Methods to convert LED matrix output to terminal
 * output. Note that after every output, cursor is moved back
 * to row 8 to ensure scroll_down() works. Background and car
 * are represented by coloured whitespaces. Power-up is a blinking green 'P'.
 * This is essentially a plugin since no major changes to original code required,
 * apart from remembering to move moved back to row 8 after every output.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#include "terminalio.h"
#include "term.h"
#include "game.h"

/* Does the same thing as redraw_game_row() in game.c.
 */
void term_redraw_game_row(uint8_t row) {
	// Obtain row data
	uint8_t background_row_data = get_background_data(row);
	uint8_t i;
	move_cursor(37,23 - row);
	for(i=0;i<=7;i++) {
		if(background_row_data & (1<<i)) {
			// Bit i is set, meaning background is present
			set_display_attribute(BG_BLUE);
			printf_P(PSTR(" "));
			normal_display_mode();
		} else {
			printf_P(PSTR(" "));
		}
	}
	move_cursor(37,8);
}

/* Does the same thing as draw_start_or_finish_line() in game.c.
 */
void term_draw_start_or_finish_line(uint8_t row) {
	// Draw a white line
	set_display_attribute(BG_WHITE);
	move_cursor(37,23-row);
	printf_P(PSTR("        "));
	normal_display_mode();
	move_cursor(37,8);
}

/* Does the same thing as redraw_car() in game.c.
 */
void term_redraw_car(uint8_t colr, uint8_t column) {
	// Normal colour
	set_display_attribute(BG_YELLOW);
	if(colr == COLOUR_CRASH) {
		// Car crashed
		set_display_attribute(BG_RED);
	} else if(colr == COLOUR_POWERUP) {
		// Car powered up
		set_display_attribute(BG_GREEN);
	}
	move_cursor(37+column,21);
	printf_P(PSTR(" "));
	move_cursor(37+column,22);
	printf_P(PSTR(" "));
	normal_display_mode();
	move_cursor(37,8);
}

/* Does the same thing as erase_car() in game.c. Background detection
 * done in game.c.
 */
void term_erase_car(uint8_t bg1, uint8_t bg2, uint8_t column) {
	move_cursor(37+column,22);
	if(bg1) {
		// Is background, colour blue
		set_display_attribute(BG_BLUE);
		printf_P(PSTR(" "));
		normal_display_mode();
	} else {
		// Not background, colour black
		printf_P(PSTR(" "));
	}

	move_cursor(37+column,21);
	if(bg2) {
		// Is background, colour blue
		set_display_attribute(BG_BLUE);
		printf_P(PSTR(" "));
		normal_display_mode();
	} else {
		// Not background, colour black
		printf_P(PSTR(" "));
	}

	move_cursor(37,8);
}

/* Does the same thing as redraw_powerup() in game.c.
 */
void term_redraw_powerup(uint8_t row, uint8_t column, uint8_t colr) {
	move_cursor(37+column,23-row);
	if(colr == COLOUR_POWERUP) {
		set_display_attribute(BG_GREEN);
		printf_P(PSTR(" "));
		normal_display_mode();
	} else {
		printf_P(PSTR(" "));
	}
	move_cursor(37,8);
}