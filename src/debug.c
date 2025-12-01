#include "debug.h"

// global variable for debug function
static bool cangoback = false;

void handlefault(void) {

  uint32_t baddr = getbaddr();  // technically this should be a pointer, but it doesn't matter.  its not going to be used anywhere except in assembly, so type doesn't really matter

  uint8_t didcrash = getdidcrash();

  if (didcrash) {
    customfault(baddr);
  } else {
    normalfault();
  }
}

void customfault(uint32_t baddr) {

  // we encountered a custom fault, logic for handling it is here
  char *msg = "A custom crash was encountered, attempting to jump back to address before link.\r\n";

  output_string(msg);
}

void normalfault(void) {

//  void *badaddrp = calloc(1,4); // grab some memory
  uint32_t badaddr = 0; // where the bad address will be stored
  volatile void *cfsr = (volatile void *)0xE000ED28;  // address of the configurable fault status register, can get further information from bitmasking this

  char* msg = "FAULT OCCURRED!!!  HANDLING FAULT!!!\r\n";
  output_string(msg);

  int testv = 4;
  testv = test(testv);

  uint32_t mmsr = (*(uint32_t *)cfsr) & (uint32_t)0xFF;        // memfault status register is only 1 byte in size
  uint32_t bfsr = (*(uint32_t *)cfsr) & (uint32_t)0xFF00;      // bus fault status reg also one byte in size, at 0xE000ED29, shift bitmask
  uint32_t ufsr = (*(uint32_t *)cfsr) & (uint32_t)0xFFFF0000;  // usage fault status reg is upper halfword, mask for just that

  if (mmsr != 0) {
    uint32_t baddr = handle_memfault(mmsr);
//    *(uint32_t *) badaddrp = baddr; // write to memory
    badaddr = baddr;
  }

  if (bfsr != 0) {
    uint32_t baddr = handle_busfault(bfsr);
//    *(uint32_t *) badaddrp = baddr;
    badaddr = baddr;
  }

  if (ufsr != 0) {
    // no address register associated with a usage fault
    ufsrprint(ufsr);
  }


  // make sure baddr is actually valid
  if(badaddr > 0x2F000000){
	  char* baddrmsg = "Determined the given address is invalid\r\n";
	  output_string(baddrmsg);
	  cangoback = false;
  }

  if (cangoback) {
	  ngoback(badaddr);  // goes back to assembly passing in the bad address
  }


  for(;;);
//  struct frame faultFrame;
//  // unstack frame
//  asm volatile (
//		  "\tpop\t{r0-r3,r12}\n" // grab r0-r3 and r12
//		  "\tmov\t%0, r0\n"	// move into c
//		  "\tmov\t%1, r1\n"
//		  "\tmov\t%2, r2\n"
//		  "\tmov\t%3, r3\n"
//		  "\tmov\t%4, r12\n"			// grab lr, pc and xspr - store in temp registers
//		  "\tpop\t{r0-r2}\n"
//		  "\tmov\t%5, r0\n"				// lr
//		  "\tmov\t%6, r1\n"				// pc
//		  "\tmov\t%7, r2\n" : "=r" (faultFrame.r0), "=r" (faultFrame.r1), "=r" (faultFrame.r2), "=r" (faultFrame.r3), "=r" (faultFrame.r12), "=r" (faultFrame.lr), "=r", (faultFrame.pc), "=r" (faultFrame.xpsr));
//		  : "r0", "r1", "r2", "r3", "memory");

}



uint32_t handle_memfault(uint32_t mmsr) {
  volatile void *mmar = (volatile void *)0xE000ED34;  // address of the MemManage Address Register
  // - contains address of faulted instruction
  mmsrprint(mmsr);

  uint32_t badaddr = *(uint32_t *)mmar;  // dereference mmar to get the address of the faulted instruction
  return badaddr;
}

void mmsrprint(uint32_t mmsr) {
  // now we print to uart what the type of error is
  uint32_t MMARVALID = mmsr & 0x80;
  uint32_t MLSPERR = mmsr & 0x20;
  uint32_t MSTKERR = mmsr & 0x10;
  uint32_t MUNSTKERR = mmsr & 0x08;
  uint32_t DACCVIOL = mmsr & 0x02;
  uint32_t IACCVIOL = mmsr & 0x01;

  char *msg = "N/A\r\n";
  // there's no == because it'll do this if MMARVALID is nonzero, and MMARVALID will only ever be zero or nonzero.
  if (MMARVALID) {
    msg = "MMAR Contains valid fault address\r\n";
    cangoback = true;
  } else {
    msg = "value in MMAR is not a valid fault addres\r\n";
  }

  output_string(msg);  // goes to assembly

  if (MLSPERR) {
    msg = "a MemManage fault occurred during floating-point lazy state preservation.\r\n";
  } else {
    msg = "no MemManage fault occurred during floating-point lazy state preservation\r\n";
  }

  output_string(msg);

  if (MSTKERR) {
    msg = "stacking for an exception entry has caused one or more access violations. When this bit is 1, the SP is still adjusted but the values in the context area on the stack might be incorrect. The processor has not written a fault address to the MMAR.\r\n";
  } else {
    msg = "no stacking fault.\r\n";
  }

  output_string(msg);

  if (MUNSTKERR) {
    msg = "unstack for an exception return has caused one or more access violations. This fault is chained to the handler. This means that when this bit is 1, the original return stack is still present. The processor has not adjusted the SP from the failing return, and has not performed a new save. The processor has not written a fault address to the MMAR.\r\n";
  } else {
    msg = "no unstacking fault.\r\n";
  }

  output_string(msg);

  if (DACCVIOL) {
    msg = "The processor attempted a load or store at a location that does not permit the operation. When this bit is 1, the PC value stacked for the exception return points to the faulting instruction. The processor has loaded the MMAR with the address of the attempted access.\r\n";
  } else {
    msg = "no data access violation fault.\r\n";
  }

  output_string(msg);

  if (IACCVIOL) {
    msg = "The processor attempted an instruction fetch from a location that does not permit execution\r\n";
  } else {
    msg = "no instruction access violation fault.\r\n";
  }

  output_string(msg);
}

uint32_t handle_busfault(uint32_t bfsr) {

  volatile void *bfar = (volatile void *)0xE000ED38;
  uint32_t badaddr = *(uint32_t *)bfar;

  bfsrprint(bfsr);
  return badaddr;
}

void bfsrprint(uint32_t obfsr) {
  // types could *probably* be turned into uint8_t or a bool, but meh i don't really care and don't want to deal with type changing
  uint32_t bfsr = obfsr >> 8;

  uint32_t BFARVALID = bfsr & 0x80;
  uint32_t LSPERR = bfsr & 0x20;
  uint32_t STKERR = bfsr & 0x10;
  uint32_t UNSTKERR = bfsr & 0x08;
  uint32_t IMPRECISERR = bfsr & 0x04;
  uint32_t PRECISERR = bfsr & 0x02;
  uint32_t IBUSERR = bfsr & 0x01;

  // chain of ifs to print the message
  char *msg = "N/A\r\n";
  if (BFARVALID) {
    msg = "BFAR holds a valid fault address\r\n";
    cangoback = true;
  } else {
    msg = "value in BFAR is not a valid fault address\r\n";
  }

  output_string(msg);

  if (LSPERR) {
    msg = "a bus fault occurred during floating-point lazy state preservation\r\n";
  } else {
    msg = "no bus fault occurred during floating-point lazy state preservation\r\n";
  }

  output_string(msg);

  if (STKERR) {
    msg = "no stacking fault\r\n";
  } else {
    msg = "stacking for an exception entry has caused one or more BusFaults.\r\n";
  }

  output_string(msg);

  if (UNSTKERR) {
    msg = "unstack for an exception return has caused one or more BusFaults.\r\n";
  } else {
    msg = "no unstacking fault.\r\n";
  }

  output_string(msg);

  if (IMPRECISERR) {
    msg = "a data bus error has occurred, but the return address in the stack frame is not related to the instruction that caused the error.\r\n";
  } else {
    msg = "no imprecise data bus error.\r\n";
  }

  output_string(msg);

  if (PRECISERR) {
    msg = "a data bus error has occurred, and the PC value stacked for the exception return points to the instruction that caused the fault.\r\n";
  } else {
    msg = "no precise data bus error\r\n";
  }

  output_string(msg);

  if (IBUSERR) {
    msg = "an instruction bus error has occurred.\r\n";
  } else {
    msg = "no instruction bus error has occurred.\r\n";
  }

  output_string(msg);
}

void ufsrprint(uint32_t oufsr) {

  uint32_t ufsr = oufsr >> 16;
  uint16_t DIVBYZERO = ufsr & 0x200;
  uint16_t UNALIGNED = ufsr & 0x100;
  uint16_t NOCP = ufsr & 0x008;
  uint16_t INVPC = ufsr & 0x004;
  uint16_t INVSTATE = ufsr & 0x002;
  uint16_t UNDEFINSTR = ufsr & 0x001;

  char *msg = "N/A\r\n";
  if (DIVBYZERO) {
    msg = "the processor has executed an SDIV or UDIV instruction with a divisor of 0\r\n";
  } else {
    msg = "no divide by zero fault, or divide by zero trapping not enabled\r\n";
  }

  output_string(msg);

  if (UNALIGNED) {
    msg = "The processor has made an unaligned memory access\r\n";
  } else {
    msg = " no unaligned access fault, or unaligned access trapping not enabled";
  }

  output_string(msg);

  if (NOCP) {
    msg = "The processor has attempted to access a coprocessor\r\n";
  } else {
    msg = "no UsageFault caused by attempting to access a coprocessor\r\n";
  }

  output_string(msg);

  if (INVPC) {
    msg = "the processor has attempted an illegal load of EXC_RETURN to the PC, as a result of an invalid context, or an invalid EXC_RETURN value\r\nNOTE: When this bit is set to 1, the PC value stacked for the exception return points to the instruction that tried to perform the illegal load of the PC.\r\n";
  } else {
    msg = "no invalid load into PC UsageFault\r\n";
  }

  output_string(msg);

  if (INVSTATE) {
    msg = "the processor has attempted to execute an instruction that makes illegal use of the EPSR\r\n";
  } else {
    msg = "no invalid state UsageFault\r\n";
  }

  output_string(msg);

  if (UNDEFINSTR) {
    msg = "The processor has attempted to execute an undefined instruction.  When this bit is set to 1, the PC value stacked for the exception return points to the undefined instruction. An undefined instruction is an instruction that the processor cannot decode.\r\r\n";
  } else {
    msg = "no undefined instruction UsageFault.\r\n";
  }

  output_string(msg);
}

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

char *addrtostring(uint32_t addr) {
  // converts a address to a hexadecimal string to print it out - bounded with a max of 8 characters for a 4 byte address
  char *string = "";
  // first two characters are 0x
  uint8_t filter = 0xF;
  string[0] = '0';
  string[1] = 'x';

  int i = 0;
  for (i = 2; i < 9; i++) {
    uint32_t num = addr & filter;  // pull targeted nibble out
    num = num >> i;                // force it to be smallest nibble, so 0xA0 would turn into 0x0A
    char numc;                     // num but character verison
    if (num < 10) {
      numc = 0x30 + num;  // the ascii versions of 0-9 is 0x30 + 0-9
    } else if (10 <= num && num <= 15) {
      // here we need to use A-F, which will be 0x37 + num.  0xA + 0x37 = 0x41 for example
      numc = 0x37 + num;
    }
    string[i] = numc;
  }

  return string;
}
