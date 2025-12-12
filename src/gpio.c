/*
 * gpio.c
 *
 *  Created on: Dec 1, 2025
 *      Author: nfox
 */

// main GPIO initialization function, only supports APB
#include "gpio.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>


static address portToAddr(gpio_port p){
// set RCGC GPIO and base address
address base;
  switch(p){
    case portA:
      base = 0x40004000;
      break;
    case portB:
      base = 0x40005000;
      break;
    case portC:
      base = 0x40006000;
      break;
    case portD:
      base = 0x40007000;
      break;
    case portE:
      base = 0x40024000;
      break;
    case portF:
      base = 0x40025000;
      break;
    default:
      fprintf(stderr, "Attempt to use GPIO port that does not exist\n");
      return -1;
      
  }

  return base;
}

void _gpio_init(gpio_port port, uint32_t pins[], size_t pinnum, bool output, uint32_t alternate_func_select, bool analogue_select, bool interruptToggle){

  // Determining the size of an array inside a function is an unsolvable problem

  bitfield pin_mask = _make_array_bitfield(pins, pinnum);
  
  address *RCGC = (address *) 0x400FE608;
  address base = portToAddr(port);

// set RCGC GPIO and base address
  switch(port){
    case portA:
      *RCGC |= (1 << 0);
      break;
    case portB:
      *RCGC |= (1 << 1);
      break;
    case portC:
      *RCGC |= (1 << 2);
      break;
    case portD:
      *RCGC |= (1 << 3);
      break;
    case portE:
      *RCGC |= (1 << 4);
      break;
    case portF:
      *RCGC |= (1 << 5);
      break;
    default:
      fprintf(stderr, "Attempt to use GPIO port that does not exist\n");
      return;
  }

  // set direction of the pins on the gpio port
  *uptradd(base, 0x400) |= output * pin_mask;

  if(alternate_func_select != 0){
    // gpio afsel stuff
    // TODO: Implement alternate function
  }

  if(analogue_select){      *RCGC |= (1 << 5);
    // TODO: Implement analogue 
  } else {
    // GPIO digital enable
    *uptradd(base, 0x51C) = pin_mask; 
  }

}

uint8_t gpio_read(gpio_port p, int32_t pin, ...){

  //  reads pins from the specified gpio_port, the last pin needs to be -1.  pin numbers greater than 8 will return an error
  
  address base = portToAddr(p);
  uint32_t gpio_data = * (address *) base;
  uint8_t ret = gpio_data & (1 << pin); 
  va_list pins;
  va_start(pins, pin);

  do {
    int8_t currentPin = va_arg(pins, uint32_t);
    if(currentPin == -1){
      va_end(pins);
      break;
    }

    if(currentPin >= 8){
      fprintf(stderr, "Attempt to access reserved pin %i!  Returning 0",currentPin);
      // return garbage data
      va_end(pins);
      return 255;

    }
    ret += gpio_data & (1 << currentPin);
  } while(1);


  return ret;
}

uint8_t gpio_write(gpio_port p, uint8_t data){
  // Write data to the specified port and returns the data written
  address base = portToAddr(p);
  *uptradd(base, 0) = data;

  // return the data written to the gpio port
  return *uptradd(base, 0);
}