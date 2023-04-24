#ifndef MPX_INTERRUPTS_H
#define MPX_INTERRUPTS_H

/**
 @file mpx/interrupts.h
 @brief Kernel functions related to software and hardware interrupts
*/

#define PIC_EOI      (0x20)
#define PIC_READ_ISR (0x0b)

#define PIC_1      (0x20)
#define PIC_1_CMD  (PIC_1)
#define PIC_1_MASK (PIC_1 + 1)
#define IRQV_BASE (0x20)

#define IRQ_BIT(irq) (1 << (irq))

/** Disable interrupts */
#define sti() __asm__ volatile ("sti")

/** Enable interrupts */
#define cli() __asm__ volatile ("cli")

/**
 Installs the initial interrupt handlers for the first 32 IRQ lines. Most do a
 panic for now.
*/
void irq_init(void);

/**
 Initializes the programmable interrupt controllers and performs the necessary
 remapping of IRQs. Leaves interrupts turned off.
*/
void pic_init(void);

/** Creates and installs the Interrupt Descriptor Table. */
void idt_init(void);

/** Installs an interrupt handler */
void idt_install(int vector, void (*handler)(void *));

#endif
