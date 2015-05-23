/*
 * timer2.c
 *
 * Author: Thuan Song Teoh
 *
 * We setup timer2 to generate an interrupt every 1ms
 * Tune played is determined via the interrupt handler and
 * get_timer1_clock_ticks().
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer2.h"
#include "timer1.h"

// Frequency of beep
volatile static int16_t num;

// Number of times buzzer beeped
volatile static int8_t sounded;

// Current time (from timer 1)
volatile static uint32_t current_time;

// Previous time (from timer 1)
volatile static uint32_t prev_time;

// 1 if tune is still playing, 0 if not playing
volatile static uint8_t playing = 0;

/* Type of sound:
 * sounds[0][n] - No sound
 * sounds[1][n] - Lap complete sound
 * sounds[2][n] - Game over sound
 * sounds[3][n] - Power-up sound
 *
 * Structure of sound info:
 * sounds[n][0] - Number of beeps
 * sounds[n][1] - Length of beep (sounds[n][2] * 10ms)
 * sounds[n][2] - Length before next beep (sounds[n][2] * 10ms)
 * sounds[n][3] - Frequency of beep (must not get below 487Hz)
 */
static uint16_t sounds[4][4] =  { { 0 }, { 2, 3, 5, 4000 }, { 1, 10, 10, 3000 }, { 3, 2, 5, 2000 } };

// Current tune
volatile static uint16_t* cur_sound;

/* Helper function to extract a bit from a value.
 * Thanks to Marcus Herbert (posted in Piazza).
 */
uint8_t get_bit(uint8_t value, uint8_t index) {
	/* Declare an unsigned 8 bit integer (this should match the return value type)
	 * and do the operation discussed above in the note.
	 */

	uint8_t bit = !!(value & (1 << index));
	return bit;
}

void set_sound_type(uint8_t type) {
	cur_sound = sounds[type];
	if(cur_sound[0]) {
		num = cur_sound[3];
		OCR2A = F_CPU / 64 / num - 1;
		sounded = 0;
		playing = 1;
		prev_time = get_timer1_clock_ticks();
	} else {
		// No tune, so not playing
		playing = 0;
		OCR2A = F_CPU / 64 / 2000 - 1;
	}
}

void init_timer2(void) {
	/* Clear the timer */
	TCNT2 = 0;

	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR2A = (1<<WGM21);
	TCCR2B = (1<<CS22);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK2 |= (1<<OCIE2A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR2 &= (1<<OCF2A);

	/* Set the output compare value for 2kHz */
	OCR2A = F_CPU / 64 / 2000 - 1;

	/* Set Port D pin 3 to be output */
	DDRD = 1<<3;

	/* Turn sound off at first */
	set_sound_type(0);
}

uint8_t is_sound_playing(void) {
	return playing;
}

ISR(TIMER2_COMPA_vect) {
	if(cur_sound[0]) {
		current_time = get_timer1_clock_ticks();
		if(sounded < cur_sound[0]) {
			if(current_time <= prev_time + cur_sound[1]){
				// Beep until specified length
				if(!get_bit(PIND,2)) {
					// Toggle bit
					PORTD ^= 1<<3;
				} else {
					// Muted, clear bit
					PORTD &= ~(1<<3);
				}
			} else if(current_time >= prev_time + cur_sound[2]) {
				// Initiate next beep
				prev_time = get_timer1_clock_ticks();
				sounded++;
			} else {
				// Clear bit, not beeping for now
				PORTD &= ~(1<<3);
			}
		} else {
			// Tune completed, reset bit
			playing = 0;
			PORTD &= ~(1<<3);
		}
	} else {
		// Not playing any tune, clear bit
		PORTD &= ~(1<<3);
	}
}