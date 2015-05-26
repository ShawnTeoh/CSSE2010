/*
 * term.c
 *
 * Author: Thuan Song Teoh
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

#include "terminalio.h"
#include "term.h"
#include "game.h"

static uint8_t powerup_displayed = 0;

void term_redraw_background(void) {
	powerup_displayed = 0;
	move_cursor(37, 8);
}

void term_redraw_game_row(uint8_t row) {
	uint8_t background_row_data = get_background_data(row);
	uint8_t i;
	move_cursor(37, 23 - row);
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
	move_cursor(37, 8);
}

void term_draw_start_or_finish_line(uint8_t row) {
	set_display_attribute(BG_WHITE);
	move_cursor(37, 23 - row);
	printf_P(PSTR("        "));
	normal_display_mode();
	move_cursor(37, 8);
}

void term_redraw_car(uint8_t colr, uint8_t column) {
	set_display_attribute(BG_YELLOW);
	if(colr == COLOUR_CRASH) {
		set_display_attribute(BG_RED);
	} else if(colr == COLOUR_POWERUP) {
		set_display_attribute(BG_GREEN);
	}
	move_cursor(37 + column, 21);
	printf_P(PSTR(" "));
	move_cursor(37 + column, 22);
	printf_P(PSTR(" "));
	normal_display_mode();
	move_cursor(37, 8);
}

void term_erase_car(uint8_t bg1, uint8_t bg2, uint8_t column) {
	move_cursor(37 + column, 22);
	if(bg1) {
		set_display_attribute(BG_BLUE);
		printf_P(PSTR(" "));
		normal_display_mode();
	} else {
		printf_P(PSTR(" "));
	}

	move_cursor(37 + column, 21);
	if(bg2) {
		set_display_attribute(BG_BLUE);
		printf_P(PSTR(" "));
		normal_display_mode();
	} else {
		printf_P(PSTR(" "));
	}

	move_cursor(37, 8);
}

void term_draw_powerup(uint8_t column) {
	if(!powerup_displayed) {
		move_cursor(37 + column, 8);
		set_display_attribute(TERM_BLINK);
		set_display_attribute(TERM_BRIGHT);
		set_display_attribute(FG_GREEN);
		printf_P(PSTR("p"));
		normal_display_mode();
		move_cursor(37, 8);
	}
	powerup_displayed = 1;
}