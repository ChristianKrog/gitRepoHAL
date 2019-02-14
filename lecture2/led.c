#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 8
char buffer[BUFFER_SIZE];
char export_buffer[3] = "20";
char direction_buffer[4] = "out";
int fd, fd1, fd2;


int main(int argc, char* argv[])
{
    buffer[0] = 48;
    fd1 = open("/sys/class/gpio/export", O_WRONLY);
    int fd1_num;
    fd1_num = write(fd1, export_buffer, strlen(export_buffer));
if(fd1_num == -1){
    printf("Error in writing to export. %s", strerror(errno));
}
else{
    printf("bytes written to export: %d", fd1_num);
    close(fd1);
}

    fd2 = open("/sys/class/gpio/gpio20/direction", O_WRONLY);
    int fd2_num;
    fd2_num = write(fd2, direction_buffer, strlen(direction_buffer));

if(fd2_num == -1){
    printf("Error in writing to direction: %s", strerror(errno));
    }

else{
    printf("bytes written to direction: %d", fd2_num);
    close(fd2);
}

for(;;){
    fd = open("/sys/class/gpio/gpio20/value", O_WRONLY);
    int num_read;
    num_read = write(fd, buffer, 1);

if(num_read == -1){
    printf("buuuh! Error: %s", strerror(errno));
    sleep(1);
    close(fd);
}
else{
    sleep(1);
    close(fd);
    buffer[0] ^= 1;
}
}
}
