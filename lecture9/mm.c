#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define u32 unsigned int

#define GPIO_INPUT 		0b000 	// GPIO Pin is an input
unsigned int GPIO_OUTPUT = 0x00000001; 	// GPIO Pin is an output
#define GPIO_ALT0 		0b100	// GPIO Pin takes alternate function 0
#define GPIO_ALT1		0b101 	// GPIO Pin takes alternate function 1
#define GPIO_ALT2		0b110 	// GPIO Pin takes alternate function 2
#define GPIO_ALT3		0b111	// GPIO Pin takes alternate function 3
#define GPIO_ALT4		0b011	// GPIO Pin takes alternate function 4
#define GPIO_ALT5		0b010	// GPIO Pin takes alternate function 5 

#define GPFSEL0 		0x00000000
#define GPFSEL1 		0x00000004
#define GPFSEL2 		0x00000008
#define GPFSEL3 		0x0000000c
#define GPFSEL4 		0x00000010
#define GPFSEL5 		0x00000014

unsigned int GPSET0 = 0x0000001c;
#define GPSET1			0x00000020

#define GPCLR0			0x00000028
#define GPCLR1			0x0000002c

#define GPLEV0 			0x00000034

#define BCM2835_PERIPH_BASE	0x20000000
#define BCM2835_PERIPH_SIZE	0x01000000

unsigned int* vmem_addr;
volatile unsigned int * vmem_gpio_addr, vmem_clock_addr, vmem_pwm_addr;
int mem_fd;
unsigned int gpio_offset = 	0x200000; 
unsigned int clock_offset = 0x101000;
unsigned int pwm_offset =	0x20c000;

void reg_setbit(volatile unsigned int * addr, unsigned int bit);
void reg_clrbit(volatile unsigned int * addr, unsigned int bit);
unsigned int reg_rd(volatile unsigned int * addr);
void reg_wr(volatile unsigned int* addr, unsigned int value);
void set_output(volatile unsigned int * addr, unsigned int pin);

int main(int argc, char * argv[])
{
	mem_fd = open("/dev/mem", O_RDWR);

	vmem_addr = mmap(NULL, BCM2835_PERIPH_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                           mem_fd, BCM2835_PERIPH_BASE);
       
	printf("Pointer address to virtual memory: %p\n", vmem_addr);

	vmem_gpio_addr = vmem_addr + gpio_offset/4;

	vmem_clock_addr = (volatile unsigned int)(vmem_addr + clock_offset/4);

	//reg_wr(vmem_gpio_addr + GPFSEL2, 0);
	//reg_wr((volatile unsigned int *)((u32)vmem_gpio_addr + GPSET0), 0);

	/*for(int i = 0; i < 32; i++)
	{
		reg_clrbit((volatile unsigned int *)((u32)vmem_gpio_addr + GPCLR0), i);
	}
*/

	set_output(vmem_gpio_addr, 20);

	for(int i = 0; i < 10; i++)
	{

	reg_rd((volatile unsigned int *)((u32)vmem_gpio_addr + GPLEV0));
	
	if(i % 2 == 1)reg_setbit((volatile unsigned int *)((u32)vmem_gpio_addr + GPSET0), 20);

	else reg_clrbit((volatile unsigned int *)((u32)vmem_gpio_addr + GPCLR0), 20);

	//printf("evenuneven%d\n", i % 2);

	sleep(1);

	reg_rd((volatile unsigned int *)((u32)vmem_gpio_addr + GPLEV0));

	}

    int err = munmap(vmem_addr, BCM2835_PERIPH_SIZE);

    if(err != 0)printf("Error in munmap. %s", strerror(errno));

    close(mem_fd);


    return 0;
}

void set_output(volatile unsigned int * addr, unsigned int pin)
{

	unsigned int reg, t = pin / 10;
	if(t == 0) reg = GPFSEL0;
	else if(t == 1) reg = GPFSEL1;
	else if(t == 2) reg = GPFSEL2;
	else if(t == 3) reg = GPFSEL3;
	else if(t == 4) reg = GPFSEL4;
	else if(t == 5) reg = GPFSEL5;
	else printf("set_output error! pin number not in range\n");
	
	volatile unsigned int * a = (volatile unsigned int *)((unsigned int)addr + reg);

	unsigned int b = (GPIO_OUTPUT << (pin % 10));
	//printf("b = %x\n", b);

	reg_wr(a, b);
	//printf("t = %u reg_wr called with value: %x\n", t, (GPIO_OUTPUT << (pin % 10)));
	//printf("GPFSEL%u got written to %x\n", t, (u32)*a);
}

void reg_setbit(volatile unsigned int * addr, unsigned int bit)
{
	//printf("reg_setbit *addr = %x\n", (u32)*addr);
	//printf("reg_setbit bit = %u\n", bit);
	*addr = ((u32)*addr | (1 << bit));
	//printf("reg_setbit: register at addr %p is set to %x\n", addr, (u32)*addr);
}

void reg_clrbit(volatile unsigned int * addr, unsigned int bit)
{
	unsigned int value = (u32)*addr & (1 << bit);
	*addr = value;
	//printf("register at addr %p is set to %x\n", addr, (u32)*addr);
}

unsigned int reg_rd(volatile unsigned int * addr)
{
	printf("register at addr %p is read as %x\n", addr, (u32)*addr);
	return (u32)*addr;
}

void reg_wr(volatile unsigned int* addr, unsigned int value)
{
	*addr = value;
	
	//printf("reg_wr: register at addr %p is set to %x\n", addr, (u32)*addr);
}