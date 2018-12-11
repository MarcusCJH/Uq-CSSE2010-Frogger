/*
 * FroggerProject.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by Marcus Chan Jun Hong
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "game.h"
#include "lives.h"
#include "joystick.h"

#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void handle_next_level(void);
void status_update(void);
void handle_game_completion(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27

// Pause status (0 resume, 1 pause)
static uint8_t paused = 0;

// Game Level
static uint8_t level;

// Game Over Flag
static uint8_t game_over_flag;


// Scroll direction swap
static int8_t direction_left = -1;
static int8_t direction_right = 1;

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
		new_game();
		while(!game_over_flag) {
			if(no_lives_left()) 
			{
				handle_game_over();
			} 
			if(!no_lives_left()) 
			{
				handle_next_level();
				play_game();
			}
			
		}
	
	
	
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	init_joystick_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	
	
	init_seven_seg();
	
	
	// Turn on global interrupts
	sei();
}

void splash_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_cursor(10,10);
	printf_P(PSTR("Frogger"));
	move_cursor(10,12);
	printf_P(PSTR("Project by Marcus Chan Jun Hong 45057377"));
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	while(1) {
		set_scrolling_display_text("Frogger 45057377", COLOUR_GREEN);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed
		while(scroll_display()) {
			_delay_ms(150);
			if(button_pushed() != NO_BUTTON_PUSHED) {
				return;
			}
		}
	}
}

void new_game(void) {
	
	level = 0;
	game_over_flag = 0;
	// Initialise the game and display
	//initialise_game();
	
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the score
	init_score();
	
	// Initialise the lives
	init_lives();
	
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	
	init_timer0();
	uint32_t current_time, last_move_time1, last_move_time2, last_move_time3,last_move_time4,last_move_time5;
	int8_t button;
	
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	
	// Get the current time and remember this as the last time the vehicles
	// and logs were moved.
	set_count((14+level));
	current_time = get_current_time();
	last_move_time1 = current_time;
	last_move_time2 = current_time;
	last_move_time3 = current_time;
	last_move_time4 = current_time;
	last_move_time5 = current_time;
	
	// XOR HAPPENS HERE
	direction_left = direction_left ^ direction_right;
	direction_right = direction_right ^ direction_left;
	direction_left = direction_left ^ direction_right;
	
	
	// We play the game while the frog is alive and we haven't filled up the 
	// far riverbank
	while(!no_lives_left() && !is_riverbank_full()) 
	{
		
		if(!is_frog_dead()) 
		{
			if(frog_has_reached_riverbank())
			{
				set_count((14+level));
				// Frog reached the other side successfully but the
				// riverbank isn't full, put a new frog at the start
				put_frog_in_start_position();
				
			}
			
		}
		
		if(is_count_over())
		{
			instant_dead();
			status_update();
			set_count((14+level));
		}
		
		
		if(is_frog_dead())
		{
			minus_lives();
			status_update();
			set_count((14+level));
			put_frog_in_start_position();
		}
		
		
		
		
		
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. At most one of the following three
		// variables will be set to a value other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		
		
		if(button == NO_BUTTON_PUSHED) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		if(!paused){
			// Process the input. 
			if(button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
				// Attempt to move left
				move_frog_to_left();
			} else if(button==2 || escape_sequence_char=='A' || serial_input=='U' || serial_input=='u') {
				// Attempt to move forward
				move_frog_forward();
			} else if(button==1 || escape_sequence_char=='B' || serial_input=='D' || serial_input=='d') {
				// Attempt to move down
				move_frog_backward();
			} else if(button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
				// Attempt to move right
				move_frog_to_right();
			} 
		}
		if(serial_input == 'p' || serial_input == 'P') {
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again
			paused = !paused;
			if(paused)
			{
				stop_counter();
				set_display_attribute(FG_MAGENTA);
				set_display_attribute(TERM_BRIGHT);
				move_cursor(10,16);
				printf_P(PSTR("Paused...                "));
				normal_display_mode();
				move_cursor(10, 17);
			}
			else
			{
				start_counter();
				set_display_attribute(FG_MAGENTA);
				set_display_attribute(TERM_BRIGHT);
				move_cursor(10,16);
				printf_P(PSTR("                   "));
				normal_display_mode();
				move_cursor(10, 17);
			}
		} 
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		
		current_time = get_current_time();
		if(!is_frog_dead()  && !paused) 
		{
			// 1000ms (1 second) has passed since the last time we moved
			// the vehicles and logs - move them again and keep track of
			// the time when we did this.
			
			//1
			if(current_time >= last_move_time1 + 750-50*level)
			{
				scroll_river_channel(0, direction_left);	
				last_move_time1 = current_time;	
			}
			//2
			if(current_time >= last_move_time2 + 950-50*level)
			{
				
				scroll_river_channel(1, direction_right);	
				last_move_time2 = current_time;	
			}
			//3
			if(current_time >= last_move_time3 + 1150-50*level)
			{	
				scroll_vehicle_lane(0, direction_right);
				last_move_time3 = current_time;	
			}
			//4
			if(current_time >= last_move_time4 + 1200-50*level)
			{	
				scroll_vehicle_lane(1, direction_left);
				last_move_time4 = current_time;	
			}
			//5
			if(current_time >= last_move_time5 + 1000-50*level)
			{
				scroll_vehicle_lane(2, direction_right);	
				last_move_time5 = current_time;	
			}
			
			//last_move_time = current_time;
		}
		
		
	}
	// We get here if the frog is dead or the riverbank is full
	if(is_riverbank_full())
	{
		return;
	}
}

void handle_game_over() {
		game_over_flag = 1;
		ledmatrix_clear();
		restart_count();
		clear_terminal();
		move_cursor(10,14);
		printf_P(PSTR("GAME OVER"));
		move_cursor(10,15);
		printf_P(PSTR("Press a button to start again"));
		/*while(button_pushed() == NO_BUTTON_PUSHED) {
			; // wait
		}*/
		while(1) {
			set_scrolling_display_text("GAME OVER", COLOUR_RED);
			// Scroll the message until it has scrolled off the
			// display or a button is pushed
			while(scroll_display()) {
				_delay_ms(150);
				if(button_pushed() == NO_BUTTON_PUSHED) {
					;
				}
			}
		}
	
	

}


void handle_next_level(void)
{
	restart_count();
	level++;
	if(level > 1) {
		add_lives();
	}
	
	if(level > 10)
	{
		handle_game_completion();
	}
	
	status_update();
	
	char str[8];
	sprintf(str, "LEVEL %i", level);
	set_scrolling_display_text(str,COLOUR_RED);
	// Scroll the message until it has scrolled off the
	// display or a button is pushed
	while(scroll_display()) {
		_delay_ms(75);
		if(button_pushed() == NO_BUTTON_PUSHED) {
			;
		}
	}
	
	_delay_ms(1000);
	initialise_game();
	
	
}

void handle_game_completion(void)
{
	
	clear_terminal();
	move_cursor(10,14);
	printf_P(PSTR("Congratulations"));
	move_cursor(10,15);
	printf_P(PSTR("You beat the game!"));
	while(1)
	{
		set_scrolling_display_text("Congratulation!",COLOUR_RED);
		// Scroll the message until it has scrolled off the
		// display or a button is pushed
		while(scroll_display()) {
			_delay_ms(250);
			if(button_pushed() != NO_BUTTON_PUSHED) {
				;
			}
		}
	
	}
	
	
}

void status_update(void)
{
	clear_terminal();
	move_cursor(10,13);
	printf_P(PSTR("Current Level %i"), level);
	move_cursor(10,14);
	printf_P(PSTR("Current Lives %i"), get_lives());
	move_cursor(10,15);
	printf_P(PSTR("Current Score %i"), get_score());	
}