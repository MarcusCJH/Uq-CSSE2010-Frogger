/*
 * joystick.h
 *
 * Created: 30/5/2018 1:38:38 PM
 *  Author: Marcus
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_


void init_joystick_interrupts(void);

int8_t joystick_pushed(void);


#endif /* JOYSTICK_H_ */