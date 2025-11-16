/*
 * cinit.c
 *
 *  Created on: Oct 1, 2025
 *      Author: nfox
 */
// Disable warning about pointers being 64 bit
#include <stdint.h>
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"



#include "init.h"
#ifdef clion
#include "cse479-library.lnk/library.h"
#else
#include "library.h"
#endif
#include "main.h"
// #include <string.h>

#ifndef latch_shift
#define latch_shift *((uint8_t *) 0x400063FC) |= 0x80
#define unlatch_shift *((uint8_t *) 0x400063FC) &= 0x7F
#endif

#ifdef PWM
#include "pwm.h"
#endif

#ifndef uptradd// properly offset a uint32_t pointer
#define uptradd(ptr, offset) ((uint32_t *)(ptr + offset))
// get length of array
#define len(array) (sizeof(array) / sizeof(*array)) // type agnostic
#endif

volatile uint32_t global_udmaptr;

extern void temp_set_ADCSSMUX3(void);

// thank you dr. schindler for providing this code for initializing uart
void serial_init(void) {
  /* Provide clock to UART0  */
  (*((volatile uint32_t *)(0x400FE618))) = 1;
  /* Enable clock to PortA  */
  (*((volatile uint32_t *)(0x400FE608))) = 1;
  /* Disable UART0 Control  */
  (*((volatile uint32_t *)(0x4000C030))) = 0;
  /* Set UART0_IBRD_R for 115,200 baud */
  (*((volatile uint32_t *)(0x4000C024))) = 8;
  /* Set UART0_FBRD_R for 115,200 baud */
  (*((volatile uint32_t *)(0x4000C028))) = 44;
  /* Use System Clock */
  (*((volatile uint32_t *)(0x4000CFC8))) = 0;
  /* Use 8-bit word length, 1 stop bit, no parity */
  (*((volatile uint32_t *)(0x4000C02C))) = 0x60;
  /* Enable UART0 Control  */
  (*((volatile uint32_t *)(0x4000C030))) = 0x301;
  /*************************************************/
  /* The OR operation sets the bits that are OR'ed */
  /* with a 1.  To translate the following lines   */
  /* to assembly, load the data, OR the data with  */
  /* the mask and store the result back.           */
  /*************************************************/
  /* Make PA0 and PA1 as Digital Ports  */
  (*((volatile uint32_t *)(0x4000451C))) |= 0x03;
  /* Change PA0,PA1 to Use an Alternate Function  */
  (*((volatile uint32_t *)(0x40004420))) |= 0x03;
  /* Configure PA0 and PA1 for UART  */
  (*((volatile uint32_t *)(0x4000452C))) |= 0x11;
}

void uart_interrupt_init(void){

    // uartim base address
    volatile uint32_t*  uartim = (uint32_t *) 0x4000C000;
    *(uartim + 0x38) |= 0x10; // 0x38 offset and setting pin 4

    // e0 base address
    volatile uint32_t e0 = 0xE000E000;
//    *(e0 + 0x100) |= 0x20; // set bit 5
    *uptradd(e0, 0x100) |= 0x20; // set bit 5
}

#ifdef new_gpio_init
void gpio_init(gpio_port port, int pins[8]){

    // TODO: finish implementation of this.  Use assembly for now.

    uint32_t *clockp = (uint32_t *) 0x400FE608;
    uint32_t enable;

    // support up to 8 pins and only APB
    uint32_t base_addr;

    switch(port){
        case port_a:
            base_addr = 0x40004000;
            break;
        case port_b:
            base_addr = 0x40005000;
            break;
        case port_c:
            base_addr = 0x40006000;
            break;
        case port_d:
            base_addr = 0x40007000;
            break;
        case port_e:
            base_addr = 0x40024000;
            break;
        case port_f:
            base_addr = 0x40025000;
    }
}
#endif



void gpio_interrupt_init(void){

    volatile uint32_t portD =  0x40007000;
    // Port D interrupt sense
    *((uint32_t *) (portD + 0x404)) &= 0xF0; // make PD0-PD3 interrupt during edge transition

    // make interrupts occur on only one edge
    *uptradd(portD, 0x408) &= 0xF0;


    // trigger interrupt only on posedge
//    *(portD + 0x40C) |= 0x0F;
    *uptradd(portD, 0x40C) |= 0x0F;

    // finally allow interrupts to occur by unmasking interrupt mask reg
//    *(portD + 0x410) |= 0x0F;
    *uptradd(portD, 0x410) |= 0x0F;

    // port D is pin 3, write to pin 3
    uint32_t e0 =  0xE000E000;
//    *(e0 + 0x100) |= 0x08;
    *uptradd(e0, 0x100) |= 0x08;
}
#ifdef pwm
void pwm_init(){

    // helpful variable
    volatile uint32_t mask = 0;

    /* 
    The Red   LED is M1PWM5
    The Blue  LED is M1PWM6
    the Green LED is M1PWM7

    Will eventually have to initialize for all of them
    */

    // Run Mode Clock Gating Control Register
    // volatile uint32_t RCGC =  0x400FE000;

    // GPIO port F
    volatile uint32_t GPIOF =  0x40025000;


    // Enable PWM clock
    *((uint32_t *) (RCGC + 0x100)) |= 0x00100000; 

    // Enable clock to GPIO Port F via RCGC_GPIO, Port F is BF 5 so 1 << 5 = 0x20
    *((uint32_t *)  (RCGC + 0x608)) |= 0x20;
    // typecasting is necessary to force compiler to use exact offset


    // Alternate Function Select (GPIOAFSEL) for port F
    // Red LED is Pin 1, Blue is Pin 2, Green is Pin 3
    // 0b1110 = 0xE, says to processor to have alternate function use these pins instead of GPIO
    *((uint32_t *) (GPIOF + 0x420)) |= 0xE;

        // Configure GPIOPCTL
    // using pins 1 to 3 again, according to table 23-5 lists M1PWM5, 6, 7 for P1 to P3.  Write 5 to each pin.
    // Pin 1 is BF 4-7, Pin 2 is BF 8-11, Pin 3 is BF 12-15
    // Since we need to write 5 to each, we need to do:
    // write a 1 to bit 4, 6; 8, 10; 12, 14 which turns out to be 0x5550
    mask = 0x5550;
    *((uint32_t *)(GPIOF + 0x52C)) |= mask;

    // Connect clock to pwm module 1, so write to bit 1 aka 0x2
    *uptradd(RCGC, 0x640) |= 0x2; 

    // configure Run Mode Clock Configuration register (RCC)
    // configure RCC to use PWM divide
    // volatile uint32_t RCC = 0x400FE000;

    // try a different divisor, do 64 so write a 1 to bits 19, 18, 17. Also enable USEPWMDIV by writing 1 to bit 20
    // sets PWM DIV
    uint32_t bf4[] = {19, 18, 17, 20};
    mask = bitfield(bf4, len(bf4));
    *((uint32_t *) (RCC + 0x060)) |=  mask;

    // Now configure the PWM generator to use Count down Mode with immediate parameter updates
    // Using Module 1 PWM2
    volatile uint32_t M1PWM =  0x40029000;
    // Documentation says M1PWM is 0x4002900+0x0C0, and to write 0x0000.0000 to PWM2CTL.  Documentation also says bits 31:19 are reserved and Read-Only.  Contradiction?  
    // Not a contradiction because the default value of the rest of the bits is 0
    // 0xfff80000 - new value

    *((uint32_t *) (M1PWM + 0x0C0)) &= 0xFFF80000; // PWM2CTL

    /* 
    Set the action for when the counter value reaches zero, so
    set bitfield 11:10 to 0x3 to drive it HIGH, so enable both bits
    Also set action for when counter value reaches the compare value by writing to bitfield 1:0, I'll have it invert so write 0x1 so write to 1
    */
    uint32_t bf3[] = { 10, 1};
    mask =  bitfield(bf3, len(bf3));
    *uptradd(M1PWM, 0xE4) |= mask;


    // // set the period for A and B
    // uint32_t load = change_pwm_period(PWM12A, 4); // minimum frequency is approx 4 Hz
    // // only need to do it for A to set period for both A and B

    //    // WRITING LOAD VALUE WAY TOO SMALL

    // // Set Duty Cycle for PWM2A
    // uint32_t cmpa = change_pwm_duty(PWM12A, 0.25);

    // // Set Duty Cycle for PWM2B
    // uint32_t cmpb = change_pwm_duty(PWM12B, 0.75);


    // Start timers in PWM2 Generator with PWM0CTL
    *((uint32_t *) (PWM12B + 0x40)) |= 1;

    // PWM Output Enable
    // Need to write a 1 to Pin 7, 6 and 5
    uint32_t bits[] = {5, 6, 7};
    mask = bitfield(bits, len(bits));
    *((uint32_t *) (M1PWM + 0x008)) |= mask;

    // PWMENABLE also needs to be turned on

    // try other enable? at PWM2CTL and add in debug
    *uptradd(M1PWM, 0x0C0) |= 0x5;

}
#endif


void adc_init(){


    // Initialize ADC0 module2
    // connect clock to ADC MOdule 0
    *((uint32_t *)(0x400FE638)) = 1;

    // GPIO has already been enabled for doing ADC

   // Sample Sequencer Config
   volatile uint32_t ADC0 = 0x40038000; // adc 0 base addr

//   *(volatile uint32_t *) (ADC0) = 0; // Disable ADC0 Sample Sequence 3

   // Use processor trigger, no need to set anything right now

   // Use Sample Sequence 3 as it is the simplest. 16 bit buffer.  We are reading Ain1 (PE2) so enable bit 1.
  *((volatile uint8_t *) (0x400380A0)) |= 1;
//    temp_set_ADCSSMUX3();

    // Use Continuous Sampling so write 0xF to EM3 (bits 15 to 12)
    *uptradd(ADC0, 0x014) =  (0xF << 12);

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



void init_udma(uint32_t* udma_ptr, uint32_t *source_ptr, uint32_t* dst_ptr){

    // source word address
    volatile uint32_t* source_word = udma_ptr;
    *source_word = (uint32_t) source_ptr;

    // set global udma_ptr
    global_udmaptr = (uint32_t) udma_ptr;

    // destination word is the word after the source word - this will be GPIOB
    volatile uint32_t* dst_word = udma_ptr + 1; 
    *dst_word = (uint32_t) dst_ptr; // gpio port B

    // the control word is the second word after the udma_ptr
    volatile uint32_t* ctl_word = udma_ptr + 2;

    // write a 3 to DSTINC at bits 30-31 to stop DST from incrementing, and since we wanna transfer 16 bits write 15 << 4
    // because XFERSIZE is bits 13:4 and we write 16-1
    *ctl_word = (uint32_t) (1 << 31) + (1 << 30) + (15 << 4);

    // unused word is +3, so we can use this to store a copy of the original source word
    *(udma_ptr+3) = (uint32_t) source_ptr;

    // Connect clock to UDMA by writing a 1 to UDMA's Run-Mode Clock Gating Control Reg 
    *((volatile uint32_t *) 0x400FE60C) = 1;

    // Enable udma controller
    volatile uint32_t reg = 0x4000FF00;
    *((uint32_t *) reg + 0x4) = 1;

    // set address of pointer in DMACTLBASE
    *((uint32_t *) reg + 0x8) = (uint32_t) udma_ptr;
    // this is extremely scuffed typecasting shennanigans.  pointers are 32 bits, writing 32-9 bits to 0x4000FF08, and its easier to work with integers

    // using GPIO B.  Meaning channel 18, peripheral 3, burst request
    // channel 18 is the 2nd channel select which are grouped by 4 bits
    *((uint32_t *) reg + 0x518) =  3 << (4*2);

    // Basic transfer mode, clear DMA Channel Primary Alternate Clear, 0x34 offset.  clear bit 18 by writing a
    *((uint32_t *) reg + 0x034) = 1 << 18;

    // not strictly neccessary but set DMA channel 18's priority to high
    *((uint32_t *) reg + 0x038) = 1 << 18;

    // Allow DMA requests by setting bit 18 at DMA Channel Request Mask Register and DMA Channel Enable Set
    *((uint32_t *) reg + 0x024) = 1 << 18;
    *((uint32_t *) reg + 0x028) = 1 << 18;

    // Enable transfer mode after all of the other setup
    *ctl_word += 1;

    // generate software request
    *((uint32_t *) reg + 0x14) = (1 << 18);

}


void ssi_init(){

    // gpio alternate function select
    uint32_t gpiob =  0x40005000;
    *uptradd(gpiob, 0x420) |= 0x90;    // Set pins 4 and 7 to alternate function
    *uptradd(gpiob, 0x52C) =  0x20020000;    // Write 2 to Pin 7 and Pin 4 fields and clear all else

    // latch register, write to pin 6 to latch and unlatch
    //uint32_t gpioc = 0x40006000;
    
    // connect clock to SSI2
    *((uint32_t *) 0x400FE61C) |= 0x04;

    // Using SSI2, disable during configuration
    uint32_t ssi2 = 0x4000A000;
    *uptradd(ssi2, 0x004) |= 0;     // clear, sets SSI2 as master

    // SSI clock prescale divisor to 4
    *((uint8_t *) (ssi2 + 0x010)) = 4;

    // select 16 bit data size in SSi2 config reg 0 (shift reg we're writing to is 2 daisy chained shift regs)
    *((uint16_t *) (ssi2)) |= 0xF;

    // enable loopback mode (data sent is also received in SSIDR) and enable SSI2
    *((uint8_t *) (ssi2 + 0x004)) |= 0b0011;

    // unlatch PC7 by writing 0 to it
    unlatch_shift;


}

// initializes ONE timer, if multiple timers should be configured this function should be called multiple times.
void timer_cinit(int timer_number, uint16_t load){


    return;
}