/*
 * timer1.h
 *
 * Author: Thuan Song Teoh
 *
 * We set up timer 1 to give us an interrupt
 * every 100 milliseconds. Tasks that have to occur
 * regularly (every 100 milliseconds or few) can be added
 * to the interrupt handler (in timer1.c) or can
 * be added to the main event loop that checks the
 * clock tick value. This value (16 bits) can be
 * obtained using the get_lap_timer() function.
 * (Any tasks undertaken in the interrupt handler
 * should be kept short so that we don't run the
 * risk of missing an interrupt in future.)
 */ 

#ifndef TIMER1_H_
#define TIMER1_H_

#include <stdint.h>

/* Set up our timer to give us an interrupt every 100 milliseconds
 * and update our time reference.
 */
void init_timer1(void);

/* Return the current clock tick value - in increments of 100 milliseconds since the timer was
 * initialised.
 */
uint16_t get_lap_timer(void);

/* Reset/start lap timer.
 * Supply 1 to reset timer, 0 to restart stopped timer.
 */
void start_lap_timer(uint8_t status);

/* Stop lap timer.
 */
void stop_lap_timer(void);

/* Toggle timer counter on/off.
 */
void toggle_timer1(void);

#endif /* TIMER2_H_ */