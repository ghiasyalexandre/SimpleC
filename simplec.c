#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX 4096

int isSpace(char ch)
{
	if (ch == ' ' || ch == ';' || ch == '\n') return 1;
	return 0;
}

int isDelimiter(char ch)
{
	if (ch == ' ' || ch == '+' || ch == '-' ||
		ch == '*' || ch == '/' || ch == ';' ||
		ch == '%') return 1;
	return 0;
}

char *isOperator(char ch)
{
	char *buffer = NULL;
	if (ch == '+'){
		buffer = malloc(sizeof(char) * 8);
		strcpy(buffer, "add nsw");
	} else if (ch == '-'){
		buffer = malloc(sizeof(char) * 8);
		strcpy(buffer, "sub nsw");
	} else if (ch == '*'){
		buffer = malloc(sizeof(char) * 8);
		strcpy(buffer, "mul nsw");		
	} else if (ch == '/'){
		buffer = malloc(sizeof(char) * 5);
		strcpy(buffer, "sdiv");		
	} else if (ch == '%'){
		buffer = malloc(sizeof(char) * 5);
		strcpy(buffer, "srem");	
	} else if (ch == ';'){
		buffer = malloc(sizeof(char) * 5);
		strcpy(buffer, "semi");
	} else{
		printf("INVALID operator!\n");
		return NULL;
	}
	return buffer;
}

int isIdentifier(char *str)
{
	if (!strcmp(str, "print"))
		return 1;
	return 0;
}

void printLLVM(int a, int b, int *count, char *operator)
{
	if (b == INT_MIN)
	{
		printf("\tcall void @print_intger(i32 %%t%d\n", a);
		return;
	}
	printf("\t%%t%d = %s i32 %d, %d\n", *count, operator, a, b);
	printf("\tcall void @print_intger(i32 %%t%d\n", *count);
	(*count)++;
}

void readFile(FILE *ifp)
{
	char ch, *buffer = NULL, *operator = NULL;
	int a = 0, b = 0, flag = 0, *count = 1, i = 0;

	while ((ch = fgetc(ifp)) != EOF)
	{
		if (isSpace(ch))
			continue;
		else if (ch == 'p')
		{
			buffer = calloc(MAX, sizeof(char));
			operator = calloc(10, sizeof(char));

			for (i = 0; isDelimiter(ch) == 0; i++)
			{
				buffer[i] = ch;
				ch = fgetc(ifp);
			}

			buffer[i] = '\0';
			ch = fgetc(ifp)
			while(isSpace(ch)) ch = fgetc(ifp);

			if (isIdentifier(buffer))
			{
				if (isDigit(ch))
				{
					a = atoi(ch);
					ch = fgetc(ifp);
					while(isSpace(ch)){
						ch = fgetc(ifp);
					}
					strcpy(operator, isOperator(ch));

					if (operator == NULL)
						break;
					else if (!strcmp(operator, "semi"))
						printLLVM(a,INT_MIN,NULL,NULL);
					else
					{
						ch = fgetc(ifp);
						while(isSpace(ch)){
							ch = fgetc(ifp);
						}

						if (isDigit(ch))
						{
							b = atoi(ch);
							printLLVM(a,b,count,operator);
						}
					}

				}
			}
			else
				printf("INCORRECT Identifier\n");

			free(buffer);
			free(operator);
			a = b = i = flag = 0;
			*count = 1;
		}
		else
			printf("INCORRECT Identifier\n");
	}
}

int main(int argc, char **argv)
{
	int i = 0;
	FILE *ifp;

	if (argv == NULL) return 0;
	if ((ifp = fopen(argv[1], "r")) == NULL) return 0;

	printf("define void @print_intger(i32) #0 {\n");
	printf("\n");
	
	readFile(ifp);
	
	printf("\tret i32 0\n");
	printf("}\n");
	return 0;
}
