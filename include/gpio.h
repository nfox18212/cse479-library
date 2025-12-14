/*
 * gpio.h
 *
 *  Created on: Dec 1, 2025
 *      Author: nfox
 */

#ifndef INCLUDE_GPIO_H_
#define INCLUDE_GPIO_H_


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "library.h"

typedef enum {
    portA,
    portB,
    portC,
    portD,
    portE,
    portF
} gpio_port;

void _gpio_init(gpio_port port, uint32_t* pins, size_t pinnum, bool output, uint32_t alternate_func_select, bool analogue_select, bool interrupt);

// hacky workaround to not require user to specifiy pinnum
#define gpio_init(port, pin_array, output, alt_func_select, analogue_select, interruptToggle) _gpio_init(port, pin_array, len(pin_array), output, alt_func_select, analogue_select, interruptToggle)

uint8_t gpio_read(gpio_port, int32_t pin, ...);
uint8_t gpio_write(gpio_port, uint8_t data);

#endif /* INCLUDE_GPIO_H_ */
