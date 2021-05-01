#include "template.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Max number of characters per line
#define MAX_LINE 1024

// Max operand size
#define MAX_OPER 32

// Define constants used in LLVM grammar
#define PLUS    '+'
#define MINUS   '-'
#define TIMES   '*'
#define DIVIDE  '/'
#define MOD     '%'
#define SEMI    ';'
#define SPACE   ' '
#define LPAREN  '('
#define RPAREN  ')'
#define LCURLY  '{'
#define RCURLY  '}'
#define ASSIGN  '='

#define IF      "if"
#define ELSE    "else"
#define WHILE   "while"

#define NOFILL      0
#define IFCONDITION 1
#define IFTHEN      2
#define IFELSE      3

#define WHILECONDITION  1
#define WHILEBODY       2

#define EQUALS  'e'
#define NEQUALS 'n'
#define AND     'a'
#define OR      'o'
#define LT      '<'
#define GT      '>'
#define NOT     '!'

// Define a boolean type
typedef int boolean;

// Define true and false for boolean type
const boolean TRUE = 1;
const boolean FALSE = 0;

// Creates nodes specific for stack use
typedef struct Node {
	char *data;
	struct Node *next;
} Node;

// Creates stack responsbile for simulating left recursion.
typedef struct Stack {
	Node *top;
} Stack;

// Creates a structure representing the node of what will be a SymbolTable linked list
typedef struct Symbol {
	char *variable;
	char *stackEntry;
	struct Symbol *next;
} Symbol;

// Creates SymbolTable linked list
typedef struct SymbolTable {
	Symbol *symbols;
} SymbolTable;

// Creaets a structure responsible for packaging while statement data
typedef struct IfStatement {
	char condition[MAX_LINE];
	int conditionLength;
	char ifStatement[MAX_LINE];
	int ifStatementLength;
	char elseStatement[MAX_LINE];
	int elseStatementLength;
	int fillingPart;
	int ifelsecounter;
} IfStatement;

// Creaets a structure responsible for packaging while statement data
typedef struct WhileStatement {
	char condition[MAX_LINE];
	int conditionLength;
	char bodyStatement[MAX_LINE];
	int bodyStatementLength;
  
	int fillingPart;

	int whilecounter;
} WhileStatement;

// state machine - used to read and parse the input file
typedef struct StateMachine {
	int openedCurly;
	boolean shouldProcess;
	char line[MAX_LINE];
	int lineSize;
	char word[MAX_LINE];
	int wordLength;
	boolean isIfStatement;  
	IfStatement ifStatement;
	boolean isWhileStatement;
	WhileStatement whileStatement;
	boolean isNextCharMandatory;
	char nextChar;
	int openParen;
	int openCurly;
	boolean hadCurly;

	// forward lookup array
	char forwardLookup[MAX_LINE];
	int forwardLookupLength;

} StateMachine;

// Global Variables //

int counter; // sprintf(buffer, "%%t%d", counter);

StateMachine stateMachine;

SymbolTable *symbolTable; // Declare global symbol table

//------- Stack functions declaration -------------------
Stack *createStack(); //Create an empty Stack

void destroyStack(Stack*); //Free memory used by this stack

void push(Stack*, char*); //Push item on stack 

char *pop(Stack*); //Pop item from stack

char *peek(Stack*); //Check top value on this stack

boolean isEmpty(Stack*); //Check if provided stack is empty
// ------------------------------------------------------

//------- Symbol table functions declaration -------------------
SymbolTable *createSymbolTable(); //Create an empty SymbolTable

void destroySymbolTable(SymbolTable*); //Free memory used by this symbol table

char *insertVar(SymbolTable*, char*); //Add new variable on SymbolTable and return created stack entry

boolean isVariableDeclared(SymbolTable*, char*); //Check if given variable is declared and stored in symbol table

char *getStackEntry(SymbolTable*, char*); //Return stack entry associated with given variable
// ------------------------------------------------------

//---- Aritmethic expression functions declaration---------
int precedence(char); //Operator precedence
// ------------------------------------------------------

//-----Output utility functions declarations--------------
void printHead(); // Print llvm header

void printTail(); // Print llvm tail
//--------------------------------------------------------

//-----LLVM parser utility methods declaration--------------
void parseString(char *, int);  //Parse and process given string

void statement(StateMachine*); //Process an execution block

void intDeclaration(char*, int); // Process integer declaration

void printStatement(char*, int); // Process print statement

void readStatement(char*, int); // Process read statement

void assignStatement(char*, char*, int, int); // Process assignment statement

void handleIfStatement(IfStatement*); // Process if statement

void handleWhileStatement(WhileStatement*); // Process while statement

char *expression(char*, int, int); // Calculate an arithmetic expression and returns the variablewhere result is stored

void handleArithmeticOperation(Stack*, Stack*); // Handle (eg print llvm statement) for basic arithmeticoperation
//--------------------------------------------------------

//-----State Machine utility methods declaration--------------
void initStateMachine(StateMachine*);   // Initialize state machine

void feedCharToStateMachine(StateMachine*, char ); // Feed a character to state machine

void feedCharToStateMachineProcessingIfStatement(StateMachine*, char); // State machine detected an if statement - feed char to a state machine handling if

void feedCharToStateMachineProcessingWhileStatement(StateMachine*, char); // State machine detected a while statement - feed char to a state machine handling while

boolean shouldProcess(StateMachine*); // Returns true if this statement should be printed

void newWord(StateMachine*); // Returns true is there is a new word detected

//--------------------------------------------------------

//-----Char utility methods declaration--------------
boolean isDigit(char); // Returns true is char is digit

boolean isAlpha(char); // Returns true is char is letter

boolean isAlhaNumberic(char); // Returns true is char is letter or digit

boolean isVariable(char); // Returns true is char can be starting variable name
//--------------------------------------------------------

//-----String matching method declaration---------------------------
boolean match(StateMachine*, char*);
//--------------------------------------------------------

int main(int argc, char **argv)
{
	int i;
	char c;
	FILE *ifp;

	if (argc != 2) 
	{
		printf("Usage parser <file.in>\n");
		exit(0);
	}

	// Read in and verify file.
	if ((ifp = fopen(argv[1], "r")) == NULL)
		printf("ERROR: Invalid file\n");

	printf(PROLOGUE);

	// Initialize applicationStack
	counter = 1;
	stateMachine.ifStatement.ifelsecounter = 1;
	stateMachine.whileStatement.whilecounter = 1;
	stateMachine.forwardLookupLength = 0;

	// Spawn symbol table
	symbolTable = createSymbolTable();

	initStateMachine(&stateMachine);

	// Process file
	while ((c = fgetc(ifp)) != EOF)
	{
		feedCharToStateMachine(&stateMachine, c); 
		
		if (shouldProcess(&stateMachine)) 
		{
			statement(&stateMachine);
			initStateMachine(&stateMachine);      
			
			for (i = 0; i < stateMachine.forwardLookupLength; i++)
				feedCharToStateMachine(&stateMachine, stateMachine.forwardLookup[i]);
		
			stateMachine.forwardLookupLength = 0;
		}
  	}
	statement(&stateMachine);
	printf(EPILOGUE);

	// Memory cleanup.
	destroySymbolTable(symbolTable);
	fclose(ifp);

	return 0;
}


//------- Stack functions implementation -------------------
Stack *createStack()
{
  	Stack *newStack = (Stack*)malloc(sizeof(Stack));
  	newStack->top = NULL;
  	return newStack;
}

void destroyStack(Stack *stack) 
{
  	Node *delNode, *temp = stack->top;
  	while (temp != NULL) 
	{
		delNode = temp;
		temp = temp->next;
		free(delNode->data);
		free(delNode);
	}
	free(stack);
}

boolean isEmpty(Stack *stack)
{
	return stack->top == NULL;
}

void push(Stack *stack, char *item)
{
	Node* node = (Node*)malloc(sizeof(Node));
	node->data = (char*)malloc(sizeof(char) * MAX_OPER);
	strcpy(node->data, item);
	node->next = stack->top;
	stack->top = node;
}

char *pop(Stack *stack)
{
	// If stack is empty there is an error in codding logic - throw an exception
	assert(stack->top != NULL);
	Node* node = stack->top;
	char *data = (char*)malloc(sizeof(char) * MAX_OPER);
	strcpy(data, node->data);
	stack->top = stack->top->next;
	
	// Clean up memory.
  	free(node->data);
  	free(node);
  	return data;
}

char *peek(Stack *stack)
{
	// If stack is empty there is an error in codding logic - throw an exception
	assert(stack->top != NULL);
	return stack->top->data;  
}
//------- End: Stack functions implementation -------------------

//------- Symbol table functions implementation -------------------
SymbolTable *createSymbolTable()
{
	SymbolTable *symbolTable = (SymbolTable*)malloc(sizeof(SymbolTable));
  	symbolTable->symbols = NULL;
  	return symbolTable;
}

void destroySymbolTable(SymbolTable *symbolTable)
{
  	Symbol *symbol = symbolTable->symbols;

  	while (symbol != NULL) 
  	{
		Symbol *delSymbol = symbol;
		symbol = symbol->next;
		
		// Memory cleanup.
		free(delSymbol->variable);
		free(delSymbol->stackEntry);
		free(delSymbol);
  	}

  	free(symbolTable);
}

char *insertVar(SymbolTable *symbolTable, char *variable) 
{
  	// Assume variable is not already declared - caller function should ceck if variable is declared before calling this method
  	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
  	symbol->variable = (char*)malloc(sizeof(char) * MAX_OPER);
  	strcpy(symbol->variable, variable);
  	symbol->stackEntry = (char*)malloc(sizeof(char) * MAX_OPER);
  	sprintf(symbol->stackEntry, "%%t%d", counter);
	counter++;
	symbol->next = symbolTable->symbols;
	symbolTable->symbols = symbol;

  	return symbol->stackEntry;
}

boolean isVariableDeclared(SymbolTable *symbolTable, char *variable)
{
  	Symbol *symbol = symbolTable->symbols;
	while (symbol != NULL) 
	{
		if (strcmp(symbol->variable, variable) == 0) 
			return TRUE;
		
		symbol = symbol->next;
	}

	return FALSE;
}

char *getStackEntry(SymbolTable *symbolTable, char *variable)
{
  	Symbol *symbol = symbolTable->symbols;
	while (symbol != NULL) 
	{
		if (strcmp(symbol->variable, variable) == 0) 
			return symbol->stackEntry;
		
		symbol = symbol->next;
	}

	return NULL;
}
//------- End: Symbol table functions implementation -------------------

//---- Aritmethic expression functions implementation -----------
int precedence(char c)
{
	if (c == NOT)
		return 1;
	if (c == TIMES || c == DIVIDE || c == MOD)
		return 2;
	if (c == PLUS || c == MINUS)
		return 3;
	if (c == GT || c == LT)
		return 4;
	if (c == EQUALS || c == NEQUALS)
		return 5;
	if (c == AND)
		return 6;
	if (c == OR)
		return 7;
	if (c == LPAREN)
		return 8;
}
//---- End: Aritmethic expression functions implementation -----------

//-----LLVM parser utility methods implementation--------------
void parseString(char *str, int len)
{
	StateMachine nestedStateMachine;

	initStateMachine(&nestedStateMachine);
	nestedStateMachine.ifStatement.ifelsecounter = 1;
	nestedStateMachine.forwardLookupLength = 0;

	for (int i = 0; i < len; i++)
	{
		feedCharToStateMachine(&nestedStateMachine, str[i]); 
		if ( shouldProcess(&nestedStateMachine) ) 
		{
			statement(&nestedStateMachine);
			initStateMachine(&nestedStateMachine);
			
			for (int i = 0; i < nestedStateMachine.forwardLookupLength; i++)
				feedCharToStateMachine(&nestedStateMachine, nestedStateMachine.forwardLookup[i]);   
			
			nestedStateMachine.forwardLookupLength = 0;
		}
	}
	statement(&nestedStateMachine);
}

void statement(StateMachine *stateMachine)
{
	// Verify if statement
	if ( stateMachine->isIfStatement == TRUE )
	{
		handleIfStatement( &(stateMachine->ifStatement) );
		return;
	}

	// Verify while statement
	if ( stateMachine->isWhileStatement == TRUE )
	{
		handleWhileStatement( &(stateMachine->whileStatement) );
		return;
	}

	// Verify int declaration
	if ( match(stateMachine, "int") == TRUE)
	{
		intDeclaration(stateMachine->line, stateMachine->lineSize);
	}

	// Verify print statement
	if ( match(stateMachine, "print") == TRUE)
	{
		printStatement(stateMachine->line, stateMachine->lineSize);
	}

	// Verify read statement
	if ( match(stateMachine, "read"))
	{
		readStatement(stateMachine->line, stateMachine->lineSize);
	}

	// Verify assignment
	int idx = 0;
	while (idx < stateMachine->lineSize && stateMachine->line[idx] != ASSIGN)
		idx++;
	
	if (idx > 0 && idx < stateMachine->lineSize)
	{
		// Verify variable declaration
		char *variable = (char*)malloc(sizeof(char) * MAX_OPER);
		strncpy(variable, stateMachine->line, idx);
		variable[idx] = '\0';

		if ( isVariableDeclared(symbolTable, variable) )
		{
			assignStatement(variable, stateMachine->line, idx, stateMachine->lineSize);
		} 
		else 
		{
			fprintf(stderr, "error: use of undeclared variable %s\n", variable);
			free(variable);
			exit(1);
		} 
		free(variable);
	}
}

void intDeclaration(char *buffer, int len)
{
  	char *variable = (char*)malloc(sizeof(char) * MAX_OPER);
	strncpy(variable, buffer + 3, len - 3);
	variable[len - 3] = '\0';

	// Verify variable declaration
	if ( isVariableDeclared(symbolTable, variable) == TRUE )  
	{
		fprintf(stderr, "error: multiple definitions of %s\n", variable);
		exit(1);
	}

	char *stackEntry = insertVar(symbolTable, variable);

	printf("    %s = alloca i32\n", stackEntry);

	free(variable);
}

void printStatement(char *buffer, int len)
{
  	char *result = expression(buffer, len, 4);

	if ( isVariable(result[0]) == TRUE )
	{
		// Verify variable declaration
		if ( isVariableDeclared(symbolTable, result) == FALSE )  
		{
			fprintf(stderr, "error: use of undeclared variable %s\n", result);
			exit(1);
		}
		// Load in first variable
		char *stackEntry = getStackEntry(symbolTable, result);
		sprintf(result, "%%t%d", counter);
		counter++;
		printf("    %s = load i32, i32* %s\n", result, stackEntry);
	}

	printf("    call void @print_integer(i32 %s)\n", result);
	free(result);
}

void readStatement(char *buffer, int len)
{
  	char *variable = (char*)malloc(sizeof(char) * MAX_OPER);
	strncpy(variable, buffer + 4, len - 4);
	variable[len - 4] = '\0';

	// Verify variable declaration
	if ( isVariableDeclared(symbolTable, variable) == FALSE )  
	{
		fprintf(stderr, "error: use of undeclared variable %s\n", variable);
		exit(1);
	}

	char *stackEntry = getStackEntry(symbolTable, variable);

	printf("    %%t%d = call i32 @read_integer()\n", counter);
	printf("    store i32 %%t%d, i32* %s\n", counter, stackEntry);
	counter++;

	free(variable);
}

void assignStatement(char *variable, char *buffer, int start, int len)
{
  	char *stackEntry = getStackEntry(symbolTable, variable);  
	char *result = expression(buffer, len, start);

	if ( isVariable(result[0]) == TRUE )
	{
		// Verify variable declaration
		if ( isVariableDeclared(symbolTable, result) == FALSE )  
		{
			fprintf(stderr, "error: use of undeclared variable %s\n", result);
			exit(1);
		}
		
		// Load in first variable
		char *tempEntry = getStackEntry(symbolTable, result);
		sprintf(result, "%%t%d", counter);
		counter++;
		printf("    %s = load i32, i32* %s\n", result, tempEntry); 
	}

	printf("    store i32 %s, i32* %s\n", result, stackEntry);
}

void handleIfStatement(IfStatement* ifStatement)
{
	ifStatement->condition[ifStatement->conditionLength] = '\0';
	ifStatement->ifStatement[ifStatement->ifStatementLength] = '\0';
	ifStatement->elseStatement[ifStatement->elseStatementLength] = '\0';
	// printf("|%s| -- |%s| -- |%s|\n", ifStatement->condition, ifStatement->ifStatement, ifStatement->elseStatement);
	char *result = expression(ifStatement->condition, ifStatement->conditionLength, 0);
	printf("    br i1 %s, label %%ifbranch_%p_%d, label %%elsebranch_%p_%d\n", result, ifStatement, ifStatement->ifelsecounter, ifStatement, ifStatement->ifelsecounter);
	printf("ifbranch_%p_%d:\n", ifStatement, ifStatement->ifelsecounter);
	
	parseString(ifStatement->ifStatement, ifStatement->ifStatementLength);
	printf("    br label %%end_%p_%d\n", ifStatement, ifStatement->ifelsecounter);
	printf("elsebranch_%p_%d:\n", ifStatement, ifStatement->ifelsecounter);  
	
	if ( ifStatement->elseStatementLength > 0 )
	{
		parseString(ifStatement->elseStatement, ifStatement->elseStatementLength);    
	}
	
	printf("    br label %%end_%p_%d\n", ifStatement, ifStatement->ifelsecounter);
	printf("end_%p_%d:\n", ifStatement, ifStatement->ifelsecounter);  
	ifStatement->ifelsecounter++;
}

void handleWhileStatement(WhileStatement* whileStatement)
{
	whileStatement->condition[whileStatement->conditionLength] = '\0';
	whileStatement->bodyStatement[whileStatement->bodyStatementLength] = '\0';
	//  printf("while |%s| -- |%s|\n", whileStatement->condition, whileStatement->bodyStatement);
  
	printf("    br label %%whilehead_%p_%d\n", whileStatement, whileStatement->whilecounter);
	printf("whilehead_%p_%d:\n", whileStatement, whileStatement->whilecounter);
	char *result = expression(whileStatement->condition, whileStatement->conditionLength, 0);
	printf("    br i1 %s, label %%whilebody_%p_%d, label %%whileexit_%p_%d\n", result, whileStatement, whileStatement->whilecounter, whileStatement, whileStatement->whilecounter);
	printf("whilebody_%p_%d:\n", whileStatement, whileStatement->whilecounter);

	parseString(whileStatement->bodyStatement, whileStatement->bodyStatementLength);

	printf("  br label %%whilehead_%p_%d\n", whileStatement, whileStatement->whilecounter);
	printf("whileexit_%p_%d:\n", whileStatement, whileStatement->whilecounter);
	whileStatement->whilecounter++;
}

char *expression(char *buffer, int len, int start) 
{
	// evaluate expression
	// virtually transform given expression from infix to postfix - printing llvm statements
	char *token, *result, *operand;
	Stack *operands = createStack();
	Stack *operators = createStack();

	int pred = start;
	for (int i = pred+1; i < len; i++) 
	{
		if (buffer[i] == LPAREN) 
		{
			push(operators, "(");
			pred = i;
		}
		if (buffer[i] == RPAREN)
		{
			token = (char*)malloc(sizeof(char) * (i - pred));
			strncpy(token, buffer + pred + 1, i - pred - 1);
			token[i - pred - 1] = '\0';

		if (strlen(token) >= 1)
		{
			push(operands, token);
		}
		// pop from stack until we find LPAREN
		while (( isEmpty(operators) == FALSE ) && peek(operators)[0] != LPAREN)
		{
			handleArithmeticOperation(operands, operators);
		}
		if (( isEmpty(operators) == FALSE ) && peek(operators)[0] == LPAREN)
		{
			pop(operators);
		}
		pred = i;
		free(token);
		}

		if (buffer[i] == PLUS || buffer[i] == TIMES || buffer[i] == DIVIDE || buffer[i] == MOD || buffer[i] == NOT ||
			buffer[i] == GT || buffer[i] == LT || buffer[i] == ASSIGN || buffer[i] == NOT || buffer[i] == AND || buffer[i] == OR ||
			(buffer[i] == MINUS && (i > start + 1 && (isAlhaNumberic(buffer[i-1]) == TRUE || buffer[i-1] == RPAREN)))) 
		{
			token = (char*)malloc(sizeof(char) * (i - pred));
			strncpy(token, buffer+pred + 1, i - pred - 1);
			token[i - pred - 1] = '\0';

			if (strlen(token) > 0)
			{
				push(operands, token);
			}

			if (buffer[i] == '=' && i + 1 < len && buffer[i+1] == '=') 
			{
				i++;
				buffer[i] = EQUALS;
			}
			if (buffer[i] == '!' && i + 1 < len && buffer[i+1] == '=') 
			{
				i++;
				buffer[i] = NEQUALS;
			}
			if (buffer[i] == '&' && i + 1 < len && buffer[i+1] == '&') 
			{
				i++;
				buffer[i] = AND;
			}
			if (buffer[i] == '|' && i + 1 < len && buffer[i+1] == '|') 
			{
				i++;
				buffer[i] = OR;
			}

			while (( isEmpty(operators) == FALSE ) && precedence(peek(operators)[0]) <= precedence(buffer[i]))
			{
				handleArithmeticOperation(operands, operators);  
			}
			operand = (char*)malloc(sizeof(char) * 2);
			operand[0] = buffer[i];
			operand[1] = '\0';
			token[i - pred - 2] = '\0';
			push(operators, operand);
			pred = i;
			free(token);  
		}
	}

	token = (char*)malloc(sizeof(char)* (len - pred));
	strncpy(token, buffer+pred+1, len - pred-1);
	token[len - pred - 1] = '\0';
	
	if ((strlen(token) > 1) || (strlen(token) == 1 && isAlhaNumberic(token[0]) == TRUE))   // this is an operand
	{
		push(operands, token);
	}
	else
	{
		if (token[0] == RPAREN) 
		{
			// pop from stack until we find LPAREN
			while ((isEmpty(operators) == FALSE) && peek(operators)[0] != LPAREN)
			{
				handleArithmeticOperation(operands, operators);
			}
			if ((isEmpty(operators) == FALSE) && peek(operators)[0] == LPAREN)
			{
				pop(operators);
			}
		}
	}

	while (operands->top->next != NULL) 
	{
		handleArithmeticOperation(operands, operators);
	}

	result = (char*)malloc(sizeof(char) * MAX_OPER);
	strcpy(result, operands->top->data);
	
	destroyStack(operands);
	destroyStack(operators);  

	return result;
}
//-----End: LLVM parser utility methods implementation--------------

void handleArithmeticOperation(Stack *operands, Stack *operators)
{
	char operator;
  	char operand1[MAX_OPER];
	char operand2[MAX_OPER];

	operator = pop(operators)[0];
	strcpy(operand2, pop(operands));

	if (operator != NOT)
	{
		strcpy(operand1, pop(operands));
	}
	
	// ceck if any of the oprator is a variable
	if (isVariable(operand1[0]) == TRUE && operator != NOT)
	{
		// is the var declared?
		if (isVariableDeclared(symbolTable, operand1) == FALSE)  
		{
			fprintf(stderr, "error: use of undeclared variable %s\n", operand1);
			exit(1);
		}
		// first load variable value
		char *stackEntry = getStackEntry(symbolTable, operand1);
		sprintf(operand1, "%%t%d", counter);
		counter++;
		printf("    %s = load i32, i32* %s\n", operand1, stackEntry);
	}

	if (isVariable(operand2[0]) == TRUE)
	{
		// is the var declared?
		if (isVariableDeclared(symbolTable, operand2) == FALSE)  
		{
			fprintf(stderr, "error: use of undeclared variable %s\n", operand2);
			exit(1);
		}
		// first load variable value
		char *stackEntry = getStackEntry(symbolTable, operand2);
		sprintf(operand2, "%%t%d", counter);
		counter++;
		printf("    %s = load i32, i32* %s\n", operand2, stackEntry); 
	}

	char buffer[MAX_OPER];
	sprintf(buffer, "%%t%d", counter);
	counter++;
	printf("    %s = ", buffer);
	
	switch (operator)
	{
		case PLUS:
		printf("add nsw i32 %s, %s\n", operand1, operand2);
		break;
		
		case MINUS:
		printf("sub nsw i32 %s, %s\n", operand1, operand2);
		break;

		case TIMES:
		printf("mul nsw i32 %s, %s\n", operand1, operand2);
		break;

		case DIVIDE:
		printf("sdiv i32 %s, %s\n", operand1, operand2);
		break;

		case MOD:
		printf("srem i32 %s, %s\n", operand1, operand2);
		break;

		case GT:
		printf("icmp sgt i32 %s, %s\n", operand1, operand2);
		break;  

		case LT:
		printf("icmp slt i32 %s, %s\n", operand1, operand2);
		break;  

		case EQUALS:
		printf("icmp eq i32 %s, %s\n", operand1, operand2);
		break;

		case NEQUALS:
		printf("icmp ne i32 %s, %s\n", operand1, operand2);
		break;

		case AND:
		printf("and i1 %s, %s\n", operand1, operand2);
		break;  

		case OR:
		printf("or i1 %s, %s\n", operand1, operand2);
		break;

		case NOT:
		printf("xor i1 %s, 1\n", operand2);      
		break;
	}

	// Push new operand onto stack
	push(operands, buffer);
}

//-----State Machine utility methods implementation--------------
void initStateMachine(StateMachine *stateMachine)
{
  	stateMachine->openedCurly = 0;
	stateMachine->shouldProcess = FALSE;
	stateMachine->lineSize = 0;
	stateMachine->wordLength = 0;
	stateMachine->isIfStatement = FALSE;
	stateMachine->isWhileStatement = FALSE;
	
	stateMachine->ifStatement.conditionLength = 0;
	stateMachine->ifStatement.ifStatementLength = 0;
	stateMachine->ifStatement.elseStatementLength = 0;
	stateMachine->ifStatement.fillingPart = IFCONDITION;

	stateMachine->whileStatement.conditionLength = 0;
	stateMachine->whileStatement.bodyStatementLength = 0;
	stateMachine->whileStatement.fillingPart = WHILECONDITION;

	stateMachine->isNextCharMandatory = FALSE;
	stateMachine->openParen = 0;
	stateMachine->openCurly = 0;
	stateMachine->hadCurly = FALSE;
}

void feedCharToStateMachine(StateMachine *stateMachine, char c)
{
	if (c == '\n' || c == '\t' || c == SPACE) // Ignore newline inside statement
	{
		newWord(stateMachine);
	}
	else 
	{
		if ( stateMachine->isNextCharMandatory == TRUE && stateMachine->nextChar != c)
		{
		fprintf(stderr, "error: |%c| character expected but |%c| found instead\n", stateMachine->nextChar, c);
		}
		stateMachine->isNextCharMandatory = FALSE;

		if (c == SEMI) 
		{
		newWord(stateMachine);
		stateMachine->line[stateMachine->lineSize] = '\n';
		if ( stateMachine->isIfStatement == FALSE && stateMachine->isWhileStatement == FALSE )
		{
			stateMachine->shouldProcess = TRUE;  
		}
		}
		else 
		{
		if (isAlpha(c)) 
		{
			stateMachine->word[stateMachine->wordLength++] = c;
		}
		else 
		{
			newWord(stateMachine);
			if ( stateMachine->isNextCharMandatory == TRUE && stateMachine->nextChar != c)
			{
			fprintf(stderr, "error: |%c| character expected but |%c| found instead\n", stateMachine->nextChar, c);
			}
			stateMachine->isNextCharMandatory = FALSE;
		}
		stateMachine->line[stateMachine->lineSize++] = c;
		}
	}

	if (stateMachine->isIfStatement == TRUE)
	{
		feedCharToStateMachineProcessingIfStatement(stateMachine, c);
	}

	if (stateMachine->isWhileStatement == TRUE)
	{
		feedCharToStateMachineProcessingWhileStatement(stateMachine, c);
	}
}

void feedCharToStateMachineProcessingIfStatement(StateMachine *stateMachine, char c)
{
	// Ignore newline inside statement
  	if (c == '\n' || c == '\t' || c == SPACE)
	{
		switch(stateMachine->ifStatement.fillingPart)
		{
		case IFTHEN: 
			stateMachine->ifStatement.ifStatement[stateMachine->ifStatement.ifStatementLength++] = c;
			break;
		case IFELSE:
			stateMachine->ifStatement.elseStatement[stateMachine->ifStatement.elseStatementLength++] = c;
			break;
		}
	}
	else 
	{
		if (stateMachine->isNextCharMandatory == TRUE && stateMachine->nextChar != c)
		{
			fprintf(stderr, "error: |%c| character expected but |%c| found instead\n", stateMachine->nextChar, c);
		}
		stateMachine->isNextCharMandatory = FALSE;

		if (c == SEMI) 
		{
			if ( stateMachine->openCurly == 0)
			{
				if ( stateMachine->ifStatement.fillingPart == IFTHEN )
				{
					stateMachine->ifStatement.fillingPart = NOFILL;
					stateMachine->hadCurly = FALSE;
				}

				if (stateMachine->ifStatement.fillingPart == IFELSE )
				{
					stateMachine->shouldProcess = TRUE;      
				}
			}
			else
			{
				switch(stateMachine->ifStatement.fillingPart)
				{
				case IFTHEN: 
					stateMachine->ifStatement.ifStatement[stateMachine->ifStatement.ifStatementLength++] = c;
					break;
				case IFELSE:
					stateMachine->ifStatement.elseStatement[stateMachine->ifStatement.elseStatementLength++] = c;
					break;
				}
			}
		} 
		else 
		{
			if (isAlpha(c)) 
			{
				if (stateMachine->ifStatement.fillingPart == NOFILL)
					stateMachine->forwardLookup[stateMachine->forwardLookupLength++] = c;
			}
			else 
			{
				if ( stateMachine->isNextCharMandatory == TRUE && stateMachine->nextChar != c)
					fprintf(stderr, "error: |%c| character expected but |%c| found instead\n", stateMachine->nextChar, c);
				
				stateMachine->isNextCharMandatory = FALSE;
			}
			
			switch (stateMachine->ifStatement.fillingPart)
			{
				case IFCONDITION: 
				stateMachine->ifStatement.condition[stateMachine->ifStatement.conditionLength++] = c;
				if (c == LPAREN)
				{
					stateMachine->openParen++;
				}
				if (c == RPAREN)
				{
					stateMachine->openParen--;
				}
				if (stateMachine->openParen == 0)
				{
					stateMachine->ifStatement.fillingPart = IFTHEN;     
				}
				break;
				case IFTHEN: 
				
				if (c != LCURLY && c != RCURLY)
				{
					stateMachine->ifStatement.ifStatement[stateMachine->ifStatement.ifStatementLength++] = c;
				}
				if (c == LCURLY)
				{
					stateMachine->openCurly++;
					stateMachine->hadCurly = TRUE;
				}
				if (c == RCURLY)
				{
					stateMachine->openCurly--;
					stateMachine->hadCurly = TRUE;
				}
				if (stateMachine->openCurly == 0 && stateMachine->hadCurly == TRUE)
				{
					stateMachine->ifStatement.fillingPart = NOFILL;     
					stateMachine->hadCurly = FALSE;
				}
				break;  
				case IFELSE: 
				if (c != LCURLY && c != RCURLY)
				{
					stateMachine->ifStatement.elseStatement[stateMachine->ifStatement.elseStatementLength++] = c;
				}
				if (c == LCURLY)
				{
					stateMachine->openCurly++;
					stateMachine->hadCurly = TRUE;
				}
				if (c == RCURLY)
				{
					stateMachine->openCurly--;
					stateMachine->hadCurly = TRUE;
				}
				if (stateMachine->openCurly == 0 && stateMachine->hadCurly == TRUE)
				{
					stateMachine->shouldProcess = TRUE;     
				}
				break;  
			}
		}
	}
}

void feedCharToStateMachineProcessingWhileStatement(StateMachine *stateMachine, char c)
{
  	if (c == '\n' || c == '\t' || c == SPACE) // Ignore newline inside statement
	{
		switch(stateMachine->whileStatement.fillingPart)
		{
		case WHILEBODY: 
			stateMachine->whileStatement.bodyStatement[stateMachine->whileStatement.bodyStatementLength++] = c;
			break;
		}
	}
	else 
	{
		if (c == SEMI) 
		{
		if ( stateMachine->openCurly == 0)
		{
			if (stateMachine->whileStatement.fillingPart == WHILEBODY )
			{
			stateMachine->shouldProcess = TRUE;      
			}
		}
		else
		{
			switch(stateMachine->whileStatement.fillingPart)
			{
			case WHILEBODY: 
				stateMachine->whileStatement.bodyStatement[stateMachine->whileStatement.bodyStatementLength++] = c;
				break;
			}  
		}
		}
		else 
		{
		switch(stateMachine->whileStatement.fillingPart)
		{
			case WHILECONDITION: 
			stateMachine->whileStatement.condition[stateMachine->whileStatement.conditionLength++] = c;
			if ( c == LPAREN )
			{
				stateMachine->openParen++;
			}
			if ( c == RPAREN )
			{
				stateMachine->openParen--;
			}
		
			if (stateMachine->openParen == 0)
			{
				stateMachine->whileStatement.fillingPart = WHILEBODY;     
			}
			break;
			case WHILEBODY: 
			if ( c != LCURLY && c != RCURLY )
			{
				stateMachine->whileStatement.bodyStatement[stateMachine->whileStatement.bodyStatementLength++] = c;
			}
			if ( c == LCURLY )
			{
				stateMachine->openCurly++;
				stateMachine->hadCurly = TRUE;
			}
			if ( c == RCURLY )
			{
				stateMachine->openCurly--;
				stateMachine->hadCurly = TRUE;
			}
			if (stateMachine->openCurly == 0 && stateMachine->hadCurly == TRUE)
			{
				stateMachine->whileStatement.fillingPart = NOFILL;     
				stateMachine->hadCurly = FALSE;
				stateMachine->shouldProcess = TRUE;
			}
			break;  
		}
		}
	}  
}

boolean shouldProcess(StateMachine *stateMachine)
{
	return stateMachine->shouldProcess;
}

void newWord(StateMachine *stateMachine)
{
	stateMachine->word[stateMachine->wordLength] = '\0';
	if (stateMachine->wordLength == 0)
	{
		return; 
	}
	//printf("word: |%s|\n", stateMachine->word);

	if ( strcmp(stateMachine->word, ELSE) != 0 && stateMachine->isIfStatement == TRUE && stateMachine->ifStatement.fillingPart == NOFILL)
	{
		stateMachine->shouldProcess = TRUE;
		return;
	}

	if ( strcmp(stateMachine->word, IF) == 0 && stateMachine->isWhileStatement == FALSE && stateMachine->isIfStatement == FALSE )
	{
		stateMachine->isIfStatement = TRUE;
		// next char fed is mandatory to be LPAREN
		stateMachine->isNextCharMandatory = TRUE;
		stateMachine->nextChar = LPAREN;

		// initialize ifStatement
		stateMachine->ifStatement.conditionLength = 0;
		stateMachine->ifStatement.ifStatementLength = 0;
		stateMachine->ifStatement.elseStatementLength = 0;
		stateMachine->ifStatement.fillingPart = IFCONDITION;
		stateMachine->openParen = 0;
		stateMachine->openCurly = 0;
	}

	if ( strcmp(stateMachine->word, ELSE) == 0 && stateMachine->isIfStatement == TRUE )
	{
		stateMachine->ifStatement.fillingPart = IFELSE;
		stateMachine->forwardLookupLength = 0;
	}

	if ( strcmp(stateMachine->word, WHILE) == 0 && stateMachine->isIfStatement == FALSE && stateMachine->isWhileStatement == FALSE )
	{
		stateMachine->isWhileStatement = TRUE;
		// next char fed is mandatory to be LPAREN
		stateMachine->isNextCharMandatory = TRUE;
		stateMachine->nextChar = LPAREN;

		// initialize ifStatement
		stateMachine->whileStatement.conditionLength = 0;
		stateMachine->whileStatement.bodyStatementLength = 0;
		stateMachine->whileStatement.fillingPart = WHILECONDITION;
		stateMachine->openParen = 0;
		stateMachine->openCurly = 0;
	}

	if (stateMachine->wordLength > 0)
	{
	//  printf("word: %s\n", stateMachine->word);
	}
	stateMachine->wordLength = 0;
}
//-----End: State Machine utility methods implementation--------------

//-----Char utility methods implementation--------------
boolean isDigit(char c)
{
	return (c >= '0' && c <= '9');
}

boolean isAlpha(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

boolean isAlhaNumberic(char c) 
{
	return isDigit(c) || isAlpha(c);
}

boolean isVariable(char c)
{
	return (c != '%' && !(c >= '0' && c <= '9') && c != MINUS);
}
//-----End: Char utility methods implementation--------------

//-----String matching method implementation---------------------------
boolean match(StateMachine *stateMachine, char *pattern)
{
	boolean m = TRUE;
  	int len = strlen(pattern);
	if (stateMachine->lineSize < len)
	{
		m = FALSE;
	}
	else
	{
		for (int i = 0; i < len; i++) 
		{
			if (stateMachine->line[i] != pattern[i])
			{
				m = FALSE;
			}
		}  
	}

	return m;
}
//-----End: String matching method implementation---------------------------
