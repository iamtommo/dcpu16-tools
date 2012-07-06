#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

const char *token_delimeters = " ,\t\n";
char input[] = "SET A, 0x69\nSET B, 0x01\nSET [0x5000+B], B";
char* token = NULL;//current token
uint16_t PC = 0;//program counter
uint16_t assembled[65535];//the assembled result
int total_instructions = 0;

void die(char* reason) {
	printf("Exiting! Reason: %s\n", reason);
	exit(1);
}

void next_token() {
	if (token == NULL) {
		token = strtok(input, token_delimeters);
	} else {
  		token = strtok(NULL, token_delimeters);
  	}
  	if (token != NULL) {
	  	int i;
	  	for (i = 0; i < strlen(token); i++) {
	  		token[i] = tolower(token[i]);
	  	}
	}
}

int is_string(char* str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (!isalpha(str[i])) {
			return 0;
		}
	}

	return 1;
}

int is_number(char* str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i]) && str[i] != 'x') {
			return 0;
		}
	}

	return 1;
}

int hexstr_to_int(char* str) {
	char *c = str;
	int i;
	for (i = 0; i < strlen(c); i++) {
		if (c[i] != NULL && c[i] == 'x') {
			c[i] = '0';
		}
	}

	return atoi(c);
}

char* remove_brackets(char* src) {
	char *c = (char*) malloc(strlen(src) - 2);
	strncpy(c, src+1, strlen(src)-2);
	return c;
}

void tolowercase(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		str[i] = tolower(str[i]);
	}
}

int assemble_operand() {
	next_token();

	if (strcmp(token, "a") == 0) {
		return 0x00;
	} else if (strcmp(token, "b") == 0) {
		return 0x01;
	} else if (strcmp(token, "c") == 0) {
		return 0x02;
	} else if (strcmp(token, "x") == 0) {
		return 0x03;
	} else if (strcmp(token, "y") == 0) {
		return 0x04;
	} else if (strcmp(token, "z") == 0) {
		return 0x05;
	} else if (strcmp(token, "i") == 0) {
		return 0x06;
	} else if (strcmp(token, "j") == 0) {
		return 0x07;
	} else if (strcmp(token, "pop") == 0) {
		return 0x18;
	} else if (strcmp(token, "peek") == 0) {
		return 0x19;
	} else if (strcmp(token, "push") == 0) {
		return 0x1a;
	} else if (strcmp(token, "sp") == 0) {
		return 0x1b;
	} else if (strcmp(token, "pc") == 0) {
		return 0x1c;
	} else if (strcmp(token, "o") == 0) {
		return 0x1d;
	} else {
		if (is_string(token)) {
			assembled[PC] = 0;
			//TODO: Strings/labels
			return 0x1f;
		} else if (is_number(token)) {
			int n = hexstr_to_int(token);
			if (n < 0x20) return n + 0x20;
			assembled[PC++] = n;
			return 0x1f;
		} else {
			//must be a bracketed value
			if (token[0] != '[') {
				die(strcat("Expected bracketed value, instead got ", token));
			}
			printf("Analyzing bracketed token: %s\n", token);

			//remove the brackets from the token
			char *token2 = remove_brackets(token);
			printf("%s > %s\n", token, token2);
			//split it into subtokens
			char* saveptr;
			char* subtoken = strtok_r(token2, "+", &saveptr);
			tolowercase(subtoken);
			//TODO: Labels inside bracketed tokens
			if (is_string(subtoken)) {
				printf("%s is string\n", subtoken);
				uint16_t val = 0;
				if (strcmp(subtoken, "a") == 0) {
					val = 0x08;
				} else if (strcmp(subtoken, "b") == 0) {
					val = 0x09;
				} else if (strcmp(subtoken, "c") == 0) {
					val = 0x0a;
				} else if (strcmp(subtoken, "x") == 0) {
					val = 0x0b;
				} else if (strcmp(subtoken, "y") == 0) {
					val = 0x0c;
				} else if (strcmp(subtoken, "z") == 0) {
					val = 0x0d;
				} else if (strcmp(subtoken, "i") == 0) {
					val = 0x0e;
				} else if (strcmp(subtoken, "j") == 0) {
					val = 0x0f;
				}

				subtoken = strtok_r(NULL, "+", &saveptr);
				if (subtoken != NULL) {
					tolowercase(subtoken);
					if (is_number(subtoken)) {
						assembled[PC++] = hexstr_to_int(subtoken);
						return val;
					} else if (is_string(subtoken)) {
						//TODO
					} else {
						die(strcat("Expected real value, instead got: ", subtoken));
					}
				}
			} else if (is_number(subtoken)) {
				printf("%s is number\n", subtoken);
				assembled[PC++] = hexstr_to_int(subtoken);
				subtoken = strtok_r(NULL, "+", &saveptr);
				if (subtoken != NULL) {
					tolowercase(subtoken);
					if (strcmp(subtoken, "a") == 0) {
						return 0x10;
					} else if (strcmp(subtoken, "b") == 0) {
						return 0x11;
					} else if (strcmp(subtoken, "c") == 0) {
						return 0x12;
					} else if (strcmp(subtoken, "x") == 0) {
						return 0x13;
					} else if (strcmp(subtoken, "y") == 0) {
						return 0x14;
					} else if (strcmp(subtoken, "z") == 0) {
						return 0x15;
					} else if (strcmp(subtoken, "i") == 0) {
						return 0x16;
					} else if (strcmp(subtoken, "j") == 0) {
						return 0x17;
					} else {
						die(strcat("Invalid register: ", subtoken));
					}
				} else {
					return 0x1e;//next word
				}
			} else {
				die(strcat("Malformed operand: ", subtoken));
			}
			//free(token2);
		}
	}

	return 0;
}

uint16_t get_opcode(char *op) {
	if (strcmp(op, "set") == 0) {
		return 0x1;
	} else if (strcmp(op, "add") == 0) {
		return 0x2;
	} else if (strcmp(op, "sub") == 0) {
		return 0x3;
	} else if (strcmp(op, "mul") == 0) {
		return 0x4;
	} else if (strcmp(op, "div") == 0) {
		return 0x5;
	} else if (strcmp(op, "mod") == 0) {
		return 0x6;
	} else if (strcmp(op, "shl") == 0) {
		return 0x7;
	} else if (strcmp(op, "shr") == 0) {
		return 0x8;
	} else if (strcmp(op, "and") == 0) {
		return 0x9;
	} else if (strcmp(op, "bor") == 0) {
		return 0xa;
	} else if (strcmp(op, "xor") == 0) {
		return 0xb;
	} else if (strcmp(op, "ife") == 0) {
		return 0xc;
	} else if (strcmp(op, "ifn") == 0) {
		return 0xd;
	} else if (strcmp(op, "ifg") == 0) {
		return 0xe;
	} else if (strcmp(op, "ifb") == 0) {
		return 0xf;
	} else {
		return 0x0;//non-basic opcode
	}
}

void assemble() {
	uint16_t _PC = PC++;
	total_instructions++;
	int a, b;
	char* op = token;

	if (strcmp(token, "push") == 0) {
		op = "set";
		a = 0x1a;//push
		b = assemble_operand();
	} else if (strcmp(token, "pop") == 0) {
		op = "set";
		a = assemble_operand();
		b = 0x18;//pop
	} else if (strcmp(token, "nop") == 0) {
		op = "set";
		a = 0x20;
		b = 0x20;
	} else {
		a = assemble_operand();
		b = assemble_operand();
	}

	uint16_t opcode = get_opcode(op);
	printf("o=%x a=%x b=%x\n", opcode, a, b);
	assembled[_PC] = opcode;//lower 4 bits
	assembled[_PC] |= (a << 4);//lower 6 of 12 bits
	assembled[_PC] |= (b << 10);//higher 6 of 12 bits
}

int main(int argc, char **argv) {
	printf("-----------------------------\n");
	for(;;) {
		next_token();

		if (token == NULL) {
			printf("-----------------------------\n%d instructions assembled, pc = %d\n", total_instructions, PC);
			printf("Assembled data: \n");
			int i;
			for (i = 0; i < PC; i++) {
				printf("\tvalue: 0x%x\n", assembled[i]);
			}
			die("Null token reached!");
			break;
		} else {
			assemble();
		}
	}

	return 0;
}