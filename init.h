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
void serial_init();
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


typedef struct pwm_options{



} pwmopt;



typedef struct timer_options{
    int width; // can only be 16, 32, or 64
    int capture_mode; 
    bool pwm;    // pwm definitions struct?
    pwmopt pwm_options;
    uint32_t load; // add error checking for if the user passes in a number that's too big for the given width



} topt;

    




// void gpio_init(gpio_port port, int pins[8]);
void gpio_interrupt_init();
void init_udma(uint32_t* udma_ptr, uint32_t *source_ptr, uint32_t* dst_ptr);

void ssi_init();

void adc_init();

// implemented in assembly
extern void gpio_init(void);
extern void timer_init(void);



#endif /* INIT_H_ */
