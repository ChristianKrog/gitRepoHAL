#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 8
char buffer[BUFFER_SIZE];
char export_buffer[8] = "12";
char direction_buffer[3] = "in";
int fd, fd1, fd2;

int main(int argc, char* argv[])
{

// fd1 = open("/sys/class/gpio/export", O_WRONLY);
// int fd1_num;
// fd1_num = write(fd1, export_buffer, strlen(export_buffer));

// if(fd1_num == -1){
// 	printf("Error in writing to export. %s", strerror(errno));
// }
// else{
// 	printf("bytes written to export: %d\n", fd1_num);
	
// }
// close(fd1);

// fd2 = open("/sys/class/gpio/gpio12/direction", O_WRONLY);
// int fd2_num;
// fd2_num = write(fd2, direction_buffer, strlen(direction_buffer));
// if(fd2_num == -1){
// 	printf("Error in writing to direction: %s", strerror(errno));
// }
// else{
// 	printf("bytes written to direction: %d\n", fd2_num);
	
// }
// close(fd2);

for(;;){
fd = open("/dev/button", O_RDONLY);
int num_read;
num_read = read(fd, buffer, 1);

if(num_read == -1){
	printf("buuuh! Error: %s\n", strerror(errno));
	sleep(1);
	close(fd);
}
else{
	buffer[num_read] = 0;
	printf("Success! read: %s\n", buffer);
	sleep(1);
	close(fd);
}
}
}
