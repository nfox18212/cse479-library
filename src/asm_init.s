    .cdecls "init.h"

    .data

    .text
    .global init
    .global kp_handler
    .global timer_init
    .global gpio_init
	.global temp_set_ADCSSMUX3


init:
    push    {lr}
    ; bl      uart_init            ; initalize uart
    bl      serial_init
    bl      gpio_init            ; init gpio
    bl      fpu_init
    bl      adc_init

    ;bl      ssi_init
    ; temporarily disable initializing timer to manually write to SPI
    ; bl      timer_init

    pop        {lr}
    mov        pc, lr                ; return


uart_init:
    PUSH {r4-r12,lr}    ; Spill registers to stack

    ;(*((volatile uint32_t *)(0x400FE618))) = 1;
    ;Provide clock to UART0
    mov r0, #0xE618
    movt r0, #0x400F
    mov r1, #1
    str r1, [r0]

    ;Enable clock to PortA
    ;(*((volatile uint32_t *)(0x400FE608))) = 1;
    mov r0, #0xE608
    movt r0, #0x400F
    str r1, [r0]

    ;Disable UART0 Control
    ;(*((volatile uint32_t *)(0x4000C030))) = 0;
    mov r0, #0xC030
    movt r0, #0x4000
    mov r1, #0
    str r1, [r0]

    ;Set UART0_IBRD_R for 115,200 baud
    ;(*((volatile uint32_t *)(0x4000C024))) = 8;
    mov r0, #0xC024
    movt r0, #0x4000
    mov r1, #8
    str r1, [r0]

    ;Set UART0_FBRD_R for 115,200 baud
    ;(*((volatile uint32_t *)(0x4000C028))) = 44;
    add r0, r0, #0x4
    mov r1, #44
    str r1, [r0]

    ;Use System Clock
    ;(*((volatile uint32_t *)(0x4000CFC8))) = 0;
    mov r0, #0xCFC8
    movt r0, #0x4000
    mov r1, #0
    str r1, [r0]

    ;Use 8-bit word length, 1 stop bit, no parity
    ;(*((volatile uint32_t *)(0x4000C02C))) = #0x60;
    mov r0, #0xC02C
    movt r0, #0x4000
    mov r1, #0x60
    str r1, [r0]

    ;Enable UART0 Control
    ;(*((volatile uint32_t *)(0x4000C030))) = #0x301;
    add r0, r0, #0x4
    mov r1, #0x301
    str r1, [r0]

    ;Make PA0 and PA1 as Digital Ports
    ;(*((volatile uint32_t *)(0x4000451C))) |= #0x03;
    mov r0, #0x451C
    movt r0, #0x4000
    ldr r1, [r0]
    orr r1, r1, #0x03
    str r1, [r0]

    ;Change PA0,PA1 to Use an Alternate Function
    ;(*((volatile uint32_t *)(0x40004420))) |= #0x03;
    mov r0, #0x4420
    movt r0, #0x4000
    ldr r1, [r0]
    orr r1, r1, #0x03
    str r1, [r0]

    ;Configure PA0 and PA1 for UART
    ;(*((volatile uint32_t *)(0x4000452C))) |= #0x11;
    mov r0, #0x452C
    movt r0, #0x4000
    ldr r1, [r0]
    orr r1, r1, #0x11
    str r1, [r0]

    POP {r4-r12,lr}      ; Restore registers from stack
    MOV pc, lr


; most of the initialization work for the FPU is done already, this just ensures the rounding mode is correct
fpu_init:
    push    {r4-r12, lr}
    mov         r4, #0xE000
    movt        r4, #0xE000
    ; base address, offset of 0xF3C
    ldr         r5, [r4, #0xF3C]
    ; write 0x3 to bit 23:22 to enable "round to zero" or "floor" mode
    mov         r6, #0x3
    lsl         r6, #22
    orr         r5, r6, r5    ; write bits
    str         r5, [r4, #0xF3C]

    ; return
    pop        {r4-r12, lr}
    bx          lr




timer_init:
    push    {r4-r12, lr}
    ; connect clock to timer 0 and 1
    mov        r4, #0xe604
    movt       r4, #0x400f
    ldr        r5, [r4, #0]
    orr        r5, r5, #3
    str        r5, [r4, #0]

    
    mov        r4, #0x0000
    movt       r4, #0x4003
    ; timer 1
    add        r8, r4, #0x1000

    ; disable timer 0 
    ldr        r5, [r4, #0xC]
    mov        r6, #0xFFFE
    movt       r6, #0xFFFF
    and        r5, r6, r5
    str        r5, [r4, #0xC]

    ; disable timer 1
    ldr        r5, [r8, #0xC]
    and        r5, r6, r5
    str        r5, [r8, #0xC]

    ; set timer 0 to 32 bit mode
    ldr        r5, [r4, #0]
    mov        r6, #0xFFF8
    movt       r6, #0xFFFF
    and        r5, r6, r5
    str        r4, [r4, #0]

    ; set timer 1 to 32 bit mode
    ldr        r5, [r8, #0]
    mov        r6, #0xFFF8
    movt       r6, #0xFFFF
    and        r5, r6, r5
    str        r4, [r8, #0]

    ; set timer 0 to periodic mode
    ldr        r5, [r4, #0x4]
    mov        r6, #0xFFFC
    movt       r6, #0xFFFF
    and        r5, r6, r5
    orr        r5, r5, #2
    str        r5, [r4, #0x4]

    ; set timer 1 to periodic mode
    ldr        r5, [r8, #0x4]
    and        r5, r6, r5
    orr        r5, r5, #2
    str        r5, [r8, #0x4]


    ; 16E6 t/s, i want timer to have period of 0.05s.  16E6 t/s * 10000 1/s = 16E6 * 1/10000 = 1600 tick period, 0x640
    mov        r5, #0x0640
    str        r5, [r4, #0x28]

    ; make strobing timer slower - make it twice as slow
    mov        r5, #0x0C80
    str        r5, [r8, #0x28]

    ; Interrupt setup

    ; interrupt on timer 0 and timer 1
    
    ; enable timer 0 time out interrupt mask
    ldr        r5, [r4, #0x18]
    orr        r5, r5, #0x1
    str        r5, [r4, #0x18]

    ; enable timer 1 time out interrupt mask
    ldr        r5, [r8, #0x18]
    orr        r5, r5, #0x1 ; stops interrupt from triggering
    str        r5, [r8, #0x18]

    ; Configure processor to allow timer 0 and timer 1 to interrupt processor
    mov        r9, #0xE000E000
    ; movt       r9, #0xE000
    mov        r7, #0x00280000 ; bit 21 and 19
    ; movt       r7, #0x0028 
    str        r7, [r9, #0x100]


    ; re-enable timer 0
    ldr        r5, [r4, #0xC]
    orr        r5, r5, #1
    str        r5, [r4, #0xC]

    ; re-enable timer 1
    ldr        r5, [r8, #0xC]
    orr        r5, r5, #1
    str        r5, [r8, #0xC]

    pop        {r4-r12, lr}
    mov        pc, lr

gpio_init:
    PUSH {r4-r12,lr}            ; Spill registers to stack

                                ; Your code is placed here
    mov     r0, #0xE000            ; this is for initializing clock
    movt    r0, #0x400F             ; Base address
    ldrb    r1, [r0, #0x608]    ; load clock address into r1
    ; Enable port E
    orr     r1, r1, #0x10        ; Enable Port E
    strb    r1, [r0, #0x608]     

    ; Port E base address
    mov     r0, #0x4000
    movt    r0, #0x4002

    ; E1 set to INPUT which is default
    ; gpiodirection
    
    ; do NOT set digital enable

    ; GPIO Alternate Function Select 0

    ; GPIO Analogue Mode Select
    mov     r1, #4              ; Enable analogue mode for pin 2 by setting it to HIGH
    str     r1, [r0, #0x528]




    ; Port C for HC595 reg and non rgb-leds
    ; mov     r0, #0x6000
    ; movt    r0, #0x4000

    ; ldrb    r1, [r0, #0x400]     ; gpiodir
    ; orr     r1, r1, #0x80        ; pin 7 is output
    ; strb    r1, [r0, #0x400]
    ; gpio digital enable
    ; ldrb    r1, [r0, #0x51C]    ; gpioden
    ; orr     r1, r1, #0x80        ; enable pins 1,2,3 by writing 0xE
    ; strb    r1, [r0, #0x51C]
    ; Port B for SSI SCLK (pin 4), and MOSI (pin 7)
    ; mov     r0, #0x5000
    ; movt    r0, #0x4000

    ; ldrb    r1, [r0, #0x400]     ; gpiodir
    ; orr     r1, r1, #0x90       ; pin 4 and 7 set output
    ; strb    r1, [r0, #0x400]

    ; gpio digital enable
    ; ldrb    r1, [r0, #0x51C]    ; gpioden
    ; orr     r1, r1, #0x90       ; enable pin 4 pin 7
    ; strb    r1, [r0, #0x51C]

    ; port E enable for pin 0, SS of SPI
    ; mov     r0, #0x4000
    ; movt    r0, #0x4002

    ; ldrb    r1, [r0, #0x400]    ; gpio dir
    ; and     r1, r1, #0xFE       ; only need pin 0 and need it as input
    ; strb    r1, [r0, #0x400]

    ; ldrb    r1, [r0, #0x51C]    ; gpio digital enable
    ; orr     r1, r1, #1          ; enable pin 0
    ; strb    r1, [r0, #0x51C]



    POP {r4-r12,lr}              ; Restore registers from stack
    MOV pc, lr

uart_interrupt_init:

    ; Your code to initialize the UART0 interrupt goes here

    mov     r1, #0xC000        ; UARTIM base address
    movt    r1, #0x4000
    ldr     r0, [r1, #0x38]    ; UARTIM offset
    ORR     r0, r0, #0x10    ; set RXIM pin 4
    str     r0, [r1, #0x38]    ; store set pin

    mov     r1, #0xE000E000        ; e0 base address
    ldr     r0, [r1, #0x100]    ; load offset
    ORR     r0, r0, #0x20    ; set bit 5 to 1
    str     r0, [r1, #0x100]    ; store change

    MOV pc, lr



temp_set_ADCSSMUX3:

	mov		r0, #0x80A0
	movt	r0, #0x4003
	mov		r1, #1
	str		r1, [r0, #0]
	bx		lr
