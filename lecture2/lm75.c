#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 8
char buffer[BUFFER_SIZE];
char export_buffer[8] = "20";
char direction_buffer[4] = "out";
char ledLow[2] = "0";
char ledHigh[2] = "1";
int fd, fd1, fd2, ledwrite;


int main(int argc, char* argv[]){
    fd1 = open("/sys/class/gpio/export", O_WRONLY);
    int fd1_num;
    fd1_num = write(fd1, export_buffer, strlen(export_buffer));

if(fd1_num == -1){
    printf("Error in writing to export. %s\n", strerror(errno));
    close(fd1);
}
else{
    printf("bytes written to export: %d\n", fd1_num);
    close(fd1);
}

    fd2 = open("/sys/class/gpio/gpio20/direction", O_WRONLY);
    int fd2_num;
    fd2_num = write(fd2, direction_buffer, strlen(direction_buffer));

if(fd2_num == -1){
    printf("Error in writing to direction: %s\n", strerror(errno));
    close(fd2);
    }

else{
    printf("bytes written to direction: %d\n", fd2_num);
    close(fd2);
}
for(;;){
    fd = open("/dev/i2c-1", O_RDWR);

    if(fd == -1){
        printf("fd error %s\n", strerror(errno));
    }

    ioctl(fd, 0x0703, 0x48);
    int i2c_read;
    i2c_read = read(fd, buffer, 2);

if(i2c_read == -1){
    printf("Read Error: %s\n", strerror(errno));
    sleep(1);
    close (fd);
}
else{

    int fd_led_value;
    fd_led_value = open("/sys/class/gpio/gpio20/value", O_WRONLY);
    buffer[i2c_read] = 0;
    printf("Temperature in degrees C: %d\n", buffer[0]);

    if(buffer[0] > 29){ 
        ledwrite = write(fd_led_value, ledHigh, 1);
    }

    else{
        ledwrite = write(fd_led_value, ledLow, 1);
    }

    sleep(1);
    close(fd);
    close(fd_led_value);
}
}
}
