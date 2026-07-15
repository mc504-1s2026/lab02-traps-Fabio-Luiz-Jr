#include <arch/csr.h>
#include <arch/plic.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/panic.h>
#include <kernel/serial.h>

/* defined in src/trap_entry.S */
extern void trap_entry();

void handle_irq(){
    u64 cause = csr_read(CSR_SCAUSE);
    u64 irq_code = cause & ~(1ULL << 63);
	u32 irq;

    if(irq_code == 5)
		timer_irq();
    else if(irq_code == 9){
		irq = plic_hart_claim_irq(0);
		if (irq == 10)
			serial_irq();
		else if(irq != 0)
			info("Interrupção externa desconhecida. IRQ %d\n", irq);
		if(irq != 0)
			plic_hart_complete_irq(0, irq);
    }else
        info("Interrupção não tratada! Código: %llu\n", irq_code);
}

void handle_exception(){
	u64 cause = csr_read(CSR_SCAUSE);
    u64 epc = csr_read(CSR_SEPC);
    u64 tval = csr_read(CSR_STVAL);
    panic("Exceção! Cause: %llu, EPC: %llx, TVAL: %llx\n", cause, epc, tval);
}

void trap_setup(){
	csr_write(CSR_STVEC, (u64)trap_entry);
}

void handle_trap(){
	u64 cause = csr_read(CSR_SCAUSE);
    int is_interrupt = (cause >> 63) & 1;
    if(is_interrupt)
        handle_irq();
    else
        handle_exception();
}

void hart_irq_enable(){
	csr_write(CSR_SSTATUS, csr_read(CSR_SSTATUS) | (1ULL << 1));
}

u64 hart_irq_save(){
	u64 status = csr_read(CSR_SSTATUS);
    hart_irq_disable();
    return status;
}

void hart_irq_restore(u64 flags){
	csr_write(CSR_SSTATUS, flags);
}

void hart_irq_disable(){
	csr_write(CSR_SSTATUS, csr_read(CSR_SSTATUS) & ~(1ULL << 1));
}
