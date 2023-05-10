#include <inc/assert.h>
#include <inc/irq.h>
#include <inc/trap.h>

// Current IRQ mask.
// Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
uint16_t irq_mask_8259A = 0xFFF8;
static int didinit;

/* Initialize the 8259A interrupt controllers. */
void pic_init(void) {
    didinit = 1;

    // mask all interrupts
    outb(IO_PIC1 + 1, 0xFF);
    outb(IO_PIC2 + 1, 0xFF);

    // Set up master (8259A-1)

    // ICW1:  0001g0hi
    outb(IO_PIC1, 0x11);

    // ICW2:  Vector offset
    outb(IO_PIC1 + 1, IRQ_OFFSET);

    // ICW3:  bit mask of IR lines connected to slave PICs (master PIC),
    outb(IO_PIC1 + 1, 1 << IRQ_SLAVE);

    // ICW4:  000nbmap
    outb(IO_PIC1 + 1, 0x3);

    // Set up slave (8259A-2)
    outb(IO_PIC2, 0x11);                // ICW1
    outb(IO_PIC2 + 1, IRQ_OFFSET + 8);  // ICW2
    outb(IO_PIC2 + 1, IRQ_SLAVE);       // ICW3
    outb(IO_PIC2 + 1, 0x01);            // ICW4

    // OCW3:  0ef01prs
    outb(IO_PIC1, 0x68); /* clear specific mask */
    outb(IO_PIC1, 0x0a); /* read IRR by default */

    outb(IO_PIC2, 0x68); /* OCW3 */
    outb(IO_PIC2, 0x0a); /* OCW3 */

    if (irq_mask_8259A != 0xFFFF) irq_setmask_8259A(irq_mask_8259A);
    // outb(0x20, 0x11);
    // outb(0x21, 0x20);
    // outb(0x21, 0x04);
    // outb(0x21, 0x01);
    // // slave
    // outb(0xA0, 0x11);
    // outb(0xA1, 0x28);
    // outb(0xA1, 0x02);
    // outb(0xA1, 0x01);
    // // unmask all irqs
    // outb(0x21, 0x0);
    // outb(0xA1, 0x0);
}

void irq_setmask_8259A(uint16_t mask) {
    int i;
    irq_mask_8259A = mask;
    if (!didinit) return;
    outb(IO_PIC1 + 1, (char)mask);
    outb(IO_PIC2 + 1, (char)(mask >> 8));
    cprintf("enabled interrupts:");
    for (i = 0; i < 16; i++)
        if (~mask & (1 << i)) cprintf(" %d", i);
    cprintf("\n");
}
