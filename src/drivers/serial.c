#include <arch/csr.h>
#include <arch/plic.h>
#include <kernel/panic.h>
#include <kernel/types.h>
#include <kernel/serial.h>
#include <arch/spinlock.h>

#define UART_BASE 0xFFFFFFC010000000ULL
#define UART_RBR 0x00
#define UART_THR 0x00
#define UART_IER 0x01
#define UART_FCR 0x02
#define UART_LCR 0x03
#define UART_LSR 0x05
#define Reg(reg) ((volatile u8 *)(UART_BASE + (reg)))
#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))
#define SERIAL_BUF_SIZE 256

struct serialdev{
    char buf[SERIAL_BUF_SIZE];
    size_t len;
    struct spinlock lock;
}dev;

void serial_init(){
	dev.len = 0;
    WriteReg(UART_IER, 0x00);
    WriteReg(UART_LCR, 0x03);
    WriteReg(UART_FCR, 0x01);
    WriteReg(UART_IER, 0x01);
}

void serial_irq_enable(){
	plic_irq_set_priority(10, 1);
    plic_hart_set_threshold(0, 0);
    plic_hart_enable_irq(0, 10);
    csr_write(CSR_SIE, csr_read(CSR_SIE) | (1ULL << 9));
}

void serial_irq_disable(){
	csr_write(CSR_SIE, csr_read(CSR_SIE) & ~(1ULL << 9));
}

void serial_irq(){
	spin_lock(&dev.lock);
    while(ReadReg(UART_LSR) & 0x01){
        char c = ReadReg(UART_RBR);
        if (dev.len < SERIAL_BUF_SIZE)
            dev.buf[dev.len++] = c;
    }
    spin_unlock(&dev.lock);
}

size_t serial_read(char *buf){
	u64 flags = hart_irq_save();
    spin_lock(&dev.lock);
    size_t size = dev.len;
    for(size_t i = 0; i < size; i++){
        buf[i] = dev.buf[i];
    }
    dev.len = 0;
    spin_unlock(&dev.lock);
    hart_irq_restore(flags);
    return size;
}

void serial_puts(char *str){
	while(*str)
        serial_putc(*str++);
}

void serial_putc(char c){
	while((ReadReg(UART_LSR) & (1 << 5)) == 0){}
    WriteReg(UART_THR, c);
}
