#include <arch/csr.h>
#include <arch/timer.h>
#include <kernel/panic.h>
#include <kernel/printf.h>

#define TIMER_FREQ 10000000

static u64 uptime = 0;
static u64 alarm_time = 0;

u64 timer_read(){
	return csr_read(CSR_TIME);
}

void timer_irq_enable(){
	csr_write(CSR_STIMECMP, timer_read() + TIMER_FREQ);
    csr_write(CSR_SIE, csr_read(CSR_SIE) | (1ULL << 5));
}

void timer_irq_disable(){
	csr_write(CSR_SIE, csr_read(CSR_SIE) & ~(1ULL << 5));
}

void timer_set_alarm(u64 secs){
	alarm_time = uptime + secs;
}

void timer_irq(){
	uptime++;
    if(alarm_time > 0 && uptime >= alarm_time){
        printk(LOG_INFO, "alarm\n");
        alarm_time = 0;
    }
    csr_write(CSR_STIMECMP, timer_read() + TIMER_FREQ);
}
