#include "globals.h"

static char* errortype_to_str(ErrorType type);

// for yacc
void yyerror(char *s, ...) {
	va_list ap;
	va_start(ap, s);

	fprintf(stderr, "Syntax Error in line %d: ", yylineno);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");

	exit(1);
}

void error(ErrorType type, char *s, ...) {
	va_list ap;
	va_start(ap, s);

	switch(type) {
		case LexicalError:
		case SyntaxError:
		case SemanticError:
		case TypeError:
			fprintf(stderr, "%s in line %d: ", errortype_to_str(type), yylineno);
			break;
		default:
			fprintf(stderr, "%s: ", errortype_to_str(type));
	}
	
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");

	exit(1);
}

static char* errortype_to_str(ErrorType type) {
	char* s;
	switch(type) {
		case Bug:
			s = "Bug";
			break;
		case RuntimeError:
			s = "Runtime Error";
			break;
		case LexicalError:
			s = "Lexical Error";
			break;
		case SyntaxError:
			s = "Syntax Error";
			break;
		case SemanticError:
			s = "Semantic Error";
			break;
		case TypeError:
			s = "Type Error";
			break;
		default:
			error(Bug, "unknown error type");
	}
	return s;
}

char* type_to_str(Type type) {
	char* s;
	switch(type) {
		case IntT:
			s = "IntT";
			break;
		case IntArrayT:
			s = "IntArrayT";
			break;
		case VoidT:
			s = "VoidT";
			break;
		case BoolT:
			s = "BoolT";
			break;
		default:
			error(Bug, "unknown type");
	}
	return s;
}

char* stkind_to_str(StKind kind) {
	char* s;
	switch(kind) {
		case Global_Var:
			s = "Global_Var";
			break;
		case Fun_Param:
			s = "Fun_Param";
			break;
		case Compound_Var:
			s = "Compound_Var";
			break;
		default:
			error(Bug, "unknown symtab kind");
	}
	return s;
}


