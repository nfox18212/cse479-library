/*
 * library.h
 *
 *  Created on: Sep 22, 2025
 *      Author: njfox4
 */
 



#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>


// An example of classes being rather useful
typedef struct color{

    // red, blue, green duty cycles - dc is clamped between 0 and 1
    float red_dc;
    float blue_dc;
    float green_dc;
    float frequency;
    
} color;


typedef enum pwm_gen {
    PWM00A,
    PWM00B,
    PWM01A,
    PWM01B,
    PWM02A,
    PWM02B,
    PWM03A,
    PWM03B,
    PWM10A,
    PWM10B,
    PWM11A,
    PWM11B,
    PWM12A,
    PWM12B,
    PWM13A,
    PWM13B,
    PWM14A,
    PWM14B
} pwm_gen; // avoid having to type enum out again

#define PWMCLK = 8000000 // set frequency of the PWM clock

#endif