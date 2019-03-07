#include <stdio.h>
#include <string.h>

#define arr_len 64

unsigned int cnt_isr[arr_len];
unsigned int val_isr[arr_len];
unsigned int cnt_read[arr_len];
unsigned int val_read[arr_len];

char ins_buf[2] = "2";

int insert(unsigned int **table, char * ins_buf)
{
	int i, c;
	for(i = 0; i < 4; i++)
	{
		table[i] += ins_buf[0];
		//printf("item %d in table %d\t is: %d", c, i, *(table[i]+c));

	}
	printf("%d\n", table[i]);
	c++;
}


int main(int argc, char *argp[])
{
	
unsigned int *table[4] = {cnt_isr, val_isr, cnt_read, val_read};
int j;
for(j=0;j<10;j++)
{
	insert(table, ins_buf);

}
}
