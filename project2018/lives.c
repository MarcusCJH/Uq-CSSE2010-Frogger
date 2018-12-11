/*
 * lives.c
 *
 * Created: 19/5/2018 9:55:18 PM
 *  Author: Marcus
 */ 

#include "lives.h"
#include <avr/pgmspace.h>
#include <stdio.h>
#include "terminalio.h"

uint8_t lives = 3;

// Initialize lives
void init_lives(void)
{
	DDRA = 0xF0;
	display_lives();
}

// Lose Lives
void minus_lives(void)
{
	// Check to see lives that must not be less than 0.
	if(lives >= 0)
	{
		lives--;
	}
	display_lives();
	
}

// Add Lives
void add_lives(void)
{
	if(lives < 4)
	{
		lives++;
	}
	display_lives();
	
	
}

// Getter properties
uint8_t get_lives(void) {
	return (lives);
}


uint8_t no_lives_left(void)
{
	return (lives == 0);
}

void set_lives(uint16_t value)
{
	lives = value;
}


// Restart Port according to life
void display_lives(void)
{	
	if(lives == 0)
	{
		PORTA = 0b0000000;
	}
	if(lives == 1)
	{
		PORTA = 0b10000000;
	}
	if(lives == 2)
	{
		PORTA = 0b11000000;
	}
	if(lives == 3)
	{
		PORTA = 0b11100000;
	}
	if (lives == 4)
	{
		PORTA = 0b11110000;
	}
}

