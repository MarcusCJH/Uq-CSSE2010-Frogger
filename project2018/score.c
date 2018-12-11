/*
 * score.c
 *
 * Written by Peter Sutton
 */
#include "terminalio.h"
#include "score.h"
#include <avr/pgmspace.h>
#include <stdio.h>

uint32_t score;

void init_score(void) {
	score = 0;
}

void add_to_score(uint16_t value) {
	score += value;
	move_cursor(72, 1);
	printf_P(PSTR("Score: %i"), get_score());
}

uint32_t get_score(void) {
	return score;
}


