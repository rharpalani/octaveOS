/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
unsigned char master_mask = NEG_ONE; /* IRQs 0-7  */
unsigned char slave_mask = NEG_ONE;  /* IRQs 8-15 */
unsigned char set_mask = 0;

/* Initialize the 8259 PIC */
void i8259_init(void) {

    // initialize master and slave PIC
    outb(ICW1, MASTER_CMD);
    outb(ICW1, SLAVE_CMD);

    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW2_SLAVE, SLAVE_DATA);

    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);

    outb(ICW4, MASTER_DATA);
    outb(ICW4, SLAVE_DATA);

    // masks all interrupts initially
    outb(master_mask, MASTER_DATA);
    outb(slave_mask, SLAVE_DATA);

    enable_irq(SLAVE_IRQ);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    set_mask = 0;

    if (irq_num < MAX_PIC) {
        set_mask = 1 << irq_num;
        master_mask = master_mask & ~set_mask;
        outb(master_mask, MASTER_DATA);
    } else {
        set_mask = 1 << (irq_num - MAX_PIC);
        slave_mask = slave_mask & ~set_mask;
        outb(slave_mask, SLAVE_DATA);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    set_mask = 0;

    if (irq_num < MAX_PIC) {
        set_mask = 1 << irq_num;
        master_mask = master_mask | set_mask;
        outb(master_mask, MASTER_DATA);
    } else {
        set_mask = 1 << (irq_num - MAX_PIC);
        slave_mask = slave_mask | set_mask;
        outb(slave_mask, SLAVE_DATA);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    // irq came from slave, necessary to send command to both PICs
    if(irq_num >= MAX_PIC) {
		outb(EOI | (irq_num - MAX_PIC), SLAVE_CMD);
        outb(EOI | (SLAVE_IRQ), MASTER_CMD);    // slave is in IRQ 2 of master
    } else {
        outb(EOI | irq_num, MASTER_CMD);
    }
}
