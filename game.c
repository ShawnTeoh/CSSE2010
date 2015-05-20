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

#include "game.h"
#include "ledmatrix.h"
#include "pixel_colour.h"
#include "timer0.h"
#include <stdint.h>
#include <stdlib.h>

///////////////////////////////// Global variables //////////////////////
// car_column stores the current position of the car. Game columns are numbered
// from 0 (left) to 7 (right). The car spans game rows 1 and 2.
static int8_t car_column;
#define CAR_START_ROW 1

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

static uint8_t disp_powerup = 0;

// Colours
#define COLOUR_BACKGROUND	COLOUR_LIGHT_GREEN
#define COLOUR_CAR			COLOUR_LIGHT_ORANGE
#define COLOUR_CRASH		COLOUR_RED	
#define COLOUR_FINISH_LINE	COLOUR_YELLOW		/* Also the start line */
#define COLOUR_POWERUP		COLOUR_LIGHT_YELLOW

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
static void redraw_powerup();
static void erase_car();

		
/////////////////////////////// Public Functions ///////////////////////////////
// These functions are defined in the same order as declared in game.h

// Reset the game
void init_game(void) {
	// Initial scroll position
	srandom(get_clock_ticks());
	initial_scroll = random() % NUM_GAME_ROWS;
	scroll_position = initial_scroll;
	powerup_scroll_position = random() % (RACE_DISTANCE - 10) + 50 + scroll_position;
	powerup_row += scroll_position;
	powerup = 0; // Always turn off powerup at start of game

	redraw_background();
	
	// Add a car to the display. (This will redraw the car.)
	put_car_at_start();
}

// Add a car to the game, placed at an empty background.
void put_car_at_start(void) {
	// Initial starting position of car. It must be guaranteed that this
	// initial position does not clash with the background.
	srandom(get_clock_ticks());
	// Keep on changing position until car does not clash
	// with background (including 1 row before and after)
	do {
		car_column = rand() % 7;
	} while(car_crashes_at(car_column, 1));
	
	// Car is initially alive and hasn't finished
	car_crashed = 0;
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
		redraw_car();
	} // else car is at left hand side (column 0) and can't move left
}

void move_car_right(void) {
	if(car_column != 7) {
		// Car not at right hand side
		erase_car();
		car_column++;
		car_crashed = car_crashes_at(car_column, 0);
		redraw_car();
	} // else car is at right hand side (column 7) and can't move right
}

uint8_t get_car_column(void) {
	return car_column;
}

uint8_t has_car_crashed(void) {
	return !powerup && car_crashed;
}

void set_powerup(uint8_t status) {
	powerup = status;
}

uint8_t powerup_status(void) {
	return powerup;
}

uint8_t powerup_display(void) {
	if (scroll_position >= powerup_scroll_position && powerup_row > -1) {
		disp_powerup = 1;
	} else {
		disp_powerup = 0;
	}
	return !powerup && disp_powerup;
}

void blink_powerup(uint8_t reset) {
	if(reset) {
		powerup_colour = COLOUR_BLACK;
	} else {
		powerup_colour = (powerup_colour == COLOUR_POWERUP ? COLOUR_BLACK:COLOUR_POWERUP);
	}
	redraw_powerup();
}

void toggle_car_colour(uint8_t reset) {
	if(reset) {
		car_colour = COLOUR_CAR;
	} else {
		car_colour = (car_colour == COLOUR_CAR ? COLOUR_POWERUP:COLOUR_CAR);
	}
	redraw_car();
}

uint8_t has_lap_finished(void) {
	// If the top pixel in the car (two ahead of the scroll position) has
	// reached the finish row then we have crossed the line
	return lap_finished;
}

void scroll_background(void) {
	scroll_position++;
	if(scroll_position == powerup_scroll_position) {
		powerup_row = 15;
	}
	if(powerup_row > -1) {
		powerup_row--;
	}
	
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
	ledmatrix_shift_display_right();
	if(powerup_display()) {
		redraw_powerup();
	}
	redraw_car();
	redraw_game_row(15);
}

uint8_t get_lives(void) {
	return lives;
}

void reset_lives(void) {
	lives = MAX_LIVES;
}

void set_lives(int8_t num) {
	if (lives >= 0 && lives <= MAX_LIVES) {
		lives += num;
	}
	if (lives > MAX_LIVES) {
		lives = MAX_LIVES;
	}
}

/////////////////////////////// Private (Helper) Functions /////////////////////

// Return 1 if the car crashes if moved into the given column. We compare the car
// position with the background in rows 1 and 2, rows 0, 3 and 4 as well if extend == 1
static uint8_t car_crashes_at(uint8_t column, uint8_t extend) {
	uint8_t start;
	uint8_t end;
	if(extend) {
		start = 0;
		end = 4;
	} else {
		start = 1;
		end = 2;
	}

	// Check rows at this column
	uint8_t i;
	for (i=start;i<=end;i++) {
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
	// Remove "broken" walls or crashed car
	redraw_game_row(0);
	redraw_game_row(1);
	redraw_game_row(2);
	if(has_car_crashed()) {
		car_colour = COLOUR_CRASH;
	}
	ledmatrix_update_pixel(15 - CAR_START_ROW, car_column, car_colour);
	ledmatrix_update_pixel(15 - (CAR_START_ROW+1), car_column, car_colour);
}

static void redraw_powerup(void) {
	// Reset previous pixel
	if(powerup_row < 15) {
		ledmatrix_update_pixel(powerup_row + 1, powerup_column, COLOUR_BLACK);
	}
	// Update current pixel
	ledmatrix_update_pixel(powerup_row, powerup_column, powerup_colour);
}

// Erase the car (we assume it hasn't crashed - so we just replace
// the car position with black)
static void erase_car(void) {
	ledmatrix_update_pixel(15 - CAR_START_ROW, car_column, COLOUR_BLACK);
	ledmatrix_update_pixel(15 - (CAR_START_ROW+1), car_column, COLOUR_BLACK);
}