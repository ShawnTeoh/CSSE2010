/*
 * game.c
 *
 * Author: Peter Sutton
 *
 * Note that we use the display in a rotated orientation. The display should 
 * be oriented such that the "R/G LED MATRIX" words are in the correct
 * orientation. 
 * Display columns (0 to 15) are game rows 15 (top) to 0 (bottom) in this orientation.
 * Display rows (0 to 7) are game columns 0 (left) to 7 (right) in this orientation.
 */ 

#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "ledmatrix.h"
#include "timer0.h"
#include "timer2.h"
#include "terminalio.h"
#include "term.h"

///////////////////////////////// Global variables //////////////////////
// car_column stores the current position of the car. Game columns are numbered
// from 0 (left) to 7 (right). The car spans game rows 1 and 2.
static int8_t car_column = 0;
#define CAR_START_ROW 1

// Position variables of power-up
static uint8_t powerup_scroll_position;
static int8_t powerup_column;
static int8_t powerup_row = -1;

// Boolean flag to indicate whether the car has crashed or not
static uint8_t car_crashed;

// Boolean flag to indicate whether the lap has finished or not
static uint8_t lap_finished;

// Number of lives player has left
static uint8_t lives;

// Maximum number of player lives
#define MAX_LIVES 3

// Background - 8 bits in each row. A 1 indicates background being
// present, a 0 is empty space. We will scroll through this and
// return to the other end. Bit 0 (LSB) in these patterns will end
// up on the left of the display (column 0).
#define NUM_GAME_ROWS 32	
static uint8_t background_data[NUM_GAME_ROWS] = {
		0b10000011,
		0b10000111,
		0b10000111,
		0b10000111,
		0b11000011,
		0b11100001,
		0b11110001,
		0b11110001,
		0b11100000,
		0b11100000,
		0b11100000,
		0b11000000,
		0b11000001,
		0b10000001,
		0b10000011,
		0b10000011,
		0b10000001,
		0b10000001,
		0b00010000,
		0b00011000,
		0b00011100,
		0b00111100,
		0b00111000,
		0b00111000,
		0b00010000,
		0b00010000,
		0b00000001,
		0b10000001,
		0b10000001,
		0b11000011,
		0b10000111,
		0b10000111
};
		
#define RACE_DISTANCE 128	// The effective row number where the finish line 
							// will appear

// Which row is at the bottom of the screen - starting at 0 and counting up 
// until we reach the finish line (defined by RACE_DISTANCE)
static uint8_t scroll_position;

// Offset of scroll position
static uint8_t initial_scroll;

// Power-up status (1 on, 0 off)
static uint8_t powerup = 0;

// Flag for power-up display (1 on, 0 off)
static uint8_t disp_powerup = 0;

// Variable to contain current car colour
static uint8_t car_colour = COLOUR_CAR;

// Variable to contain current power-up pixel colour
static uint8_t powerup_colour = COLOUR_POWERUP;


/////////////////////////////// Function Prototypes for Helper Functions ///////
// These functions are defined after the public functions. Comments are with the
// definitions.
static uint8_t car_crashes_at(uint8_t column, uint8_t extend);
static void redraw_background(void);
static void redraw_game_row(uint8_t row);
static void draw_start_or_finish_line(uint8_t row);
static void redraw_car();
static uint8_t check_if_background(uint8_t row, uint8_t column);
static void erase_car();
static void redraw_powerup();
static uint8_t powerup_display(void);
static uint8_t powerup_crashes_at(uint8_t column);
static void place_powerup(void);
static uint8_t car_touches_powerup(uint8_t column);
static void powerup_check(void);


/////////////////////////////// Public Functions ///////////////////////////////
// These functions are defined in the same order as declared in game.h

// Reset the game
void init_game(void) {
	// Initial scroll position
	srandom(get_timer0_clock_ticks());
	initial_scroll = random() % NUM_GAME_ROWS;
	scroll_position = initial_scroll;

	// Reset sound
	set_sound_type(0);

	// Determine where power-up will appear
	place_powerup();
	powerup = 0; // Always turn off powerup at start of game

	clear_terminal();
	set_scroll_region(8, 23);
	redraw_background();
	term_redraw_background();
	
	// Add a car to the display. (This will redraw the car.)
	put_car_at_start();
}

// Add a car to the game, placed at an empty background.
void put_car_at_start(void) {
	// Initial starting position of car. It must be guaranteed that this
	// initial position does not clash with the background.
	erase_car();
	srandom(get_timer0_clock_ticks());
	// Keep on changing column until car does not clash
	// with background (including 2 rows before)
	do {
		car_column = random() % 7;
	} while(car_crashes_at(car_column, 1));
	
	// Car is initially alive and hasn't finished
	car_crashed = 0;
	car_colour = COLOUR_CAR; // Reset car colour
	lap_finished = 0;

	// Show the car
	redraw_car();
}

void move_car_left(void) {
	if(car_column != 0) {
		// Car not at left hand side
		erase_car();
		car_column--;
		car_crashed = car_crashes_at(car_column, 0);
		// Check if car on power-up pixel
		powerup_check();
		redraw_car();
	} // else car is at left hand side (column 0) and can't move left
}

void move_car_right(void) {
	if(car_column != 7) {
		// Car not at right hand side
		erase_car();
		car_column++;
		car_crashed = car_crashes_at(car_column, 0);
		// Check if car on power-up pixel
		powerup_check();
		redraw_car();
	} // else car is at right hand side (column 7) and can't move right
}

uint8_t get_car_column(void) {
	return car_column;
}

uint8_t has_car_crashed(void) {
	return !powerup && car_crashed;
}

uint8_t has_lap_finished(void) {
	// If the top pixel in the car (two ahead of the scroll position) has
	// reached the finish row then we have crossed the line
	return lap_finished;
}

uint8_t get_lives(void) {
	return lives;
}

void set_lives(int8_t num) {
	lives += num;
	if(lives > MAX_LIVES) {
		lives = MAX_LIVES;
	} else if(lives < 0) {
		lives = 0;
	}
}

void reset_lives(void) {
	lives = MAX_LIVES;
}

void set_powerup(uint8_t status) {
	powerup = status;
}

uint8_t powerup_status(void) {
	return powerup;
}

void blink_powerup() {
	if(powerup_display()) {
		powerup_colour = (powerup_colour == COLOUR_POWERUP ? COLOUR_BLACK:COLOUR_POWERUP);
		redraw_powerup();
	}
}

void toggle_car_colour(uint8_t reset) {
	if(reset) {
		car_colour = COLOUR_CAR;
	} else {
		car_colour = (car_colour == COLOUR_CAR ? COLOUR_POWERUP:COLOUR_CAR);
	}
	redraw_car();
}

void scroll_background(void) {
	scroll_position++;
	// Display power-up if position reached
	if(scroll_position == powerup_scroll_position) {
		powerup_row = 16; // 16 instead of 15 cause will be decreased right away below
	}
	// Shift power-up pixel
	if(powerup_row > -1) {
		powerup_row--;
	}

	// Check if car on power-up pixel
	powerup_check();

	// Check if the lap has finished. We add 2 to the scroll position
	// because we're looking at the front of the car. 
	if(scroll_position - initial_scroll + 2 == RACE_DISTANCE) {
		lap_finished = 1;	
	} else {
		// If we haven't finished the lap, then 
		// check whether the car has crashed or not in its current column
		// (The background may have scrolled into it.)
		car_crashed = car_crashes_at(car_column, 0);
	}

	// For speed purposes, we don't redraw the whole display, we
	// erase the car, shift the display down (right in the sense of the
	// LED matrix) and redraw the car and draw the new row 15
	erase_car();
	ledmatrix_shift_display_right(); // Scroll LED matrix
	scroll_down(); // Scroll terminal
	redraw_car();
	redraw_game_row(15);
	if(powerup_display()) {
		// Display power-up pixel
		redraw_powerup();
		term_draw_powerup(powerup_column);
	}
}

uint8_t get_background_data(uint8_t row) {
	uint8_t race_row = scroll_position + row;
	return background_data[race_row % NUM_GAME_ROWS];
}

/////////////////////////////// Private (Helper) Functions /////////////////////

// Return 1 if the car crashes if moved into the given column. We compare the car
// position with the background in rows 1 and 2, rows 3 and 4 as well if extend == 1
static uint8_t car_crashes_at(uint8_t column, uint8_t extend) {
	uint8_t end;
	if(extend) {
		end = 4;
	} else {
		end = 2;
	}

	// Check rows at this column
	uint8_t i;
	for(i=1;i<=end;i++) {
		uint8_t background_row_number = (i + scroll_position) % NUM_GAME_ROWS;
		uint8_t background_row_data = background_data[background_row_number];
		if(background_row_data & (1<< column)) {
			// Collision between car and background in row i
			return 1;
		}
	}

	// No collision 
	return 0;
}

// Clear the screen and redraw the background. The car is not redrawn.
static void redraw_background() {
	// Clear the display
	ledmatrix_clear();
	
	uint8_t row;
	for(row = 0; row <= 15; row++) {
		redraw_game_row(row);
	}
}

// Redraw the row with the given number (0 to 15). The car is not redrawn.
// Rows in the game are columns in the terminology of the display
static void redraw_game_row(uint8_t row) {	
	MatrixColumn row_display_data; // Note that a game row corresponds to a display
								   // column.
	uint8_t i;
	uint8_t race_row = scroll_position + row;
	if(race_row - initial_scroll == 0 || race_row - initial_scroll == RACE_DISTANCE) {
		draw_start_or_finish_line(row);
		term_draw_start_or_finish_line(row); // Do the same for terminal output
	} else {
		uint8_t background_row_data = background_data[race_row % NUM_GAME_ROWS];
		for(i=0;i<=7;i++) {
			if(background_row_data & (1<<i)) {
				// Bit i is set, meaning background is present
				row_display_data[i] = COLOUR_BACKGROUND;
			} else {
				row_display_data[i] = COLOUR_BLACK;
			}
		}
		ledmatrix_update_column(15 - row, row_display_data);
		term_redraw_game_row(row); // Do the same for terminal output
	}
}

static void draw_start_or_finish_line(uint8_t row) {
	// Draw a solid line at the given game display row (0 to 15)
	MatrixColumn row_display_data;
	uint8_t i;
	for(i=0;i<=7;i++) {
		row_display_data[i] = COLOUR_FINISH_LINE;
	}
	ledmatrix_update_column(15 - row, row_display_data);
}

// Redraw the car in its current position.
static void redraw_car(void) {
	if(has_car_crashed()) {
		car_colour = COLOUR_CRASH;
	}
	ledmatrix_update_pixel(15 - CAR_START_ROW, car_column, car_colour);
	ledmatrix_update_pixel(15 - (CAR_START_ROW+1), car_column, car_colour);
	term_redraw_car(car_colour, car_column); // Do the same for terminal output
}

// Check if given coordinates is a background or not (1 yes, 0 no)
static uint8_t check_if_background(uint8_t row, uint8_t column) {
	uint8_t race_row = scroll_position + row;
	uint8_t background_row_data = background_data[race_row % NUM_GAME_ROWS];
	return background_row_data & (1<<column);
}

// Erase the car (replace the car position with previous colour)
static void erase_car(void) {
	uint8_t bg1, bg2;
	if(check_if_background(CAR_START_ROW, car_column)) {
		ledmatrix_update_pixel(15 - CAR_START_ROW, car_column, COLOUR_BACKGROUND);
		bg1 = 1;
	} else {
		ledmatrix_update_pixel(15 - CAR_START_ROW, car_column, COLOUR_BLACK);
		bg1 = 0;
	}

	if(check_if_background(CAR_START_ROW+1, car_column)) {
		ledmatrix_update_pixel(15 - (CAR_START_ROW+1), car_column, COLOUR_BACKGROUND);
		bg2 = 1;
	} else {
		ledmatrix_update_pixel(15 - (CAR_START_ROW+1), car_column, COLOUR_BLACK);
		bg2 = 0;
	}

	term_erase_car(bg1, bg2, car_column); // Do the same for terminal output
}

// Update power-up pixel
static void redraw_powerup(void) {
	ledmatrix_update_pixel(15 - powerup_row, powerup_column, powerup_colour);
}

// Display power-up pixel iff position reached, within rows 0-15 and power-up not enabled
static uint8_t powerup_display(void) {
	if(scroll_position >= powerup_scroll_position && powerup_row > -1) {
		disp_powerup = 1;
	} else {
		disp_powerup = 0;
	}
	return !powerup && disp_powerup;
}

// Helper function to find an empty pixel to place power-up (1 not empty, 0 empty)
static uint8_t powerup_crashes_at(uint8_t column) {
	uint8_t background_row_number = (powerup_scroll_position + 15) % NUM_GAME_ROWS;
	uint8_t background_row_data = background_data[background_row_number];
	if(background_row_data & (1<< column)) {
		// Collision between power-up and background in chosen row
		return 1;
	}

	// No collision
	return 0;
}

// Helper function to determine the position to place power-up
static void place_powerup(void) {
	// We want to place the power-up slightly after the start of a lap
	// and not too close to finishing line
	powerup_scroll_position = random() % (RACE_DISTANCE - 60 - 30 + 1) + 30 + scroll_position;
	powerup_row = 16;
	// Keep on changing column until power-up does not clash with background
	do {
		powerup_column = random() % 7;
	} while(powerup_crashes_at(powerup_column));
}

// Determine if car is on power-up pixel
static uint8_t car_touches_powerup(uint8_t column) {
	if(powerup_display()) {
		if(powerup_row == 1 || powerup_row == 2) {
			return car_column == powerup_column;
		}
	}
	return 0;
}

// Change colour of car when powered-up
static void powerup_check(void) {
	if(powerup_display()) {
		powerup = car_touches_powerup(car_column);
		car_colour = (powerup ? COLOUR_POWERUP:COLOUR_CAR);
	}
}