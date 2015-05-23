/*
 * timer2.h
 *
 * Author: Thuan Song Teoh
 * We set up timer 2 to give us an interrupt
 * every millisecond. Buzzer is manipulated via the 
 * interrupt handler (in timer2.c).
 */

#ifndef TIMER2_H_
#define TIMER2_H_

#include <stdint.h>

/* Set up our timer.
 */
void init_timer2(void);

/* Returns 1 if sound is playing.
 */
uint8_t is_sound_playing(void);

/* Toggle sound according to specified type.
 */
void set_sound_type(uint8_t type);

#endif /* TIMER2_H_ */