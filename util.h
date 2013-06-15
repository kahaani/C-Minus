#ifndef UTIL_H
#define UTIL_H

typedef enum {
	Bug, RuntimeError,
	LexicalError, SyntaxError, SemanticError, TypeError
} ErrorType;

void yyerror(char *s, ...);
void error(ErrorType type, char *s, ...);

char* type_to_str(Type type);
char* stkind_to_str(StKind kind);

#endif /* UTIL_H */
