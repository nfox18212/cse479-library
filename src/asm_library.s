; this file is provided entirely for reference of the old assembly implementation
.data

    .global numPrompt
    .global demPrompt
    .global dividend
    .global divisor
    .global quotient
    .global remainder
    .global fault_pc

numPrompt:      .cstring "Give me a divisor:  "
demPrompt:      .cstring "Give me a dividend: "
dividend:       .cstring "        " ; also numerator
divisor:        .cstring "        " ; also denominator
quotient:       .cstring "        "
remainder:      .cstring "        "
quoreturn:      .cstring "Quotient =  "
sixtyspaces:    .cstring "                                                            "
remreturn:      .cstring "Remainder =  "
eightyspaces:   .cstring "                                                                                "
space1:         .space 8
space2:         .space 8


lab4prompt      .cstring "Enter a number 1-4: "
introdummy      .cstring "I said 1-4, dummy. Press enter and try again. "
restartprompt   .cstring "Would you like to restart? y or n: "

fault_pc:       .word   0x0


; okay so these space labels are admittedly REALLY hacky.  This is just so that it'll allow me to store string length metadata because
; i don't feel like rewriting the entire algorithm of string2int to not need to use string length
; good practice? nope, but good practice can suck my balls i need this lab done

negFlag:  .byte 0

    .text

ptr_to_nprompt:         .word numPrompt
ptr_to_dprompt:         .word demPrompt
ptr_to_numerator:       .word dividend
ptr_to_denominator:     .word divisor
ptr_to_quotient:        .word quotient
ptr_to_remainder:       .word remainder
ptr_to_remreturn:       .word remreturn
ptr_to_quoreturn:       .word quoreturn
ptr_to_negFlag:         .word negFlag
ptr_to_space1:          .word space1
ptr_to_space2:          .word space2
ptr_to_eightyspaces:    .word eightyspaces
ptr_to_sixtyspaces:     .word sixtyspaces ; for use with a 80 character wide terminal
ptr_to_lab4prompt:      .word lab4prompt
ptr_to_introdummy:      .word introdummy
ptr_to_restart:         .word restartprompt

globals:    .word 0x0


    .global output_character
    .global read_character
    .global read_string
    .global output_string
    .global read_from_push_btns
    .global illuminate_LEDs
    .global illuminate_RGB_LED
    .global read_tiva_push_button
    .global div_and_mod
    .global uart_interrupt_init
    .global gpio_interrupt_init
    .global goback
    .global crash
    .global asm_int2string
    .global long2string

newdm:      .set    1

read_string:
    push {r4-r12,lr}        ; store any registers in the range of r4 through r12
                            ; that are used in your routine.  include lr if this
                            ; routine calls another routine.
    and     r6, r6, #0      ; clear r6, we'll be using this to count each character in the passed in string and use it as metadata
    mov     r4, r0          ; preserve address given in r0 by copying it into r4

rsloop:
    bl      read_character  ; goto read_char subroutine to read the character put into the terminal
    strb    r0, [r4, r6]    ; store character into passed in address r4 into r0 using r6 as offset
    add     r6, r6, #1      ; r6++
    cmp     r0, #0x2D       ; check to see if the byte is -.  if it is, then we have a negative number
    it      eq              ; this *should* force conditional execution
    bleq    setNegFlag      ; if we encounter a negative number, set the regative flag
    cmp     r0, #0x0D       ; compare with the hex value for the enter byte except NOT because for SOME FUCKING REASON WINDOWS USING 0D 0A
    ; there is potential for a LINUX ONLY BUG HERE
    ; linux ends its lines with 0A, not 0D 0A.  This code only looks for 0D.  So it isn't explicitly accomodating for linux, potential for infinite loop
    bne     rsloop          ; if user hasn't inputted enter, loop

    sub     r6, r6, #1      ; subtract one from strlen to account for the enter byte

    ; so here we're just d  oing cleanup
    ;strb   r6, [r4, #-1]   ; storing string length metadata.  since its before the start of the string, its gonna be considered metadata
    ; that doesn't work because how one accesses memory is much more restricted than i once thought
    ldr     r7, ptr_to_space1; get a ptr to space 1 which is gonna be where we store string metadata
    ldr     r8, ptr_to_space2
    ; for numerator, we'll use the byte at space1, and we'll check to see if it has contents.  if it does, we'll use the byte at space2
    ; this will ensure that the demoninator will use space2
    ldrb    r10, [r7,#0]    ; grab the contents of ptr_to_space1 and shove it in r10
    cmp     r10, #0         ; check to see if ptr_to_space1 has stuff in it
    ite     eq              ; so if it does, then we shove it into denominator since we've already done this once before
    strbeq  r6, [r7, #0]    ; store the length of the numerator in memory
    strbne  r6, [r8, #0]    ; store the length of the denominator in memory

    and     r12, r12, #0    ; clear r12 so we can store a null byte at the end of the string
    strb    r12, [r4, r6]   ; store a null byte at the end of the string - this should replace the enter byte

    ; after this point we should be done
    mov     r0, r4          ; copy the address we saved in r4 back into r0, so the next subroutine can use it

    ; quickly, just print a newline
    push    {r0}
    mov     r0, #0xD
    bl      output_character
    mov     r0, #0xA
    bl      output_character
    pop     {r0}
    ; quick and dirty macro

    pop {r4-r12,lr}         ; restore registers all registers preserved in the
                            ; push at the top of this routine from the stack.
    mov pc, lr


output_character:
    push {r4-r12,lr}        ; store any registers in the range of r4 through r12
                            ; that are used in your routine.  include lr if this
                            ; routine calls another routine.

    mov     r6, #0xc000     ; don't use a temp reg
    movt    r6, #0x4000
    mov     r4, #0x20
                            ; looking for RxFF flag, offset 5 bits from #0x4000C018
tloop:                      ; Transmit loop
    ldrb    r1, [r6, #0x18] ; load flag reg to see if transmit buffer is full
    and     r7, r1, #0x20   ; bitmask looking for 1 in last place, store in r7 to preserve flag reg
    cmp     r7, r4          ; checking bitmask
    beq     tloop           ; if flag reg contains 1, loop
                            ; now we need to store r0 in data reg
    strb    r0, [r6, #0]    ; no offset needed, storing r0 into data reg


    pop {r4-r12,lr}         ; restore registers all registers preserved in the
                            ; push at the top of this routine from the stack.
    mov pc, lr



output_string:
    push {r4-r12,lr}
                            ; your code for your output_string routine is placed here
    mov     r5, r0          ; copy r0 into r5 to back it up
    ; i don't actually need to do that, just use post-indexing loads instead

str_out_loop:
    ldrb    r0, [r5], #1    ; load the character from r5+r4 (r4 is offset) into r0
    bl      output_character; print said character
    cmp     r0, #0          ; check to see if the printed out character is a null byte or not
    bne     str_out_loop    ; if we did not print out a null byte, loop

    pop {r4-r12,lr}         ; restore registers all registers preserved in the
                            ; push at the top of this routine from the stack.
    mov pc, lr

; Make sure no code calls this, gpio registers are not properly configured for this

read_from_push_btns:
    PUSH {r4-r12,lr}        ; Spill registers to stack

                            ; Your code is placed here
    mov     r1, #0x7000
    movt    r1, #0x4000         ; put address of port D in r1
poll:
    ldrb    r0, [r1, #0x3FC] ; load data byte into r0
    cmp     r0, #0
    beq     poll            ; make sure the user actually enters data into r0
                            ; poll until the user does
                            ; just return r0

    POP {r4-r12,lr}         ; Restore registers from stack
    MOV pc, lr

illuminate_LEDs:
    PUSH {r4-r12,lr}    ; Spill registers to stack

    ; we are assuming that there's input in r0 to store in memory

    mov     r1,#0x5000  ; Your code is placed here
    movt    r1,#0x4000  ; address of port B
    strb    r0, [r1, #0x3FC]    ; address of data reg
    ; after this, there's not much that needs to be done

    POP {r4-r12,lr}     ; Restore registers from stack
    MOV pc, lr

illuminate_RGB_LED:
    PUSH {r4-r12,lr}    ; Spill registers to stack

	mov		r1, #0x5000
	movt	r1, #0x4002				; address of port F

	; data reg shouldn't be here but try anyways i guess
	strb	r0, [r1, #0x3FC]			; write to GPIO

	pop 	{r4-r12,lr}  	; Restore registers from stack
	bx		lr


change_color:
    PUSH {r4-r12,lr}    ; Spill registers to stack
    ; we change color depending on the contents of r2
    ; use shift trickery to save instructions
    cmp     r2, #6      ; light blue , 0b1100, not listed color to display, so skip it
    it      eq
    addeq       r2, r2, #1  ; skips light blue

    cmp     r2, #7      ; we don't want it to be greater than 7
    it      gt
    movgt   r2, #1      ; if we iterate above 7, set to 1

    lsl     r2, #1      ; for example, 2 = 0b0010, but we are not using bit 0.  shifting it
                        ; left once turns it onto 0b0100, which turns it into something we can write to the datareg


    POP {r4-r12,lr}     ; Restore registers from stack
    MOV pc, lr

read_tiva_push_button:

    PUSH {r4-r12,lr}    ; Spill registers to stack

    ; set pull up resistor for pin 4
    ldr r1, [r3, #0x510]        ; offset
    AND r1, r1, #0x10           ; mask 4th pin to enable
    str r1, [r3, #0x510]        ; store masked bit

    ; read data register pin until receive 1
read_loop:
    ; change r0 to a different reg
    ldr r1, [r3, #0x3FC]        ; offset for gpio data register
    AND r1, r1, #0x10           ; mask 4th pin
    cmp r1, #0x10
    ite eq                      ; do stuff based on equality
    moveq r0, #0                ; if equal, return 0
    movne r0, #1                ; if not equal, return 1

    POP {r4-r12,lr}             ; Restore registers from stack
    MOV pc, lr

    .if newdm=0
div_and_mod: ; old version
    push {r4-r12,lr}    ; store any registers in the range of r4 through r12
                        ; that are used in your routine.  include lr if this
                        ; routine calls another routine.
                        ; your code for the div_and_mod routine goes here.
    and     r2, r2, #0  ; clear r2, used for iterator
                        ; movt comes first to deal with negatives
                        ; we need to test to see if the denominator is greater than the numerator, eg 2/5 - both reg need to be positive
    cmp     r0, r1      ; is r0 < r1?
    blt     r0ltr1      ; if so, branch to r0ltr1 which handles this case

divloop:                    ; this section is where we actually do the divison loop
    add     r2, r2, #1  ; r2++
    sub     r0, r0, r1  ; r0 := r0 - r1
    cmp     r0, r1      ; test to see if r0 >= r1.  at the end of the division, r0 should be smaller than r1
    bge     divloop     ; if r0 >= r1, then loop again
                        ; division over
    mov     r1, r0      ; r0 currently contains remainder, r1 is where remainder should be returned in
    mov     r0, r2      ; r2 is number of times iterated, so quotient.  copy into r0

    pop {r4-r12,lr}     ; restore registers all registers preserved in the
                        ; push at the top of this routine from the stack.
    mov pc, lr
    .endif

    .if newdm=1

div_and_mod: ; new version
    ; in1 = r0, in2 = r1.
    push    {r4-r12,lr}
    cmp     r0, r1
    blt     r0ltr1
    beq     r0eqr1

    mov     r4, #1  ; amount to multiply by
    mov     r5, #0  ; clear r5
dvmdl:
    mls     r5, r1, r4, r0 ; r5 = r0 - (r1*r4)
    add     r4, r4, #1
    cmp     r5, r1          ; is the result of the multiplication less than r1?  meaning, is in1 - (in2*r4) < y?  if so, we're done multiplying
    bgt     dvmdl
    ; r5 is the remainder, r4 is the quotient
    mov     r0, r4
    mov     r1, r5

    pop     {r4-r12,lr}
    bx      lr
    .endif

r0eqr1:
    ; this is the case that in1 == in2
    mov     r0, #1  ; quotient is always 1
    mov     r1, #0  ; remainder is always 0

    pop     {r4-r12, lr}
    mov     pc, lr


r0ltr1:
    mov     r1, r0      ; copy r0 into r1
    mov     r0, #0      ; copy a zero into r0
    movt    r0, #0

    pop {r4-r12,lr}     ; restore registers all registers preserved in the
    mov pc, lr


setNegFlag:
    push {r4-r12,lr}        ; store any registers in the range of r4 through r12

    and     r4, r4, #0      ; clear r4, just so its 0
    ldr r6, ptr_to_negFlag  ; grab the location of negFlag
    ldrb    r5, [r6, #0]    ; actually load negFlag's contents into r5
    eor     r5, r5, #1      ; xor r5 with 1, so that if this subroutine is called twice it'll unset the negative flag
    strb    r5, [r6, #0]    ; now that we've set or potentially unset the negative flag, shove it into memory again


    pop {r4-r12,lr}         ; restore registers all registers preserved in the
                            ; push at the top of this routine from the stack.
    mov pc, lr

read_character:
    push {r4-r12,lr}


    mov     r4, #0x10
    mov     r8, #0xc000
    movt    r8, #0x4000
                            ; looking for RxFE flag, which is stored at 0x4000C018
rloop:                      ; Recieve loop

    ldrb    r1, [r8, #0x18] ; first thing is to check flag reg to see if buffer is full, r2 contains first half, u0frRF is offset of 4
    and     r5, r1, #0x10   ; bitmask looking for 1 in last place, store in r5 to preserve flag reg
    cmp     r5, r4          ; check to see if flag reg is 0 or 1 - if 1, loop
    beq     rloop           ; if flag reg contains 1, loop
                            ; now there should be data to read
    ldrb    r0, [r8, #0]    ; no offset needed, just load data reg into r0 and return from routine

    ; print character out
    bl      output_character

    pop {r4-r12,lr}         ; restore registers all registers preserved in the
                            ; push at the top of this routine from the stack.
    mov pc, lr



asm_int2string:
    ; limitation: only will work for numbers with at most 3 digits.
    push    {r4-r12,lr}     ; store any registers in the range of r4 through r12
                            ; that are used in your routine.  include lr if this
                            ; routine calls another routine.
    mov     r5, r0          ; preserve the address in r0
    mov     r9, r1          ; preserve original number in r6
    mov     r0, r1          ; copy the number we're given into r0
    and     r8, r8, #0      ; clear r8, use this to contain the string - WE CAN CONTAIN THE WHOLE STRING IN ONE REG!
    ; now we need to determine the number of digits, we can do it by comparing to 99 and 9
    cmp     r1, #99         ; if its greater than 99, then it has to be 3 digits in size
    ittt    gt              ; if-then-then block based on it being greater than 99
    movgt   r10, #1
    movgt   r4, #3
    bgt     skip            ; skip the rest of the checks

    cmp     r1, #9          ; if its greater than 9, it must be 2 digits.  if its not, then it must be one digit
    itte    gt
    movgt   r10, #1         ; >1 byte flag
    movgt   r1, #2
    movle   r1, #1

skip:

    ; neg flag
    ldr     r7, ptr_to_negFlag; we need to know if we have a negative number
    ldrb    r6, [r7, #0]
    cmp     r6, #1          ; so if it is, then add 0x2D into r8 and shift it
    ittt    eq
    moveq   r10, #1
    addeq   r8, r8, #0x2D   ; 0x2D is hex for -
    lsleq   r8, #8          ; shift it by one byte, the smallest byte should be zero

    cmp     r1, #3          ; check to see if we have three digits
    beq     three_digits    ; if so, handle everything in three_digits.  repeat for 2 and 1

    cmp     r1, #2
    beq     two_digits

    cmp     r1, #1
    beq     one_digit


three_digits:

    mov     r1, #100        ; copy 100 into r1 to do a div_and_mod with 100, root out the 100ths place
    bl      div_and_mod
    add     r0, r0, #0x30   ; convert int to char
    add     r8, r8, r0      ; we want quotient because that'll tell us the 100ths place
                            ; r8 is where we're keeping track of the string, adding 0x30 turns it into ascii
    lsl     r8, #8          ; shift it by a byte, to keep the last two bytes 00
    mov     r0, r1          ; then just copy the remainder into r0
                            ; now we just do it for two digits

two_digits:

    mov     r1, #10         ; copy in 10 to r1
    bl      div_and_mod
    add     r0, r0, #0x30
    add     r8, r8, r0      ; same as earlier
    lsl     r8, #8
    mov     r0, r1

one_digit:

    ; unlike the other two times, if we only have 1 digit then we just add it to r8
    add         r0, r0, #0x30
    add         r8, r8, r0
    ; no need for shifting

    ; now at this point, given 435 with negative flag, r8 would look like: 0x2D343335 or "534-" because endianness. fix with rev instruction
    cmp     r10, #1 ; reverse byte order only if we have more than one character to print
    itttt   eq
    clzeq   r11, r8 ; figure out the number of leading zeros in r8 and store in r11 - eg, 0x3430 has 18 leading zeros
    subeq   r11, r11, #2 ; the amount we want to shift will be the leading zeros minus 2 to make it a printable string
    reveq   r8, r8
    lsreq   r8, r8, r11

    ; now we can just store it
    str     r8, [r5, #0]    ; address will still be in r5
    mov     r0, r5          ; restore address to r0
    mov     r1, r9          ; restore original int to r1

    pop {r4-r12,lr}         ; restore registers all registers preserved in the
                            ; push at the top of this routine from the stack.
    mov pc, lr



long2string:
    ; int2string but expanded for one full register
    ; r0: string to store long in
    ; r1: long to print
    push    {r4-r12,lr}

    mov     r9, r0      ; iterate using r9 to preserve value of r0
    mov     r6, #0x30   ; ascii 0
    strb    r6, [r9], #1; post indexed to move r9
    mov     r6, #0x78   ; ascii x
    strb    r6, [r9], #1

    mov     r10, #0     ; shift iterator
    mov     r11, #0     ; strlen iterator
    mov     r4, #0xF    ; iterate through all 8 nibbles
l2sloop:
    add     r10, r10, #4; increment by 4 each time
    add     r11, r11, #1; increment by 1 each time
    and     r5, r4, r1  ; get first value
    lsl     r4, r4, #4  ; shift by one nibble
    ; r5 might be something like 0x0A00, shift it back
    lsr     r5, r5, r10
    cmp     r5, #0xA    ; is r5 >= A?
    ite ge
    addge   r5, r5, #0x37; 0xA -> 'A' is difference of 0x37
    addlt   r5, r5, #0x30; 0x1 -> '1' is difference of 0x30
    strb    r5, [r9], #1
    cmp     r11, #7      ; if we've done this 7 times, break
    bne     l2sloop

    pop     {r4-r12,lr}


    .end
