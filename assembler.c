#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

const char *token_delimeters = " ,\t\n";
char input[] = "SET A, 0x69\nSET B, 0x68";
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

int assemble_operand() {
	next_token();

	if (token == "a") {
		return 0x00;
	} else if (token == "b") {
		return 0x01;
	} else if (token == "c") {
		return 0x02;
	} else if (token == "x") {
		return 0x03;
	} else if (token == "y") {
		return 0x04;
	} else if (token == "z") {
		return 0x05;
	} else if (token == "i") {
		return 0x06;
	} else if (token == "j") {
		return 0x07;
	//TODO: Bracketed registers
	} else if (token == "pop") {
		return 0x18;
	} else if (token == "peek") {
		return 0x19;
	} else if (token == "push") {
		return 0x1a;
	} else if (token == "sp") {
		return 0x1b;
	} else if (token == "pc") {
		return 0x1c;
	} else if (token == "o") {
		return 0x1d;
	} else {
		if (is_string(token)) {
			assembled[PC] = 0;
			//TODO: Strings
			return 0x1f;
		} else if (is_number(token)) {
			int n = hexstr_to_int(token);
			if (n < 0x20) return n + 0x20;
			assembled[PC++] = n;
			return 0x1f;
		} else {
			//must be bracketed []
			//TODO: Bracketed values
		}
	}

	return 0;
}

int get_opcode(char *op) {
	if (op == "set") return 0x1;
	if (op == "add") return 0x2;
	if (op == "sub") return 0x3;
	if (op == "mul") return 0x4;
	if (op == "div") return 0x5;
	if (op == "mod") return 0x6;
	if (op == "shl") return 0x7;
	if (op == "shr") return 0x8;
	if (op == "and") return 0x9;
	if (op == "bor") return 0xa;
	if (op == "xor") return 0xb;
	if (op == "ife") return 0xc;
	if (op == "ifn") return 0xd;
	if (op == "ifg") return 0xe;
	if (op == "ifb") return 0xf;
	return 0x0;//non-basic opcode
}

void assemble() {
	uint16_t _PC = PC++;
	total_instructions++;
	int a, b;
	char* op = token;

	if (token == "push") {
		op = "set";
		a = 0x1a;//push
		b = assemble_operand();
	} else if (token == "pop") {
		op = "set";
		a = assemble_operand();
		b = 0x18;//pop
	} else if (token == "nop") {
		op = "set";
		a = 0x20;
		b = 0x20;
	} else {
		a = assemble_operand();
		b = assemble_operand();
	}

	printf("o[%x] a[%x] b[%x]\n", get_opcode(op), a, b);
	assembled[_PC] = get_opcode(op) | (a << 4) | (b << 10);
}

int main(int argc, char **argv) {
	for(;;) {
		next_token();

		if (token == NULL) {
			printf("%d instructions assembled, pc = %d\n", total_instructions, PC);
			printf("Output: \n");
			int i = 0;
			for (i; i < PC; i++) {
				printf("value: 0x%d\n", assembled[i]);
			}
			die("Null token reached!");
			break;
		} else {
			assemble();
		}
	}

	return 0;
}