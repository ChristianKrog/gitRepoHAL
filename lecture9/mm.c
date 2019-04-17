#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define BCM2835_PERIPH_BASE	0x20000000
#define BCM2835_PERIPH_SIZE	0x01000000

unsigned int * vmem_addr, vmem_gpio_addr, vmem_clock_addr, vmem_pwm_addr;
int mem_fd;
unsigned int gpio_offset = 	0x200000; 
unsigned int clock_offset = 0x101000;
unsigned int pwm_offset =	0x20c000;

int main(int argc, char * argv[])
{
	mem_fd = open("/dev/mem", O_RDWR);

	vmem_addr = mmap(NULL, BCM2835_PERIPH_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                           mem_fd, BCM2835_PERIPH_BASE);
       
	printf("Pointer address to virtual memory: %p\n", vmem_addr);

	vmem_gpio_addr = vmem_addr + gpio_offset/4;

	vmem_clock_addr = vmem_addr + clock_offset/4;

    int err = munmap(vmem_addr, BCM2835_PERIPH_SIZE);

    if(err != 0)printf("Error in munmap. %s", strerror(errno));


    return 0;
}