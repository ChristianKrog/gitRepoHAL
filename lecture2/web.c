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
char start_tags[20] = "<html><body><h1>";
char web_desciptor[40] = "Temperature in degrees C: ";
char end_tags[20] = "</h1></body></html>";
char string_to_html[80];
int fd, fd1, fd2, ledwrite, webwrite;

void int_to_ascii(char* read_value, char* parsed){
    parsed[0] = (read_value[0] / 10) + '0';
    parsed[1] = (read_value[0] % 10) + '0';
    parsed[2] = 0x00;
}

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

    int fd_led_value, fd_web;
    fd_led_value = open("/sys/class/gpio/gpio20/value", O_WRONLY);
    buffer[i2c_read] = 0;
    printf("Temperature in degrees C: %d\n", buffer[0]);

    char ascii_value[2]; 
    int_to_ascii(buffer, ascii_value);

    strcpy(string_to_html, start_tags);
    strcat(string_to_html, web_desciptor);
    strcat(string_to_html, ascii_value);
    strcat(string_to_html, end_tags);

    fd_web = open("/www/pages/index.html", O_WRONLY);
    webwrite = write(fd_web, string_to_html, 50);
    if(webwrite == -1){
        printf("Write to index.html error %s", strerror(errno));
    }
    close(fd_web);

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
