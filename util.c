#include "globals.h"

// only for yacc
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
			// with lineno
			fprintf(stderr, "%s in line %d: ", errortype_to_str(type), yylineno);
			break;
		case Bug:
		case RuntimeError:
			// without lineno
			fprintf(stderr, "%s: ", errortype_to_str(type));
			break;
		default:
			error(Bug, "unknown error type");
	}
	
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");

	exit(1);
}

char* errortype_to_str(ErrorType type) {
	switch(type) {
		case Bug:           return "Bug";
		case RuntimeError:  return "Runtime Error";
		case LexicalError:  return "Lexical Error";
		case SyntaxError:   return "Syntax Error";
		case SemanticError: return "Semantic Error";
		case TypeError:     return "Type Error";
		default: error(Bug, "unknown error type");
	}
}

char* type_to_str(Type type) {
	switch(type) {
		case IntT:      return "IntT";
		case IntArrayT: return "IntArrayT";
		case VoidT:     return "VoidT";
		case BoolT:     return "BoolT";
		default: error(Bug, "unknown type");
	}
}

char* stkind_to_str(StKind kind) {
	switch(kind) {
		case Global_Var:   return "Global_Var";
		case Fun_Param:    return "Fun_Param";
		case Compound_Var: return "Compound_Var";
		default: error(Bug, "unknown symtab kind");
	}
}

char* operator_to_str(int op) {
	switch(op) {
		case '+': return "+";
		case '-': return "-";
		case '*': return "*";
		case '/': return "/";
		case EQ: return "==";
		case NE: return "!=";
		case LT: return "<";
		case LE: return "<=";
		case GT: return ">";
		case GE: return ">=";
		default: error(Bug, "unknown operator");
	}
}

