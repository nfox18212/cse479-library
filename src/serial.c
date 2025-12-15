#include "serial.h"
#include "library.h"
// #include "library.h"
#include <stdarg.h>
#include <stdlib.h>

void write_string(const char*);

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
  /* Make PA0 and PA1 as Digital Ports  */
  (*((volatile uint32_t *)(0x4000451C))) |= 0x03;
  /* Change PA0,PA1 to Use an Alternate Function  */
  (*((volatile uint32_t *)(0x40004420))) |= 0x03;
  /* Configure PA0 and PA1 for UART  */
  (*((volatile uint32_t *)(0x4000452C))) |= 0x11;
}

void uprintf(const char* str, ...){

  va_list arg;

  va_start(arg, str);

  char *buf = calloc(MAX_STR, 1);
  // vsprintf is the internal stdio function that takes in the va_list
  int ret = vsprintf(buf, str, arg);

  // no error code received
  if(ret != -1){
    write_string(buf);
  }
  
  free(buf);
  va_end(format);
}

void write_string(const char* buf){
  char *uart = (char *) 0x4000C000;
  uint32_t *uart_fr = uptradd(uart, 0x18);
  

  char* backup_str = (char *) buf;
  char c = *backup_str;
  
  while(c != '\0'){
    if(*uart_fr != (1 <<6 )){
    // check for UART Receive buffer full.  if it is, don't move the string pointer and loop
      *uart = c;
      backup_str++;
      c = *backup_str;
    }
    

  }
}

