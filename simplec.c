#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1024

int isKeyword(char *str)
{
	if (!strcmp(str, "print"))
		return 1;
	return 0;
}

int isDelimiter(char ch)
{
	if (ch == ' ' || ch == '+' || ch == '-' ||
		ch == '*' || ch == '/' || ch == '%' ||
		ch == ';') return 1;
	return 0;
}

int isOperator(char ch)
{
	if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%')
		return 1;
	return 0;
}

void readFile(FILE *ifp)
{
	int i = 0;
	char ch, *buffer;

	while((ch = fgetc(ifp)) != EOF)
	{
		if (isDelimiter(ch))
			continue;
		
	}
}

void printFormat(int i, int val1, int val2)
{
	printf("\n");
}

int main(int argc, char **argv)
{
	FILE *ifp, *ofp;
	if (argv[1] == NULL) return 0;
	if ((ifp = fopen(argv[1], "r")) == NULL) return 0;
	if ((ofp = fopen("program.ll", "w")) == NULL) return 0;

	printf("define i32 @main() #0 {\n");

	printf("\t ret #0\n");
	printf("}\n");
	readFile(ifp);
	return 0;
}