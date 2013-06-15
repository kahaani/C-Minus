#include "globals.h"

void trace_scan() {
	int token;
	
	while(token = yylex()) {
		if(token < 258) {
			printf("line %d: %c", yylineno, token);
		} else {
			printf("line %d: %d", yylineno, token);
		}

		if(token == NUM) {
			printf(": %d\n", yylval.int_value);
		} else if(token == ID) {
			printf(": %s\n", yylval.str_value);
		} else {
			printf("\n");
		}
	}
}

