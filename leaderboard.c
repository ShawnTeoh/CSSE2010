/*
 * leaderboard.c
 *
 * Author: Thuan Song Teoh
 *
 * Logic to display and update leader board. Uses EEPROM for data
 * storage.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "leaderboard.h"

// Memory address of stored variable in EEPROM
static Highscore EEMEM scores[MAX_NUM];

// High scores are stored as an array of Highscore structures
static Highscore current_score[MAX_NUM];

void retrive_leaderboard(void) {
	eeprom_read_block((void*)&current_score, (const void*)&scores, sizeof(scores));
}

/* Update values stored in EEPROM.
 */
static void update_leaderboard(void) {
	eeprom_update_block((const void*)&current_score, (void*)&scores, sizeof(scores));
}

/* Helper function to compare current scores with scores in leader board.
 * Returns -1 if not a new record/zero, else 0 to 4 representing the ranking.
 */
static int8_t compare_score(void) {
	uint8_t rank;
	if(get_score() > 0) {
		for(rank=0;rank<5;rank++) {
			// Only compare if value initialised
			if(current_score[rank].signature == SIGNATURE) {
				if(get_score() > current_score[rank].score) {
					return rank;
				}
			} else {
				// Uninitialised value means space available for new record
				return rank;
			}
		}
	}
	return -1;
}

/* Helper function to reposition old records and add new record.
 */
static void update_scores(char* name, uint8_t rank) {
	uint8_t pos;
	// Move old records down by 1 ranking to make space for new entry
	for(pos=MAX_NUM-1;pos>rank;pos--) {
		// Avoid repositioning uninitialised values
		if(current_score[pos-1].signature == SIGNATURE) {
			current_score[pos] = current_score[pos-1];
		}
	}
	current_score[rank].signature = SIGNATURE;
	strcpy(current_score[rank].name, name);
	current_score[rank].score = get_score();
}

/* Helper function to get player's initials.
 */
static char* get_initials(void) {
	// Initialise empty name at first (empty names are still valid)
	char tmp[6];
	memset(tmp, 0, 6);
	char input;
	uint8_t pos = 0;
	uint8_t escape_seq = 0;

	while(1) {
		while(!serial_input_available()) {
			; // Wait for serial data
		}

		// Part of code adapted from project.c to ignore escape sequences
		input = fgetc(stdin);
		if(input == '\n') {
			// Enter key pressed, save name
			break;
		} else if(escape_seq == 0 && input == ESCAPE_CHAR) {
			// We've hit the first character in an escape sequence (escape)
			escape_seq++;
		} else if(escape_seq == 1 && input == '[') {
			// We've hit the second character in an escape sequence
			escape_seq++;
		} else if(escape_seq == 2) {
			// Third (and last) character in the escape sequence
			escape_seq = 0;
		} else if(isalpha(input) && pos < 5) {
			// Only alphabets are valid, maximum of 5 characters
			tmp[pos] = input;
			pos++;
		} else if(input == BACK_SPACE) {
			// Backspace key pressed, clear previous character
			if(pos > 0) {
				pos--;
				tmp[pos] = '\0';
			} else {
				tmp[pos] = '\0';
			}
		}
		// Redisplay updated name variable
		move_cursor(38, 15);
		printf_P(PSTR("%-5s"), tmp);
		move_cursor(38+pos, 15);
	}

	char* name = "     ";
	strcpy(name, tmp);
	return name;
}

void is_highscore(void) {
	int8_t ranking = compare_score();
	if (ranking != -1) {
		// Display prompt to enter player initials
		clear_terminal();
		set_display_attribute(FG_GREEN);
		move_cursor(32,8);
		printf_P(PSTR("CONGRATULATIONS!"));
		normal_display_mode();
		move_cursor(28,10);
		printf_P(PSTR("You got a new high score!"));
		move_cursor(23,12);
		printf_P(PSTR("Please enter your initials (max 5)"));
		move_cursor(30,13);
		printf_P(PSTR("Press enter to save:"));

		// Read from stdin
		move_cursor(38,15);
		set_display_attribute(FG_CYAN);
		char* name = get_initials();
		normal_display_mode();
		move_cursor(38,16);

		// Update leader board and store in EEPROM
		update_scores(name, ranking);
		update_leaderboard();
	}
}

void leaderboard_terminal_output(void) {
	// Pretty printing of leader board
	draw_horizontal_line(15, 0, 79);
	move_cursor(34,16);
	set_display_attribute(FG_YELLOW);
	set_display_attribute(TERM_UNDERSCORE);
	printf_P(PSTR("LEADER BOARD"));
	normal_display_mode();
	move_cursor(29,18);
	set_display_attribute(FG_YELLOW);
	printf_P(PSTR("Name      ->     Score"));
	normal_display_mode();

	uint8_t i;
	for(i=0;i<MAX_NUM;i++) {
		if(current_score[i].signature == SIGNATURE) {
			move_cursor(26,19+i);
			printf_P(PSTR("%d. %-5s     ->     %ld"), i+1, current_score[i].name, current_score[i].score);
		} else {
			move_cursor(26,19+i);
			printf_P(PSTR("%d."), i+1);
		}
	}
}