/*
 * game.h
 *
 * Author: Peter Sutton
 *
 * The game has 16 rows, numbered 0 to 15 from the bottom. 
 * The car is always in rows 1 and 2 (two pixels in size).
 * The background scrolls down. There are 8 columns, numbered
 * 0 to 7 from the left.
 *
 * The functions in this module will update the LED matrix
 * display as required. Note that the LED matrix display has
 * a different understanding of rows and columns. See the comment
 * at the top of game.c for details.
 */ 

#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

#include "pixel_colour.h"

// Colours
#define COLOUR_BACKGROUND	COLOUR_LIGHT_GREEN
#define COLOUR_CAR			COLOUR_LIGHT_ORANGE
#define COLOUR_CRASH		COLOUR_RED
#define COLOUR_FINISH_LINE	COLOUR_YELLOW		/* Also the start line */
#define COLOUR_POWERUP		COLOUR_GREEN

// Reset the game. Get the background ready and place the car in the 
// initial position. Output this to the LED matrix.
void init_game(void);

// Put a car at the starting position. (This would typically be called after
// a restart.)
void put_car_at_start(void);

/////////////////////////////////// MOVE FUNCTIONS /////////////////////////
// has_car_crashed() should be checked after calling one of these to see
// if the move succeeded or not

// Move the car one column left
// This function must NOT be called if the car is in column 0.
// Failure may occur if the car collides with the background.
void move_car_left(void);

// Move the car one column right
// This function must NOT be called if the car is in column 7.
// Failure may occur if the car collides with the background.
void move_car_right(void);

/////////////////////// CAR / GAME STATUS ///////////////////////////////////
// Get the column that the car is currently in (will return a number
// between 0 and 7 inclusive)
uint8_t get_car_column(void);

// Return true if the car has crashed, false otherwise
uint8_t has_car_crashed(void);

// Return true if the car has finished the lap, false otherwise
uint8_t has_lap_finished(void);

// Return the number of lives left
uint8_t get_lives(void);

// Alter number of lives (-n to deduct, n to increase)
void set_lives(int8_t num);

// Reset lives to MAX_LIVES
void reset_lives(void);

// Set power-up status
void set_powerup(uint8_t status);

// Get power-up status
uint8_t powerup_status(void);

// Blink the power-up pixel
void blink_powerup();

// Toggle car colour
void toggle_car_colour(uint8_t reset);

// Returns background data at specified row
uint8_t get_background_data(uint8_t row);

/////////////////////// UPDATE FUNCTIONS /////////////////////////////////////
// Scroll the background by one row and update the display. Note that this
// may cause the car to crash.
void scroll_background(void);

#endif /* GAME_H_ */