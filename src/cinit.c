/*
 * cinit.c
 *
 *  Created on: Oct 1, 2025
 *      Author: nfox
 */
// Disable warning about pointers being 64 bit
// #pragma clang diagnostic ignored "-Wint-to-pointer-cast"
// #pragma clang diagnostic ignored "-Wpointer-to-int-cast"

#include "init.h"
#include "library.h"
// #include "pwm.h"

#ifndef latch_shift
#define latch_shift *((uint8_t *)0x400063FC) |= 0x80
#define unlatch_shift *((uint8_t *)0x400063FC) &= 0x7F
#endif

volatile uint32_t global_udmaptr;

extern void temp_set_ADCSSMUX3(void);
void uart_interrupt_init(void) {

  // uartim base address
  volatile uint32_t *uartim = (uint32_t *)0x4000C000;
  *(uartim + 0x38) |= 0x10; // 0x38 offset and setting pin 4

  // e0 base address
  volatile uint32_t e0 = 0xE000E000;
  //    *(e0 + 0x100) |= 0x20; // set bit 5
  *uptradd(e0, 0x100) |= 0x20; // set bit 5
}

void gpio_interrupt_init(void) {

  volatile uint32_t portD = 0x40007000;
  // Port D interrupt sense
  *((uint32_t *)(portD + 0x404)) &= 0xF0; // make PD0-PD3 interrupt during edge transition

  // make interrupts occur on only one edge
  *uptradd(portD, 0x408) &= 0xF0;

  // trigger interrupt only on posedge
  //    *(portD + 0x40C) |= 0x0F;
  *uptradd(portD, 0x40C) |= 0x0F;

  // finally allow interrupts to occur by unmasking interrupt mask reg
  //    *(portD + 0x410) |= 0x0F;
  *uptradd(portD, 0x410) |= 0x0F;

  // port D is pin 3, write to pin 3
  uint32_t e0 = 0xE000E000;
  //    *(e0 + 0x100) |= 0x08;
  *uptradd(e0, 0x100) |= 0x08;
}

void adc_init() {

  // Initialize ADC0 module2
  // connect clock to ADC MOdule 0
  *((uint32_t *)(0x400FE638)) = 1;

  // GPIO has already been enabled for doing ADC

  // Sample Sequencer Config
  volatile uint32_t ADC0 = 0x40038000; // adc 0 base addr

  //   *(volatile uint32_t *) (ADC0) = 0; // Disable ADC0 Sample Sequence 3

  // Use processor trigger, no need to set anything right now

  // Use Sample Sequence 3 as it is the simplest. 16 bit buffer.  We are reading
  // Ain1 (PE2) so enable bit 1.
  *((volatile uint8_t *)(0x400380A0)) |= 1;
  //    temp_set_ADCSSMUX3();

  // Use Continuous Sampling so write 0xF to EM3 (bits 15 to 12)
  *uptradd(ADC0, 0x014) = (0xF << 12);

  // Sample Sequence 3 Control, set sample 1 as end bit
  *uptradd(ADC0, 0x0A4) = 2;

  // Set ADC Interrupt Mask to trigger an interrupt on SS3
  //    *uptradd(ADC0, 0x008) |= 8;
  // let adc interrupt processor by writing to bit 14
  //    *uptradd(0xE000E000, 0x100) |= (1 << 14);

  // enable sequencer
  *uptradd(ADC0, 0) = 8; // Enable ADC0 Sample Sequence 3

  // initialization should be done
}

void init_udma(uint32_t *udma_ptr, uint32_t *source_ptr, uint32_t *dst_ptr) {

  // source word address
  volatile uint32_t *source_word = udma_ptr;
  *source_word = (uint32_t)source_ptr;

  // set global udma_ptr
  global_udmaptr = (uint32_t)udma_ptr;

  // destination word is the word after the source word - this will be GPIOB
  volatile uint32_t *dst_word = udma_ptr + 1;
  *dst_word = (uint32_t)dst_ptr; // gpio port B

  // the control word is the second word after the udma_ptr
  volatile uint32_t *ctl_word = udma_ptr + 2;

  // write a 3 to DSTINC at bits 30-31 to stop DST from incrementing, and since
  // we wanna transfer 16 bits write 15 << 4 because XFERSIZE is bits 13:4 and
  // we write 16-1
  *ctl_word = (uint32_t)(1 << 31) + (1 << 30) + (15 << 4);

  // unused word is +3, so we can use this to store a copy of the original
  // source word
  *(udma_ptr + 3) = (uint32_t)source_ptr;

  // Connect clock to UDMA by writing a 1 to UDMA's Run-Mode Clock Gating
  // Control Reg
  *((volatile uint32_t *)0x400FE60C) = 1;

  // Enable udma controller
  volatile uint32_t reg = 0x4000FF00;
  *((uint32_t *)reg + 0x4) = 1;

  // set address of pointer in DMACTLBASE
  *((uint32_t *)reg + 0x8) = (uint32_t)udma_ptr;
  // this is extremely scuffed typecasting shennanigans.  pointers are 32 bits,
  // writing 32-9 bits to 0x4000FF08, and its easier to work with integers

  // using GPIO B.  Meaning channel 18, peripheral 3, burst request
  // channel 18 is the 2nd channel select which are grouped by 4 bits
  *((uint32_t *)reg + 0x518) = 3 << (4 * 2);

  // Basic transfer mode, clear DMA Channel Primary Alternate Clear, 0x34
  // offset.  clear bit 18 by writing a
  *((uint32_t *)reg + 0x034) = 1 << 18;

  // not strictly neccessary but set DMA channel 18's priority to high
  *((uint32_t *)reg + 0x038) = 1 << 18;

  // Allow DMA requests by setting bit 18 at DMA Channel Request Mask Register
  // and DMA Channel Enable Set
  *((uint32_t *)reg + 0x024) = 1 << 18;
  *((uint32_t *)reg + 0x028) = 1 << 18;

  // Enable transfer mode after all of the other setup
  *ctl_word += 1;

  // generate software request
  *((uint32_t *)reg + 0x14) = (1 << 18);
}

void ssi_init() {

  // gpio alternate function select
  uint32_t gpiob = 0x40005000;
  *uptradd(gpiob, 0x420) |= 0x90; // Set pins 4 and 7 to alternate function
  *uptradd(gpiob, 0x52C) =
      0x20020000; // Write 2 to Pin 7 and Pin 4 fields and clear all else

  // latch register, write to pin 6 to latch and unlatch
  // uint32_t gpioc = 0x40006000;

  // connect clock to SSI2
  *((uint32_t *)0x400FE61C) |= 0x04;

  // Using SSI2, disable during configuration
  uint32_t ssi2 = 0x4000A000;
  *uptradd(ssi2, 0x004) |= 0; // clear, sets SSI2 as master

  // SSI clock prescale divisor to 4
  *((uint8_t *)(ssi2 + 0x010)) = 4;

  // select 16 bit data size in SSi2 config reg 0 (shift reg we're writing to is
  // 2 daisy chained shift regs)
  *((uint16_t *)(ssi2)) |= 0xF;

  // enable loopback mode (data sent is also received in SSIDR) and enable SSI2
  *((uint8_t *)(ssi2 + 0x004)) |= 0b0011;

  // unlatch PC7 by writing 0 to it
  unlatch_shift;
}

