#include "pwm.h"
#include "library.h"

#ifndef uptradd// properly offset a uint32_t pointer
#define uptradd(ptr, offset) ((uint32_t *)(ptr + offset))
// get length of array
#define len(array) (sizeof(array) / sizeof(*array)) // type agnostic
#endif


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
    volatile uint32_t RCGC =  0x400FE000;

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
    volatile uint32_t RCC = 0x400FE000;

    // try a different divisor, do 64 so write a 1 to bits 19, 18, 17. Also enable USEPWMDIV by writing 1 to bit 20
    // sets PWM DIV
    mask = make_bitfield(19, 18, 17, 20, -1);
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
    set make_bitfield 11:10 to 0x3 to drive it HIGH, so enable both bits
    Also set action for when counter value reaches the compare value by writing to bitfield 1:0, I'll have it invert so write 0x1 so write to 1
    */
    mask = make_bitfield(10, 11, -1);
    *uptradd(M1PWM, 0xE4) |= mask;


    // set the period for A and B
    // uint32_t load = change_pwm_period(PWM12A, 4); // minimum frequency is approx 4 Hz
    // only need to do it for A to set period for both A and B

    // WRITING LOAD VALUE WAY TOO SMALL

    // Set Duty Cycle for PWM2A
    // uint32_t cmpa = change_pwm_duty(PWM12A, 0.25);

    // Set Duty Cycle for PWM2B
    // uint32_t cmpb = change_pwm_duty(PWM12B, 0.75);


    // Start timers in PWM2 Generator with PWM0CTL
    *((uint32_t *) (PWM12B + 0x40)) |= 1;

    // PWM Output Enable
    // Need to write a 1 to Pin 7, 6 and 5
//    uint32_t bits[] = {5, 6, 7};
    mask = make_bitfield(5, 6, 7, -1);
    *((uint32_t *) (M1PWM + 0x008)) |= mask;

    // PWMENABLE also needs to be turned on

    // try other enable? at PWM2CTL and add in debug
    *uptradd(M1PWM, 0x0C0) |= 0x5;

}
