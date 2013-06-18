#ifndef UTIL_H
#define UTIL_H

typedef enum {
	Bug, RuntimeError,
	LexicalError, SyntaxError, SemanticError, TypeError
} ErrorType;

void yyerror(char *s, ...);
void error(ErrorType type, char *s, ...);
void warning(char *s, ...);

char* errortype_to_str(ErrorType type);
char* type_to_str(Type type);
char* stkind_to_str(StKind kind);
char* operator_to_str(int op);

#endif /* UTIL_H */
