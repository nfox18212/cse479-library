/*
 * library.c
 *
 *  Created on: Sep 22, 2025
 *      Author: njfox4
 */

#include "library.h"
// #include "debug.h"


uint32_t int2string(int i, char* str){
    // this will write the int to the string and return the number of characters written



    // largest power of 10 in 32 bit integer is 10^10.
    uint32_t chars_written = 0;
    uint32_t order = 1000000000;
    bool innerZero = false; // this will flip whenever we discover a non-zero integer
    // backup pointer to do pointer math on
    char* backup_str = str;

    while(order > 0){
        // quick hack: dividing a number that would leave a decimal and storing in an integer truncates the operation
        int ret = i / order;
        char converted = '\0';

        if(ret != 0 && innerZero == false){
          innerZero = true;
        }

        if(ret != 0 || innerZero == true){
          // turn returned integer into a character by adding
          converted = ret + 0x30;
          // store the string
          *backup_str = converted;
          // increment pointer
          backup_str += 1;
          // increment chars_written
          chars_written++;
          // decrement int
          i -= ret * order;


      }

        order = order / 10;
    }

    *backup_str = '\0'; // ensure a null is written to terminate string
    return chars_written;
}

uint32_t bitfield(uint32_t *bits, uint32_t num_bits){
    // Takes an array of bit positions to turn into an unsigned integer bitfield
    uint32_t field = 0;

    for(int i = num_bits-1; i >= 0; i-- ){
      uint32_t offset = *(bits + i);
      field += (1 << offset);
    }


    return field;
}


void write_LEDs(uint32_t num){
  // Light up the designated LEDs in GPIO Port B.  Requires GPIO Port B to be enabled
  uint32_t *LEDptr = (uint32_t *) 0x400053FC;
  // Clean data to only let the last 4 bits through
  num &= 0xF;
  *LEDptr = num;
}

float fexp(float base, uint32_t power){

    float acc = base; // accumulator

    if(power == 0){
        return 1.0;
    }

    for(int i=1; i < power; i++){
        acc = base * acc;
    }

    return acc;
}



