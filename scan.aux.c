#include "globals.h"

void trace_scan() {
	int token;
	
	while(token = yylex()) {
		if(token < 258) { // character
			printf("line %d: %c", yylineno, token);
		} else { // token
			printf("line %d: %d", yylineno, token);
		}

		// token with value
		if(token == NUM) {
			printf(": %d\n", yylval.int_value);
		} else if(token == ID) {
			printf(": %s\n", yylval.str_value);
		} else {
			printf("\n");
		}
	}
}

