/*
 * term.h
 *
 * Author: Thuan Song Teoh
 */

#ifndef TERM_H_
#define TERM_H_

#include <stdint.h>

/* All functions below are similar to their counterparts
 * in game.c, difference being terminal output instead of
 * LED matrix output. Should be called right after counterparts in
 * game.c.
 */
void term_redraw_game_row(uint8_t row);
void term_draw_start_or_finish_line(uint8_t row);
void term_redraw_car(uint8_t colr, uint8_t column);
void term_erase_car(uint8_t bg1, uint8_t bg2, uint8_t column);
void term_redraw_powerup(uint8_t row, uint8_t column, uint8_t colr);

#endif /* TERM_H_ */