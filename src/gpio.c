/*
 * gpio.c
 *
 *  Created on: Dec 1, 2025
 *      Author: nfox
 */

// main GPIO initialization function, only supports APB
// TODO: Add support for GPIO AFSEL
#include "gpio.h"
#include "library.h"
#include <stdio.h>

void gpio_init(gpio_port port, uint32_t pin, uint32_t alternate_func_select, bool analogue_select, bool intteruptToggle){

  
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

  

}