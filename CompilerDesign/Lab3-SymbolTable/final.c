#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 200

int keycount= 5;

char *keywords[5] = {"main", "int", "char", "return", "func"};
int is_keyword(char value[16]) 
{
	for (int i = 0; i < keycount; ++i) 
	{
		if (strcmp(value, keywords[i]) == 0)
			return 1;
	}
	return 0;
}

typedef struct symbtble 
{
	int id;
	char lexeme[16];
	char type[16];
	int size;
	int scope;
	char arg[16];
	int noarg;
	char returntype[16];
} symb_t;

symb_t symbol_table[16];
int symb_p = 0;

int isDigit (char ch) 
{
	return (ch >= '0' && ch <= '9');
}

int isNumerical (char ch) 
{
	return (isDigit(ch) || ch == '.' || ch == 'e' || ch == 'E');
}

int isAlphabet (char ch) 
{
	return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}

int isValidID (char ch) 
{
	return (isDigit(ch) || isAlphabet(ch) || ch == '_');
}

const char *SPECIALSYMBOLS = "[]{}(),;:.#";

int isSpecialSymbol (char ch) 
{
	return (strchr(SPECIALSYMBOLS, ch) != NULL);
}

const char *OPERATORS = "+-*/%<>=!&|^~";

int isOperator (char ch) 
{
	return (strchr(OPERATORS, ch) != NULL);
}

int get_size(char value[16]) 
{
	if (strcmp(value, "int") == 0)
		return 4;
	else if (strcmp(value, "char") == 0)
		return 1;
	return 0;
}

symb_t find_or_insert_symb(char lexeme[16], char type[16], int size, int scope, char arg[16], int noarg, char returntype[16]) 
{
	
	symb_t entry;
	int i;
	for (i = 0; i < symb_p; ++i) 
	{
		if (strcmp(symbol_table[i].lexeme, lexeme) == 0) 
		{
			return symbol_table[i];
		}
	}
	
	strcpy(entry.lexeme, lexeme);
	entry.id = symb_p + 1;
	strcpy(entry.type, type);
	entry.size = size;
	entry.scope = scope;
	strcpy(entry.arg,arg);
	strcpy(entry.returntype,returntype);
	entry.noarg = noarg;
	symbol_table[symb_p++] = entry;
	return entry;

}

void print_symbol_table() 
{
	printf("\n");
	printf("-----------    SYMBOL  TABLE    ----------\n");
	printf("id |            name | type | size | scope|arguments|No. of arg|returntype\n");
	for (int i = 0; i < symb_p; ++i) 
	{
		symb_t entry = symbol_table[i];
		printf("%2d | %15s | %4s | %4d | %5d| %5s   |    %2d    |%4s \n", entry.id, entry.lexeme, entry.type, entry.size, entry.scope,entry.arg,entry.noarg,entry.returntype);
	}
}


typedef enum ttx 
{
	TOKEN_TYPE_ID,
	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_NUM,
	TOKEN_TYPE_OP,
	TOKEN_TYPE_SPECIAL,
	TOKEN_TYPE_FUN
} token_type;

typedef struct tokenx 
{
	token_type type;
	char value[16];
	symb_t entry;
} token_t;

token_t tokens[120];
int token_p = 0;

token_t last_keyword;
int current_scope = 0;

token_t new_token(token_type type, char value[16]) 
{
	token_t token;
	token.type = type;
	if (type == TOKEN_TYPE_ID) 
	{
		if (is_keyword(value)) 
		{
			token.type = TOKEN_TYPE_KEYWORD;
		} 
		else 
		{
			token.entry = find_or_insert_symb(value, last_keyword.value, get_size(last_keyword.value), current_scope,"x",0,"");	
		}
	}
	else if (type == TOKEN_TYPE_FUN) 
	{
			token.entry = find_or_insert_symb(value, "Func", 0, current_scope,"x",1,last_keyword.value);			
	}
	
	strcpy(token.value, value);
	
	if (token.type == TOKEN_TYPE_KEYWORD) 
	{
		last_keyword = token;
	}
	
	tokens[token_p++] = token;
	return token;
}

void print_token(token_t token) 
{
	printf("<");
	if (token.type == TOKEN_TYPE_ID) 
	{
		symb_t entry = find_or_insert_symb(token.value, last_keyword.value, get_size(last_keyword.value), current_scope,"x",0,last_keyword.value);
		printf("id, %d", entry.id);
	
	}
	else if (token.type == TOKEN_TYPE_NUM) 
    {
		printf("num, %s", token.value);
	} 
	else 
	{
		printf("%s", token.value);
	}
	printf("> ");
}

void print_tokens() 
{
	printf("\n");
	printf("--------------    TOKENS    --------------\n");
	for (int i = 0; i < token_p; ++i) 
	{
		print_token(tokens[i]);
		if (i % 5 == 4)
			printf("\n");
	}
	printf("\n------------------------------------------\n");
}

void tokenfun(char *inputfile) 
{
	int isFunct = 0;
	FILE *input = fopen(inputfile, "r");
	char buffer[16];
	int k = 0;
	current_scope = 0;
	char ch, lch;
	token_t token;
	do 
	{
		ch = getc(input);
		if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') 
		{
		
		}
		else if (isAlphabet(ch)) 
		{
			k = 0;
			while (isValidID(ch)) 
			{
				buffer[k++] = ch;
				ch = getc(input);
			}
			buffer[k++] = '\0';
			if (ch=='(')
			{	
				token = new_token(TOKEN_TYPE_FUN, buffer);
			}
			else
				token = new_token(TOKEN_TYPE_ID, buffer);
			ungetc(ch, input);
		}

		else if (isDigit(ch)) 
		{
			k = 0;
			while (isNumerical(ch)) 
			{
				buffer[k++] = ch;
				ch = getc(input);
			}
			ungetc(ch, input);
			buffer[k++] = '\0';
			token = new_token(TOKEN_TYPE_NUM, buffer);
		}

		else if (isSpecialSymbol(ch)) 
		{
			if (ch == '{') 
			{
				current_scope += 1;
			} 
			else if (ch == '}') 
			{
				current_scope -= 1;
			}
			k = 0;
			buffer[k++] = ch;
			buffer[k++] = '\0';
			token = new_token(TOKEN_TYPE_SPECIAL, buffer);
		}

		else if (isOperator(ch)) 
		{
			k = 0;
			while (isOperator(ch) || ch == ' ' || ch == '\t') 
			{
				if (isOperator(ch)) 
				{
					buffer[k++] = ch;
				}
				ch = getc(input);
			}
			buffer[k++] = '\0';
			ungetc(ch, input);
			token = new_token(TOKEN_TYPE_OP, buffer);
		}

		lch = ch;

	} while (ch != EOF);
	print_symbol_table();
	print_tokens();	
}

int main(int argc, const char *argv[]) 
{
	tokenfun("input1.c");
	return 0;
}
