/*
 * init.h
 *
 *  Created on: Sep 27, 2025
 *      Author: nfox
 */

#ifndef INIT_H_
#define INIT_H_

#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>


//void ctimer_init(int); // int will be period

void uart_iterrupt_init();

void pwm_init();

typedef enum gpio_ports{
    port_a,
    port_b,
    port_c,
    port_d,
    port_e,
    port_f
} gpio_port;


// void gpio_init(gpio_port port, int pins[8]);
void gpio_interrupt_init();
void init_udma(uint32_t* udma_ptr, uint32_t *source_ptr, uint32_t* dst_ptr);

void ssi_init();

void adc_init();



#endif /* INIT_H_ */
