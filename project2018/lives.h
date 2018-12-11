/*
 * lives.h
 *
 * Created: 19/5/2018 9:56:07 PM
 *  Author: Marcus
 */


#ifndef LIVES_H_
#define LIVES_H_

#include <stdint.h>
#include <avr/io.h>

void init_lives(void);
void minus_lives(void);
void add_lives(void);
uint8_t get_lives(void);
uint8_t no_lives_left(void);
void set_lives(uint16_t value);
void display_lives(void);

#endif /* LIVES_H_ */