#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h>
#include <kernel/printf.h>

extern int _hartid[];

int atoi(const char *str){
    int res = 0;
    while(*str >= '0' && *str <= '9'){
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void kmain()
{
	printk_set_level(LOG_DEBUG);
	info("entered S-mode\n");
	info("booting on hart %d\n", _hartid[0]);
	info("setting up virtual memory...\n");
	vm_init();

	info("enabling traps...\n");
	trap_setup();
	info("enabling timer...\n");
	timer_irq_enable();
	info("enabling serial...\n");
	serial_init();
	serial_irq_enable();

	hart_irq_enable();

    char input_buf[128];
    size_t input_pos = 0;
    
    serial_puts("> ");
	
    while(1){
        char recv_buf[64];
        size_t n = serial_read(recv_buf);
        for(size_t i = 0; i < n; i++){
            char c = recv_buf[i];
            if(c == '\r'){
                serial_puts("\r\n");
                input_buf[input_pos] = '\0';
                if(input_pos > 0){
                    if(strncmp(input_buf, "uptime", 6) == 0 && (input_buf[6] == ' ' || input_buf[6] == '\0')){
                        u64 uptime_secs = timer_read() / 10000000;
                        info("%llus\n", uptime_secs);
                    }
                    else if(strncmp(input_buf, "echo ", 5) == 0){
                        serial_puts(&input_buf[5]);
                        serial_puts("\r\n");
                    }
                    else if(strncmp(input_buf, "alarm ", 6) == 0){
                        int secs = atoi(&input_buf[6]);
                        timer_set_alarm((u64)secs);
                    } 
                    else
                        serial_puts("Comando desconhecido\r\n");
                }
                input_pos = 0;
                serial_puts("> ");
            }
            else if(c == '\b' || c == 0x7F){
                if(input_pos > 0){
                    input_pos--;
                    serial_puts("\b \b");
                }
            }
            else{
                if(input_pos < sizeof(input_buf) - 1){
                    serial_putc(c);
                    input_buf[input_pos++] = c;
                }
            }
        }
    }
}
