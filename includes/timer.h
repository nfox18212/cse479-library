/*
 * timer.h
 *
 *  Created on: Nov 16, 2025
 *      Author: njfox4
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "pwm.h"
#include "udma.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum timer_number {

  timer_0 = 0x1000,
  timer_1,
  timer_2,
  timer_3,
  timer_4,
  timer_5,
  wtimer_0 = 0x2000,
  wtimer_1,
  wtimer_2,
  wtimer_3,
  wtimer_4,
  wtimer_5

} timer_number;


typedef enum timer_mode {
  // wrapper for the different capture modes.  using non-default increments to
  // avoid enum collision

  one_shot = 0x10,
  periodic = 0x20,
  RTC = 0x30,
  edge_count = 0x40,
  edge_time = 0x50,
  PWM = 0x60

} timer_mode;

typedef struct timer_options {
  // only include extra options, like options for connecting to pwm or UDMA

  bool enable;

  // which half is being examined.  Valid options are a, b, or 0 if it is not being used
  char timer_half;

  uint8_t prescaler;
  // need to do some thinking about what's the best way to expose different
  // options for all the myriad things that the general purpose timers can
  // connect.  thinking something like a c version of classes.

  bool pwm; // pwm definitions struct?
  pwmopt pwm_options;




  // udma options

  // ADC options

  timer_mode __internal_mode;
} timer_options;


// initializes ONE timer, if multiple timers should be configured this function
// should be called multiple times.
int timer_cinit(timer_number t, uint32_t load, int width, int direction,
                 bool interrupt, timer_mode m, timer_options topt);

#endif