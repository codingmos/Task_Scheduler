#ifndef SES_ADC_H
#define SES_ADC_H

/*INCLUDES *******************************************************************/

#include "inttypes.h"
#include "avr/io.h"
#include "ses_common.h"

/* DEFINES & MACROS **********************************************************/

/* to signal that the given channel was invalid */
#define ADC_INVALID_CHANNEL    0xFFFF
#define ADC_PRESCALE		   ADCSRA

enum ADCChannels {
  ADC_MIC_NEG_CH=0,                     /* ADC0 */
  ADC_MIC_POS_CH=1,                     /* ADC1 */
  ADC_TEMP_CH=2,                        /* ADC2 */
  ADC_RESERVED1_CH=3,                   /* ADC3 */
  ADC_LIGHT_CH=4,                       /* ADC4 */
  ADC_JOYSTICK_CH=5,                    /* ADC5 */
  ADC_RESERVED2_CH=6,                   /* ADC6 */
  ADC_RESERVED3_CH=7,                   /* ADC7 */
  ADC_NUM=8                             /* number of ADC channels*/
};

enum JoystickDirections {
  RIGHT = 0,
  UP = 1,
  LEFT = 2,
  DOWN = 3,
  NO_DIRECTION = 4
};

/* FUNCTION PROTOTYPES *******************************************************/

/**
 * Initializes the ADC
 */
void adc_init(void);

/**
 * Read the raw ADC value of the given channel
 * @adc_channel The channel as element of the ADCChannels enum
 * @return The raw ADC value
 */
uint16_t adc_read(uint8_t adc_channel);

/**
 * Read the current joystick direction
 * @return The direction as element of the JoystickDirections enum
 */
uint8_t adc_getJoystickDirection();

/**
 * Read the current temperature
 * @return Temperature in tenths of degree celsius
 */
int16_t adc_getTemperature();

#endif /* SES_ADC_H */
