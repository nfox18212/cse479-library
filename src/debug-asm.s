	.cdecls C,NOLIST,"debug.h"

clc .macro
	push	{r0}F
	mov		r0, #0xC	; form feed
	bl	output_character
	pop		{r0}
	.endm

newl .macro				; print a newline
	push 	{r0}
	mov		r0, #0xD
	bl		output_character
	mov		r0, #0xA
	bl		output_character
	pop		{r0}
	.endm

peightyspaces .macro
	push	{r0}
	ldr		r0, ptr_to_eightyspaces
	bl		output_string
	pop		{r0}
	.endm

psixtyspaces .macro
	push	{r0}
	ldr		r0, ptr_to_sixtyspaces
	bl		output_string
	pop		{r0}
	.endm
; these macros exist to print whitespace out.  Exactly 60 and 80 spaces, and they both preserve r0
calculate_offset .macro xpos, ypos, offset
	; leaf macro,  offset = 22*ypos + xpos
	push	{r4,r5}
	mov		r5, #22
	mul		r4, ypos, r5
	add		offset, r4, xpos
	pop		{r4,r5}
	.endm






.data

didcrash:	.byte  0x0
baddr:		.word  0x0

context: 	.space 0x40	; reserve 64 bytes of space
caddr:		.word  0x0	; context address

	.global	didcrash
	.global baddr
	.global context
	.global caddr

	.text


	.global crash
	.global getdidcrash
	.global getbaddr


	.if en_uart_out=1
	.global output_character
	.global output_string
	.endif

	.if en_uart_out=0
	.global uprintf ; c function to print to uart
	.endif

didcrashp:  .word didcrash
baddrp:     .word baddr
contextp:	.word context
caddrp:		.word caddr



getdidcrash: ; routine called from C
	push	{r5}
	ldr		r5, didcrashp
	ldr		r0, [r5, #0] 	; get the didcrash value from memory
	pop		{r5}
	bx		lr				; returns from c...but doesn't set the variable to r0?

getbaddr: ; routine called from C
	push	{r5}
	ldr		r5, baddrp
	ldr		r0, [r5, #0]
	pop		{r5}
	bx		lr


crash:
	; takes crash string in r0
	; print the crash string
	bl		output_string
	; make this code uninterruptible
	cpsid	i	; disables interrupts and configurable faults

	; we are going to preserve the registers by putting them in memory
	; r0-r12 and lr is 14 registers, so 14 words.  14*4 = 56 bytes.  Also preserve the pointer so add 4 more bytes. 60 bytes total.  Add 4 more bytes for padding.  64 bytes.
	; we need to use a register for loading the context register, push r12
	push	{r12}
	ldr		r12, contextp ; get pointer to context
	stmea	r12!, {r0-r2,r4-r11} ; preserve all registers except r0
	; now we can overwrite those registers
	mov		r4, r12	 		; copy contextp to r4
	pop		{r12}			; restore original value of r12
	stmea	r4!, {r12,lr}	; preserve r12
	ldr		r5, caddrp		; get the address of where we're going to store the context address
	str		r4, [r5, #0]	; store it
	; now we've fully preserved the registers

	; set custom memory location to set that we intetionally caused a fault
	ldr		r4, didcrashp
	mov		r5, #1			; this signifies that the program was intentionally crashed
	strb	r5, [r4, #0]	; store it

	; store the link register into baddr
	; the link register stores the next instruction to execute, store it there
	ldr		r5, baddrp
	str		lr, [r5, #0] ; store lr


    ;; custom crash string in r0
    .if en_uart_out=1
	;clc
	newl
	bl		output_string
    .endif

    .if en_uart_out=0
    bl      uprintf
    .endif

    ; fault
    cpsie	i	; enable interrupts
	mov		r3, #0xFFFF
	movt	r3, #0xFFFF
	ldr		r3, [r3, #0] ; attempt to load invalid memory


goback:	
	; if need be, there's also baddrp so if this doesn't work we can grab it from there
	;push	{r3,r4}		; preserve r3/4 so we can use it as a temporary register
	;ldr		r3, didcrashp
	;ldr		r4, [r3]	; find out if the crash was caused by the crash subroutine
	;mov		r3, r4		; restore r4
	;pop		{r4}
	;cmp		r3, #1
	;bne		ngoback

	push	{r3}
	ldr		r3, caddrp
	ldr		r0, [r3, #0] ; get the caddr address - to restore context
	ldmea	r0!, {r4-r12,lr} ; restore registers
	; above line is broken
	mov		r3, r0 			 ; copy r0 into r3 to restore r0, r1 and r2
	ldmea 	r3!, {r0-r2}	 ; restore r0, r1, and r2 
	pop		{r3}			 ; restore r3
	; bl copies pc+4 into lr, we want instruction before so reduce lr by 8
	sub		lr, lr, #8		; ho-ly this is unsafe
	mov		pc, lr 			; we've preserved lr so goto it

ngoback:
	; we can't assume we can restore context.  just return to address that faulted
	sub		r0, r0, #4		; reduce address by 4 to return to instruction before faulted instruction
	bx		r0

test:
	; test to see if its possible to return a value to C and take in a value from C
	add		r0, r0, #5
	bx		lr

	.end
