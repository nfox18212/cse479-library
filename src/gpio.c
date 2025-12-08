/*
 * gpio.c
 *
 *  Created on: Dec 1, 2025
 *      Author: nfox
 */

// main GPIO initialization function, only supports APB
#include "gpio.h"
#include "library.h"
#include <stdio.h>

void _gpio_init(gpio_port port, uint32_t pins[], size_t pinnum, bool output, uint32_t alternate_func_select, bool analogue_select, bool interruptToggle){

  // Determining the size of an array inside a function is an unsolvable problem

  bitfield pin_mask = _make_array_bitfield(pins, pinnum);
  
  address *RCGC = (address *) 0x400FE608;
  address base;

// set RCGC GPIO and base address
  switch(port){
    case portA:
      base = 0x40004000;
      *RCGC |= (1 << 0);
      break;
    case portB:
      base = 0x40005000;
      *RCGC |= (1 << 1);
      break;
    case portC:
      base = 0x40006000;
      *RCGC |= (1 << 2);
      break;
    case portD:
      base = 0x40007000;
      *RCGC |= (1 << 3);
      break;
    case portE:
      base = 0x40024000;
      *RCGC |= (1 << 4);
      break;
    case portF:
      base = 0x40025000;
      *RCGC |= (1 << 5);
      break;
    default:
      fprintf(stderr, "Attempt to use GPIO port that does not exist\n");
      
  }

  // set direction of the pins on the gpio port
  *uptradd(base, 0x400) |= output * pin_mask;

  if(alternate_func_select != 0){
    // gpio afsel stuff
    // TODO: Implement alternate function
  }

  if(analogue_select){
    // TODO: Implement analogue 
  } else {
    // GPIO digital enable
    *uptradd(base, 0x51C) = pin_mask; 
  }

}