bits 32
global rtc_isr, sys_call_isr, serial_isr

; RTC interrupt handler
; Tells the slave PIC to ignore interrupts from the RTC
rtc_isr:
	cli
	push ax
	
	; Tell the PIC this is EOI
	; This really should be done
	; at the RTC level -- but this is
	; okay for now...
	mov al, 0x20
	out 0xA0, al
	
	pop ax
	sti
	iret

;;; System call interrupt handler. To be implemented in Module R3.
extern sys_call			; The C function that sys_call_isr will call
sys_call_isr:
    pushad
    push ss
    push ds
    push es
    push fs
    push gs
    push esp            ; Recall 'push esp' will push the previous value of esp, which will point to gs
	call sys_call
    cmp eax, 0
    je sys_call_isr_nocswitch   ; R or W, then just pop, else set the stack pointer
    mov esp, eax
    jmp sys_call_isr_ret
sys_call_isr_nocswitch:
    add esp, 4
sys_call_isr_ret:
    pop gs
    pop fs
    pop es
    pop ds
    pop ss
    popad
    iret

;;; Serial port ISR. To be implemented in Module R6
serial_isr:
	iret
