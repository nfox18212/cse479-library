/*
 * gpio.h
 *
 *  Created on: Dec 1, 2025
 *      Author: nfox
 */

#ifndef INCLUDE_GPIO_H_
#define INCLUDE_GPIO_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    portA,
    portB,
    portC,
    portD,
    portE,
    portF
} gpio_port;

void gpio_init(gpio_port, uint32_t, bool, uint32_t, bool, bool);

uint32_t gpio_read(gpio_port);

#endif /* INCLUDE_GPIO_H_ */
