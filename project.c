/*
 * RallyProject.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by Thuan Song Teoh.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>		// For random()

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "game.h"

#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void handle_new_lap(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	
	// Turn on global interrupts
	sei();
}

void splash_screen(void) {
	// Reset display attributes and clear terminal screen then output a message
	set_display_attribute(TERM_RESET);
	clear_terminal();
	
	hide_cursor();	// We don't need to see the cursor when we're just doing output
	move_cursor(3,3);
	printf_P(PSTR("RallyRacer"));
	
	move_cursor(3,5);
	set_display_attribute(FG_GREEN);	// Make the text green
	printf_P(PSTR("CSSE2010/7201 project by Thuan Song Teoh"));
	set_display_attribute(FG_WHITE);	// Return to default colour (White)
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	
	// Orange message the first time through
	set_text_colour(COLOUR_ORANGE);
	while(1) {
		set_scrolling_display_text("RALLYRACER 43068052");
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed. We pause for 130ms between each scroll.
		while(scroll_display()) {
			_delay_ms(130);
			if(button_pushed() != -1) {
				return;
			}
		}
		// Message has scrolled off the display. Change colour
		// to a random colour and scroll again.
		switch(random()%4) {
			case 0: set_text_colour(COLOUR_LIGHT_ORANGE); break;
			case 1: set_text_colour(COLOUR_RED); break;
			case 2: set_text_colour(COLOUR_YELLOW); break;
			case 3: set_text_colour(COLOUR_GREEN); break;
		}
	}
}

void new_game(void) {
	// Initialise the game and display
	init_game();
	
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the score
	init_score();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
	
	// Delay for half a second
	_delay_ms(500);
}

void play_game(void) {
	uint32_t current_time, last_move_time;
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	
	// Get the current time and remember this as the last time the background scrolled.
	current_time = get_clock_ticks();
	last_move_time = current_time;
	
	// We play the game while the car hasn't crashed
	while(!has_car_crashed()) {
		
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. We will be processing each character
		// independently and can't do anything until we get the third character.
		// At most one of the following three variables will be set to a value 
		// other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		
		if(button == -1) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		// Process the input. 
		if(button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
			// Attempt to move left
			move_car_left();
		} else if(button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
			// Attempt to move right
			move_car_right();
		} else if(serial_input == 'p' || serial_input == 'P') {
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again
		} 
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		
		current_time = get_clock_ticks();
		if(!has_car_crashed() && current_time >= last_move_time + 600) {
			// 600ms (0.6 second) has passed since the last time we scrolled
			// the background, so scroll it now and check whether that means
			// we've finished the lap. (If a crash occurs we will drop out of 
			// the main while loop so we don't need to check for that here.)
			scroll_background();
			if(has_lap_finished()) {
				handle_new_lap();	// Pauses until a button is pushed
				// Reset the time of the last scroll
				last_move_time = get_clock_ticks();
			} else {
				last_move_time = current_time;
			}
		}
	}
	// If we get here the car has crashed. 
}

void handle_game_over() {
	move_cursor(10,14);
	// Print a message to the terminal. The spaces on the end of the message
	// will ensure the "LAP COMPLETE" message is completely overwritten.
	printf_P(PSTR("GAME OVER   "));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	while(button_pushed() == -1) {
		; // wait until a button has been pushed
	}
	
}

void handle_new_lap() {
	move_cursor(10,14);
	printf_P(PSTR("LAP COMPLETE"));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to continue"));
	while(button_pushed() == -1) {
		; // wait until a button has been pushed
	}
	init_game();	// This will need to be changed for multiple lives
	
	// Delay for half a second
	_delay_ms(500);
}