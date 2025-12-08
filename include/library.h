/*
 * library.h
 *
 *  Created on: Sep 22, 2025
 *      Author: njfox4
 */
 



#ifndef LIBRARY_H_
#define LIBRARY_H_

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// New types
typedef volatile uint32_t address;
typedef uint32_t bitfield;

// Common Macros
// properly offset a uint32_t pointer
#define uptradd(ptr, offset) ((uint32_t *)(ptr + offset))
// get length of array
#define len(array) (sizeof(array) / sizeof(*array)) // type agnostic


// Common Functions 
bitfield make_bitfield(int32_t bits, ...);
bitfield _make_array_bitfield(uint32_t *bits, size_t num_bits);
uint32_t int2string(int, char*);

float fexp(float, uint32_t );

void enable_interrupt(int number);


// extern void init(void);
// TODO: Replace the assembly UART I/O functions with new ones.  Especially read_string to add backspace support.
extern float string2float(char*);
extern void float2string(float, char*, int); // in order to store data inside char* correctly, passing in the address to the character pointer is required so & char*
extern void read_string(char*);
extern char read_character(void);
extern void output_string(char *);
extern void output_character(char);




#endif /* LIBRARY_H_ */
