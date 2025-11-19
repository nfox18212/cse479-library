#include "timer.h"
#include "library.h"

// Disable warning about pointers being 64 bit
// #pragma clang diagnostic ignored "-Wint-to-pointer-cast"
// #pragma clang diagnostic ignored "-Wpointer-to-int-cast"

// internal functions
// static void Periodic_init(address t_addr, bool interrupt, int width, int
// direction, timer_options topt);
static void oneShot_Periodic_init(address t_addr, bool interrupt, int width,
                                  int direction, timer_mode mode,
                                  timer_options topt);
static void RTC_init(address t_addr, bool interrupt, int width, int direction,
                     timer_mode mode, timer_options topt);
static void edge_count_init(address t_addr, bool interrupt, int width,
                            int direction, timer_mode mode, timer_options topt);
static void edge_time_init(address t_addr, bool interrupt, int width,
                           int direction, timer_mode mode, timer_options topt);
static void pwm_timer_init(address t_addr, bool interrupt, int width,
                           int direction, timer_mode mode, timer_options topt);

// initializes ONE timer, if multiple timers should be configured this function
// should be called multiple times. 
// TODO: Add Variadic Macro Usage
int _timer_init(timer_number timer_num, uint32_t load, int width, int direction,
                bool interrupt, timer_mode mode, timer_options topt) {

  address timer_base = 0x0;                // default value
  address *clock = (address *)0x400FE604;  // 16/32 bit clock
  address *wclock = (address *)0x400FE654; // 32/64 bit clock
  // Used to determine if we are enabling the interrupt on the b half or not
  bool b_half = topt.timer_half == 'b';
  int interrupt_number = 0;
  
  // Unsure if this long-ass switch statement is a good way to implement this.
  // sets the base address and connects the clock depending on the inputted
  // timer number.  Also if the interrupt bool is true, enables
  switch (timer_num) {
  case timer_0:
    timer_base = 0x40030000;
    *clock |= (1 << 0); // keeps things consistent, will get optimized out
    interrupt_number = 19;
    break;
  case timer_1:
    timer_base = 0x40031000;
    *clock |= (1 << 1);
    interrupt_number = 21;
    break;
  case timer_2:
    timer_base = 0x40032000;
    *clock |= (1 << 2);
    interrupt_number = 23;
    break;
  case timer_3:
    timer_base = 0x40033000;
    *clock |= (1 << 3);
    interrupt_number = 35;
    break;
  case timer_4:
    timer_base = 0x40034000;
    *clock |= (1 << 4);
    interrupt_number = 70;
    break;
  case timer_5:
    timer_base = 0x40003500;
    *clock |= (1 << 5);
    interrupt_number = 92;
    break;
  // wide timers
  case wtimer_0:
    timer_base = 0x40036000;
    *wclock |= (1 << 0);
    interrupt_number = 94;
    break;
  case wtimer_1:
    timer_base = 0x40037000;
    *wclock |= (1 << 1);
    interrupt_number = 96;
    break;
  case wtimer_2:
    timer_base = 0x4004C000;
    *wclock |= (1 << 2);
    interrupt_number = 98;
    break;
  case wtimer_3:
    timer_base = 0x4004D000;
    *wclock |= (1 << 3);
    interrupt_number = 100;
    break;
  case wtimer_4:
    timer_base = 0x4004E000;
    *wclock |= (1 << 4);
    interrupt_number = 102;
    break;
  case wtimer_5:
    timer_base = 0x4004F000;
    *wclock |= (1 << 5);
    interrupt_number = 104;
    break;
  default:
    // invalid timer number, report and exit
    output_string("Invalid timer number passed in, stopping initialization");
    return -1;
    break;
  }

  if(interrupt){
    // mild abuse of booleans here.  in stdbool, TRUE is 1 and FALSE is 0, so adding a bool will increase the 
    // interrupt number by 1 if the user is accessing the b half
    enable_interrupt(interrupt_number + b_half);
  }

  switch (mode) {
  case one_shot:
  case periodic:
    oneShot_Periodic_init(timer_base, interrupt, width, direction, mode, topt);
    break;

  case RTC:
    RTC_init(timer_base, interrupt, width, direction, mode, topt);
    break;

  case edge_count:
    edge_count_init(timer_base, interrupt, width, direction, mode, topt);
    break;

  case edge_time:
    edge_time_init(timer_base, interrupt, width, direction, mode, topt);
    break;

  case PWM:
    pwm_timer_init(timer_base, interrupt, width, direction, mode, topt);
    break;
  }

  return 0;
}

static void oneShot_Periodic_init(address t_addr, bool interrupt, int width,
                                  int direction, timer_mode mode,
                                  timer_options topt) {

  // Start by disabling timer
  *uptradd(t_addr, 0x00C) = 0;
  // clear config
  *uptradd(t_addr, 0x000) = 0;

  // initializing to a because using topt is optional, aside from the "enable" field.
  char half = 'a';

  // addresses, assuming timer A by default.
  address *ModeRegister = uptradd(t_addr, 0x004);
  address *InterruptMaskReg = uptradd(t_addr, 0x018);

  if (topt.enable == true) {

    if ((width == 16 || width == 32) &&
        (topt.timer_half == 'a' || topt.timer_half == 'b')) {
      // Set timer to 16/32 bit mode
      *uptradd(t_addr, 0x000) = 1;
      half = topt.timer_half;
    }

    if (half == 'b') {
      ModeRegister = uptradd(t_addr, 0x008);
    }

    // this will also handle the other options like if timers are chained
  }

  // time-out mask by default
  if (interrupt && half == 'a') {
    *InterruptMaskReg = (1 << 0);
  }

  if (interrupt && half == 'b') {
    *InterruptMaskReg = (1 << 8);
  }

  // Configure the TnMR field in the Timer Mode Register
  if(mode == one_shot){
    *ModeRegister |= 1;
  }

  if(mode == periodic){
    *ModeRegister |= 2;
  }
}

static void RTC_init(address t_addr, bool interrupt, int width, int direction,
                     timer_mode mode, timer_options topt) {}

static void edge_count_init(address t_addr, bool interrupt, int width,
                            int direction, timer_mode mode,
                            timer_options topt) {}

static void edge_time_init(address t_addr, bool interrupt, int width,
                           int direction, timer_mode mode, timer_options topt) {
}

static void pwm_timer_init(address t_addr, bool interrupt, int width,
                           int direction, timer_mode mode, timer_options topt) {
}
