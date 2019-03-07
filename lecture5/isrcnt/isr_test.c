#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 16
char buffer[BUFFER_SIZE];
char export_buffer[8] = "12";
char direction_buffer[3] = "in";
int fd, fd1, fd2;

int main(int argc, char* argv[])
{
	const char s[2] = ":";
	char *token;




printf("cnt_isr\t isr_but_val\t cnt_read\t read_but_val\n");

fd = open("/dev/button", O_RDONLY);
for(;;){

int num_read;
num_read = read(fd, buffer, 24);

if(num_read == -1){
	printf("buuuh! Error: %s\n", strerror(errno));
	close(fd);
	break;
}
else{
	buffer[num_read] = 0;

	token =strtok(buffer, s);

	while(token != NULL){
	        printf("%s\t", token);

	        token = strtok(NULL, s);
	}

	printf("\n");
}
}
}

