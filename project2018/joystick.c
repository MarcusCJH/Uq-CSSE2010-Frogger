/*
 * joystick.c
 *
 * Created: 30/5/2018 1:07:03 PM
 *  Author: Marcus
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer0.h"
#include "joystick.h"
#include "buttons.h"
#include "stdio.h"





// Global variable to keep track of the last button state so that we
// can detect changes when an interrupt fires. The lower 4 bits (0 to 3)
// will correspond to the last state of port B pins 0 to 3.
static volatile uint8_t last_joystick_state;
static uint16_t value;
static uint8_t x_or_y = 0;	/* 0 = x, 1 = y */

// Our button queue. button_queue[0] is always the head of the queue. If we
// take something off the queue we just move everything else along. We don't
// use a circular buffer since it is usually expected that the queue is very
// short. In most uses it will never have more than 1 element at a time.
// This button queue can be changed by the interrupt handler below so we should
// turn off interrupts if we're changing the queue outside the handler.
#define JOYSTICK_QUEUE_SIZE 2
static volatile uint8_t joystick_queue[JOYSTICK_QUEUE_SIZE];
static volatile int8_t joystick_queue_length;

void init_joystick_interrupts(void)
{
	ADMUX = (1<<REFS0);
	ADCSRA = ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	DDRA = 0xff;
	
	joystick_queue_length = 0;
}


int8_t joystick_pushed(void) {
	int8_t return_value = NO_BUTTON_PUSHED;	// Assume no button pushed
	if(joystick_queue_length > 0) {
		// Remove the first element off the queue and move all the other
		// entries closer to the front of the queue. We turn off interrupts (if on)
		// before we make any changes to the queue. If interrupts were on
		// we turn them back on when done.
		return_value = joystick_queue[0];
		
		// Save whether interrupts were enabled and turn them off
		int8_t interrupts_were_enabled = bit_is_set(SREG, SREG_I);
		cli();
		
		for(uint8_t i = 1; i < joystick_queue_length; i++) {
			joystick_queue[i-1] = joystick_queue[i];
		}
		joystick_queue_length--;
		
		if(interrupts_were_enabled) {
			// Turn them back on again
			sei();
		}

	}
	return return_value;
}

ISR(ADC_vect){
	
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	value = ADC; // read the value
	if(x_or_y == 0) {
		printf("X: %4d ", value);
		} else {
		printf("Y: %4d\n", value);
	}
	
	// Set the ADC mux to choose ADC0 if x_or_y is 0, ADC1 if x_or_y is 1
	x_or_y ^= 1;
	if(x_or_y == 0) {
		ADMUX &= ~1;
		} else {
		ADMUX |= 1;
	}
	
	// Begin processing again
	ADCSRA |= (1<<ADSC);
	
}