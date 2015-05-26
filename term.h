/*
 * term.h
 *
 * Author: Thuan Song Teoh
 */


#ifndef TERM_H_
#define TERM_H_

#include <stdint.h>

void term_redraw_background(void);
void term_redraw_game_row(uint8_t row);
void term_draw_start_or_finish_line(uint8_t row);
void term_redraw_car(uint8_t colr, uint8_t column);
void term_erase_car(uint8_t bg1, uint8_t bg2, uint8_t column);
void term_draw_powerup(uint8_t column);

#endif /* TERM_H_ */