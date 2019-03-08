#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 16
#define arr_len 64
char buffer[BUFFER_SIZE];
int fd, c;

const char s[2] = ":";

int cnt_isr[arr_len];
int val_isr[arr_len];
int cnt_read[arr_len];
int val_read[arr_len];

char *strPtrArr[4];

void insert(int **table, char **ins_buf)
{
        int i;
        for(i = 0; i < 4; i++)
        {
                table[i][c] += atoi(ins_buf[i]);
                printf("item [%d] in [%d] is: [%d]\n", c, i, (table[i][c]));        
	}

        printf("\n");
        
}

char **split(char *str, char **tokArr)
{
	int tokencnt = 0; 

	tokArr[tokencnt] = strtok(str, s);
	tokencnt++;
	while(tokArr[tokencnt] != NULL){
	        tokArr[tokencnt] = strtok(NULL, s);
		tokencnt++;
	}
	return tokArr;
}


int main(int argc, char* argv[])
{


int *table[4] = {cnt_isr, val_isr, cnt_read, val_read};

fd = open("/dev/button", O_RDONLY);

int j = 0;
for(j=0; j < atoi(argv[1]) ; j++)
{

int num_read;
num_read = read(fd, buffer, 24);

if(num_read == -1){
	printf("buuuh! Error: %s\n", strerror(errno));
	close(fd);
	break;
}
else{
	buffer[num_read] = 0;
	
	insert(table, split(buffer, strPtrArr));
	c++;

	printf("\n");
}
}

for(j=0; j < atoi(argv[1]) ; j++)
{
int i;
for(i = 0; i < 4; i++)
{
        printf("item [%d] in [%d] is: [%d]\n", c, i, (table[i][c]));        
}
printf("\n");
}

}


