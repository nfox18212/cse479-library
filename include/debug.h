#pragma once

#include "debug_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define clearscreen 0xC  // form feed character to clear screen
#define cnewline 0xDA     // carriage return and newline
#define newline 0xA
#define carreturn 0xD
#define clear 0xC

#define lab7_globals {\
     .global init\
	.global UART0_Handler\
	.global Switch_Handler\
	.global Timer_Handler\
	.global read_character\
	.global output_character \
	.global read_string		\
	.global output_string	\
	.global lab7\
	.global set_color\
	.global get_color\
	.global get_cell\
	.global dirindex\
	.global rcd\
	.global extract_cid\
	.global new_o\
	.global check_board_state\
	.if include_debug=1\
	.global crash\
	.endif\
	.global	seed\
	.global update\
	.global end_game\
	}


#define include_debug 1
#define sw1mask	0xEF
#define sw1write 0x10
#define uartwrite 0x20
#define star 0x2A


// these two functions are implemented in assembly
extern void goback(uint32_t addr);     // debug function to jump back to assembly and we can go to a faulted instruction
extern void ngoback(uint32_t addr);    // debug function to jump back to assembly if we encounter a fault that was not casued by the crash subroutine
extern void output_string(char *str);  // print to uart
extern void output_character(char a);


// fault handling functions
uint32_t handle_memfault(uint32_t mmsr);
uint32_t handle_busfault(uint32_t bfsr);
// there's no ufar register like there is for memory or bus faults

// error message printing
void mmsrprint(uint32_t mmsr);
void bfsrprint(uint32_t bfsr);
void ufsrprint(uint32_t ufsr);

void handlefault(void);
void customfault(uint32_t baddr);
void normalfault(void);
void serial_init(void);

// assembly routines to get didcrashp and baddrp
extern uint32_t getbaddr(void);
extern uint8_t getdidcrash(void);
extern int test(int var);

// printing addresses
char *addrtostring(uint32_t addr);

// fault stack frame
struct frame {
	uint32_t r0; //r0
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t usrlr; // user's link register
	uint32_t pc; // program counter
	uint32_t xpsr; // execution program status register
};

#ifdef __clang__
#define IGNORE_INITIALIZER_WARNING(...)                                       \
    _Pragma("clang diagnostic push")                                          \
    _Pragma("clang diagnostic ignored \"-Winitializer-overrides\"")           \
    __VA_ARGS__                                                               \
    _Pragma("clang diagnostic pop")

#else
#define IGNORE_INITIALIZER_WARNING(...) __VA_ARGS__                                                                
#endif