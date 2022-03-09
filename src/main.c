#include "ses_led.h"
#include "ses_led.c"
#include "ses_button.h"
#include "ses_button.c"
#include "ses_timer.h"
#include "ses_timer.c"
#include "ses_scheduler.h"
#include "ses_scheduler.c"
#include "util/delay.h"
#include "ses_adc.h"
#include "ses_adc.c"
#include "avr/io.h"
#include "ses_uart.h"
#include "ses_lcd.h"

void leds_init() {
	led_greenInit();
	led_redInit();
	led_yellowInit();
	led_redOff();
	led_greenOff();
	led_yellowOff();
}

void interrupt_init() {
	//Enabling interrupt at all
	//INTERRUPT_DDR |= ((1 << PCIE0_BIT) | (1 << PCIE1_BIT) | (1 << PCIE2_BIT));
	//Enabling triggering interrupt if a button is pressed
	//PIN_MASK_DDR |= (1 << BUTTON_JOYSTICK_PIN);
	//PIN_MASK_DDR |= (1 << BUTTON_ROTARY_PIN);
}

//*** Task Functions ***//

/* void ledParams()
 * -Gives yellow, green, and red led options to user based off input of color
 * and toggles led accordingly
 */
bool led_on_off = false;
void ledToggle(void* colorchoice) {
	int* choicetemp = colorchoice;
	int choice = *choicetemp;
	if (choice == 1) {
		if (led_on_off == false) {
			led_greenOn();
			led_on_off = true;
		} else {
			led_greenOff();
			led_on_off = false;
		}
	} else if (choice == 2) {
		if (led_on_off == false) {
			led_redOn();
			led_on_off = true;
		} else {
			led_redOff();
			led_on_off = false;
		}
	} else if (choice == 3) {
		if (led_on_off == false) {
			led_yellowOn();
			led_on_off = true;
		} else {
			led_yellowOff();
			led_on_off = false;
		}
	} else {
		//Can be built for invalid error on color choice
	}

}

/* void button_doJoystickLedCallback()
 * -Toggles yellow led based on joystick button press or turns off after 5 seconds
 */
int joystickCounter = 2000;

int led_on = 0;
void buttonLedDebounceFlag() {
	if (led_on == 0) {
		led_on = 1;
	} else if (led_on == 1) {
		led_on = 0;
	}
}

void button_doJoystickLedCallback() {
	button_setJoystickButtonCallback(&buttonLedDebounceFlag);
	if (led_on == 0) {
			led_yellowOff();
			joystickCounter = 0;
	} else if (led_on == 1) {
		joystickCounter++;
		led_yellowOn();
		if (joystickCounter == 995) {
			led_yellowOff();
			led_on = 0;
		}
	}
}

/* button_doRotaryStopwatchCallback()- Starts and stops a stopwatch utilizing the following functions:
 * timerOnOff()- sets timer to start or pause via the do_RotaryStopwatchCallback() function
 * timerStart()- starts counter and outputs to lcd in seconds and tenths of a second
 * timerPause()- pauses timer on rotary press and outputs current timer value until rotary pressed again
 * do_RotaryStopwatchCallback()- calls timer_on to be true or false & passes this
 * information to button_doRotaryStopwatchCallback()
 */
int timer_on = 0;
void timerOnOff() {
	if (timer_on == 0) {
		timer_on = 1;
	} else if (timer_on == 1) {
		timer_on = 0;
	}
}

int countsecs = 0;
int counttenths = 0;
int values[2];
int* timerStart() {
	counttenths++;
	if (counttenths == 10) {
		counttenths = 0;
		countsecs++;
	}

	lcd_setCursor(1, 0);
	fprintf(uartout, "%d.%d seconds", countsecs, counttenths);
	fprintf(lcdout, "%d.%d seconds", countsecs, counttenths);
	values[0] = countsecs;
	values[1] = counttenths;
	return values;
}

void timerPause(int array[2]) {
	int pcountsecs = array[0];
	int pcounttenths = array[1];
	lcd_setCursor(1, 0);
	fprintf(uartout, "%d.%d seconds", pcountsecs, pcounttenths);
	fprintf(lcdout, "%d.%d seconds", pcountsecs, pcounttenths);

}

int* temptime = NULL;
void button_doRotaryStopwatchCallback() {
	if (timer_on == 0) {
		if (temptime == NULL) {
			temptime[0] = 0;
			temptime[1] = 0;
		}
		timerPause(temptime);
	} else if (timer_on == 1) {
		temptime = timerStart();
	}
}

void test() {
	led_redOn();
}

int main(void) {
// *** Initialize timer, interrupt, scheduler, leds, lcd, and buttons *** //
	uart_init(57600);
	lcd_init();
	leds_init();
	button_init();
	//interrupt_init();
	scheduler_init();
	button_setRotaryButtonCallback(&timerOnOff);

// *** Initialize tasks *** //

	// Task 1 - Initialize led toggling function
	taskDescriptor ledToggler;
	// Set initial period for execution
	ledToggler.expire = 0;
	// Set choice of color from ledParams and point to correct toggle function
	enum Colors {
		green = 1, red = 2, yellow = 3
	};
	int color = green;
	ledToggler.param = &color;
	// Set periodic repeat time to 2 seconds
	ledToggler.period = 2000;
	// Point to correct function task for led toggler
	ledToggler.task = &ledToggle;

	// Task 2 - Initialize debouncer function
	taskDescriptor debouncer;
	// Set initial period for execution
	debouncer.expire = 0;
	// Set param variable null
	debouncer.param = NULL;
	// Set periodic repeat time to 5 ms
	debouncer.period = 5;
	// Point to correct function task for debouncing
	debouncer.task = &button_checkState;

	// Task 3 - Initialize joystick/yellow led function
	taskDescriptor joystickLed;
	// Set function param to run when joystick button pressed
	joystickLed.param = NULL;
	// Set period in which led is turned off if no press
	joystickLed.period = 5;
	// Set initial expire for first button press, set to one to avoid negative in update function
	joystickLed.expire = 0;
	// Point to correct function task for debouncing
	joystickLed.task = &button_doJoystickLedCallback;

	// Task 4 - Initialize stopwatch function
	taskDescriptor stopwatch;
	// Set function param to run clock function upon firing
	stopwatch.param = NULL;
	// Set period in which stopwatch function fires output to lcd screen
	stopwatch.period = 100;
	// Set initial expire for first button press, set to one to avoid negative in update function
	stopwatch.expire = 0;
	// Point to correct function for timer start/stop
	stopwatch.task = &button_doRotaryStopwatchCallback;

// *** Add tasks to scheduler *** //

	// Task 1 - led toggler
		scheduler_add(&ledToggler);
	// Task 2 - debouncer
	scheduler_add(&debouncer);
	// Task 3 - joystick/yellow led function
    scheduler_add(&joystickLed);
	// Task 4
	scheduler_add(&stopwatch);


	while (1) {
		scheduler_run();

	}

	return 0;

}
