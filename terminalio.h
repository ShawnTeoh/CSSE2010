/*
 * terminalio.h
 *
 * Author: Peter Sutton
 *
 * Functions for interacting with the terminal. These should be used
 * to encapsulate all sending of escape sequences.
 */

#ifndef TERMINAL_IO_H_
#define TERMINAL_IO_H_

#include <stdint.h>

/*
 * x (column number) and y (row number) are measured relative to the top
 * left of the screen. First column is 1, first row is 1.
 *
 * The display parameter is a number between 0 and 47. Valid values are
 *								Foreground colours	Background colours
 *								------------------	------------------
 *	0 Reset all attributes		30 Black			40 Black
 *	1 Bright					31 Red				41 Red
 *	2 Dim						32 Green			42 Green
 *	4 Underscore				33 Yellow			43 Yellow
 *  5 Blink						34 Blue				44 Blue
 *	7 Reverse Video				35 Magenta			45 Magenta
 *	8 Hidden					36 Cyan				46 Cyan
 *								37 White			47 White
 */

typedef enum { TERM_RESET = 0, TERM_BRIGHT = 1, TERM_DIM = 2, TERM_UNDERSCORE = 4, 
		TERM_BLINK = 5, TERM_REVERSE = 7, TERM_HIDDEN = 8,
		FG_BLACK = 30, FG_RED = 31, FG_GREEN = 32, FG_YELLOW= 33, 
		FG_BLUE = 34, FG_MAGENTA = 35, FG_CYAN = 36, FG_WHITE = 37,
		BG_BLACK = 40, BG_RED = 41, BG_GREEN = 42, BG_YELLOW = 43,
		BG_BLUE = 44, BG_MAGENTA = 45, BG_CYAN = 46, BG_WHITE = 47
		} DisplayParameter;

void move_cursor(int8_t x, int8_t y);
void normal_display_mode(void);
void reverse_video(void);
void clear_terminal(void);
void clear_to_end_of_line(void);
void set_display_attribute(DisplayParameter parameter);
void hide_cursor(void);
void show_cursor(void);

// Enable scrolling for either the full screen or a particular region (rows)
// For set_scroll_region y1 < y2 and the region includes rows y1 and y2.
void enable_scrolling_for_whole_display(void);
void set_scroll_region(int8_t y1, int8_t y2);	

// If the cursor is in the first (top) row of the scroll region then scroll
// the scroll region down by one row. The bottom row of the scroll region will be lost.
// The top row of the scroll region will be blank. If the cursor is not in the first row
// of the scroll region then the cursor will just be moved up by one row.
void scroll_down(void);

// If the cursor is in the last (bottom) row of the scroll region then scroll
// the scroll region up by one row. The top row of the scroll region will be lost.
// The bottom row of the scroll region will be blank. If the cursor is not in the last
// row of the scroll region then cursor will just be moved down by one row.
void scroll_up(void);

/*
 * Draw a reverse video line on the terminal. startx must be <= endx.
 * starty must be <= endy
 */
void draw_horizontal_line(int8_t y, int8_t startx, int8_t endx);
void draw_vertical_line(int8_t x, int8_t starty, int8_t endy);

#endif /* TERMINAL_IO_H */