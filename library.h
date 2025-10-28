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
#include <stdbool.h>

uint32_t int2string(int, char*);

float fexp(float, uint32_t );

uint32_t bitfield(uint32_t bits[], uint32_t);

// properly offset a uint32_t pointer
#define uptradd(ptr, offset) ((uint32_t *)(ptr + offset))
// get length of array
#define len(array) (sizeof(array) / sizeof(*array)) // type agnostic

#endif /* LIBRARY_H_ */
