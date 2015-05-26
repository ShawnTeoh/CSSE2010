/*
 * leaderboard.h
 *
 * Author: Thuan Song Teoh
 */

#ifndef LEADERBOARD_H_
#define LEADERBOARD_H_

#include <stdint.h>

// Signature to determine if memory initialised
#define SIGNATURE 0xBAFF

// Maximum number of high scores
#define MAX_NUM 5

// Keyboard character ASCII constants
#define ESCAPE_CHAR 27
#define BACK_SPACE 127

// Structure to store high scores
typedef struct {
	uint16_t signature;
	char name[6];
	uint32_t score;
} Highscore;

/* Read values from EEPROM.
 */
void retrive_leaderboard(void);

/* Check if current score can be included in leader board,
 * prompt for player initials if yes.
 */
void is_highscore(void);

/* Function to print the formatted leaderboard in terminal.
 */
void leaderboard_terminal_output(void);

#endif /* LEADERBOARD_H_ */