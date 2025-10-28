#pragma once
// this is a configuration file for this debug module.  Use this file to enable or disable uart, interrupts, timers or gpio.
#define en_gpio_int 1 // gpio is enabled
#define en_uart_int 1 // uart is enabled
#define en_timer_int 1
#define en_uart_init 0 // use this to indicate if this code should initialize uart
#define en_uart_out 1 // if you do not have an implementation of uart_out, disable this
#define separate_alist_file 1
#define portdpoll 1	// poll on port d instead of using interrupts
#define test_new_int2str 0

#define gotoend 1
#define debug 0
#define newdm 0
