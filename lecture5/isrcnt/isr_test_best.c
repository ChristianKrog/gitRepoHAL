#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 64
#define arr_len 64
char buffer[BUFFER_SIZE];
int fd, c, samples;

const char s[2] = ":";

int cnt_isr[arr_len];
int val_isr[arr_len];
int cnt_read[arr_len];
int val_read[arr_len];

char tok1[16];
char tok2[16];
char tok3[16];
char tok4[16];

char *strPtrArr[4] = {tok1, tok2, tok3, tok4};
int *table[4] = {cnt_isr, val_isr, cnt_read, val_read};
int j = 0;

void insert(int **table, char **ins_buf)
{
        int i;
        for(i = 0; i < 4; i++)
        {
                table[i][c] = atoi(ins_buf[i]);
               printf("item [%d] in [%d] is: [%d]\n", c, i, (table[i][c]));        
		//printf("%d\t", atoi(ins_buf[i]));
	}

        printf("\n");
        
}

char **split(char *str, char **tokArr)
{
	int tokencnt = 0; 

	tokArr[tokencnt] = strtok(str, s);
	tokencnt++;
	while(tokencnt < 4){
	//	printf("token[%d]:%s\n", tokencnt, tokArr[tokencnt]);
	        tokArr[tokencnt] = strtok(NULL, s);
		tokencnt++;
	}
	return tokArr;
}

void treat(void)
{
	int cnt_diff, val_diff = 0;
	for(j = 0; j < samples; j++)
	{
		cnt_diff += (table[0][j]) - (table[2][j]);
		
		val_diff += (table[1][j]) - (table[3][j]);
		
		//int i;
		//for(i = 0; i < 4; i++)
		//{

	        //printf("item [%d] in [%d] is: [%d]\n", j, i, (table[i][j]));        		}
		//printf("\n");
	}
	printf("cnt_diff = %d \t val_diff = %d\n", cnt_diff, val_diff);
}

int main(int argc, char* argv[])
{

//table* = {cnt_isr, val_isr, cnt_read, val_read};

samples = atoi(argv[1]);

fd = open("/dev/button", O_RDONLY);

for(j =0; j < samples; j++)
{
	int num_read;
	num_read = read(fd, buffer, 24);

	if(num_read == -1)
	{
		printf("buuuh! Error: %s\n", strerror(errno));
		close(fd);
		break;
	}
	else
	{
		buffer[num_read] = 0;	
		insert(table, split(buffer, strPtrArr));
		c++;
	//	printf("\n");
	}	
}
close(fd);
treat();
}


