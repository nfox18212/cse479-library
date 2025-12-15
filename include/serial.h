/*
 * serial.h
 *
 *  Created on: Dec 1, 2025
 *      Author: nfox
 */

#ifndef INCLUDE_SERIAL_H_
#define INCLUDE_SERIAL_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_STR 512
void serial_init(void);

// borrowing the printf declaration
void uprintf(const char* str, ...);
// printf to uart 

void read_ustring(char * str);
// TODO: Add backspace support

#endif /* INCLUDE_SERIAL_H_ */
