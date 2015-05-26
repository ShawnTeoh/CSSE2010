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

/* 1 if power-up already displayed, else 0.
 * Needed because using blinking text, so only required to display once.
 * In LED matrix the power-up pixel has to be toggled manually.
 */
static uint8_t powerup_displayed = 0;

/* redraw_background() in game.c already invoked the actual redrawing.
 * Main use of this is just to reset the powerup_displayed variable.
 * By doing this, it is assumed that redraw_background() is only called at each
 * new game.
 */
void term_redraw_background(void) {
	powerup_displayed = 0;
}

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

/* Does the same thing as redraw_powerup() in game.c, but
 * only doing it once.
 */
void term_draw_powerup(uint8_t column) {
	if(!powerup_displayed) {
		move_cursor(37+column,8);
		set_display_attribute(TERM_BLINK);
		set_display_attribute(TERM_BRIGHT);
		set_display_attribute(FG_GREEN);
		printf_P(PSTR("P"));
		normal_display_mode();
		move_cursor(37,8);
	}
	powerup_displayed = 1;
}