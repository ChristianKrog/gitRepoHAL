#include <string.h>
#include <stdio.h>

int main(){
	char str[32] = "cnt1:cnt2:cnt3:cnt4";
	const char s[2] = ":";
	char *token;

	token =strtok(str, s);

	while(token != NULL){
		printf("%s\n", token);

		token = strtok(NULL, s);
	}
	retunr(0);
}



