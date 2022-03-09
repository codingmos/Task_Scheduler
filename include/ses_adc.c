#include "avr/io.h"
#include "ses_common.h"
#include "ses_adc.h"

//Peripherals
#define PERIPHERALS_PORT            PORTF

//PRR0- Power Reduction Register
#define PRADC_PIN						1

//ADMUX Port ==> setting REFS0 & REFS1 to 0 ==> 1.6V reference voltage, right-adjusted ==> ADLAR = 0
#define ADC_VREF_SRC			    ADMUX
#define ADLAR_PIN						5
#define REFS0_PIN						6
#define REFS1_PIN						7

//ADCSRA Port ==> setting pins: ADPS0 = 1, ADPSS1 = 1, S2 = 0 creates a prescale factor of 8
#define ADPS0_PIN						0
#define ADPS1_PIN						1
#define ADPS2_PIN						2
#define ADATE_PIN						5
#define ADSC_PIN						6
#define ADEN_PIN						7

//ADC Register
#define ADCL_DDR				  	 ADCL
#define ADCH_DDR					 ADCH

//Temperature constants
int ADC_TEMP_MAX = 313;
int ADC_TEMP_MIN = 293;
//uint16_t ADC_TEMP_RAW_MAX = 10*exp((3250/ADC_TEMP_MAX)-(65000/5963));
//uint16_t ADC_TEMP_RAW_MIN = 10*exp((3250/ADC_TEMP_MIN)-(65000/5963));
#define ADC_TEMP_FACTOR 100;

int decision = -1;

void adc_init(void){

	// 1- Deactivating the pullup resistor and configuring the DDRF
	PERIPHERALS_PORT &= ~((1<<ADC_MIC_NEG_CH) | (1<<ADC_MIC_POS_CH) | (1<<ADC_TEMP_CH) | (1<<ADC_LIGHT_CH) | (1<<ADC_JOYSTICK_CH));
	DDR_REGISTER(PERIPHERALS_PORT) &= ~((1<<ADC_MIC_NEG_CH) | (1<<ADC_MIC_POS_CH) | (1<<ADC_TEMP_CH) | (1<<ADC_LIGHT_CH) | (1<<ADC_JOYSTICK_CH));;

	// 2- Disable power reduction mode
	PRR0 &= ~(1 << PRADC_PIN);

	// 3 to 5- Sets VREF = 1.6V and Right-Adjusted
	ADC_VREF_SRC |= ((1 << REFS0_PIN) | (1 << REFS1_PIN));
	ADC_VREF_SRC &= ~(1 << ADLAR_PIN);

	// 6 to 7- Set ADC prescaler to maximum operation, 2 MHz ==> factor of 8
	ADC_PRESCALE &= ~((1 << ADPS0_PIN) | (1 << ADPS1_PIN));
	ADC_PRESCALE |= (1 << ADPS2_PIN);

	// 8- ADATE bit in ADCSRA should be 0
	ADC_PRESCALE &= ~(1 << ADATE_PIN);

	// 9- enable ADC; ADEN bit
	ADC_PRESCALE |= (1 << ADEN_PIN);
}


uint16_t adc_read(uint8_t adc_channel) {

	// check for invalid channel
	if ((adc_channel < 0) || (adc_channel >= ADC_NUM)) {
		return ADC_INVALID_CHANNEL;
	}

	// select the corresponding channel 0~7
	// ANDing with 7 will always keep the value
	// of adc_channel between 0 and 7
	// this can be changed if number of channels changes
	adc_channel &= 0b00000111;  // AND operation with 7
	ADMUX = ((ADMUX & 0xE0)|adc_channel); // clears the bottom 5 bits before ORing


	// start single conversion
	// write 1 to ADSC
	ADCSRA |= (1<<ADSC);

	// wait for conversion to complete
	// ADSC becomes 0 again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));

	// create 16 bit register
	int ADC_RESULT = 0x0000;
	// add lower part of register
	ADC_RESULT |= ADCL;
	// add high part of register shifted to last 8 bits of 16 bit register
	ADC_RESULT |= (ADCH << 8);

	return (ADC_RESULT);
}

uint8_t adc_getJoystickDirection() {
	uint16_t adc = adc_read(ADC_JOYSTICK_CH);
	if (adc > 100 || adc < 300) {
		decision = RIGHT;
	} else if (adc > 300 || adc < 500) {
		decision = UP;
	} else if (adc > 500 || adc < 700) {
		decision = LEFT;
	} else if (adc > 700 || adc < 900) {
		decision = DOWN;
	} else if (adc > 900 || adc < 1100) {
		decision = NO_DIRECTION;
	} return decision;
}
/*
int16_t adc_getTemperature() {
	int16_t adc = adc_read(ADC_TEMP_CH);
	int16_t slope = ADC_TEMP_FACTOR*(ADC_TEMP_MAX - ADC_TEMP_MIN) / (ADC_TEMP_RAW_MAX - ADC_TEMP_RAW_MIN);
	int16_t offset = ADC_TEMP_MAX - (ADC_TEMP_RAW_MAX * slope);
	return (adc * slope + offset) / ADC_TEMP_FACTOR;
}*/
