#include "globals.h"

#define LINENO_STK_SIZE 100
static int lineno_stk[LINENO_STK_SIZE];
static int lineno_stk_index = 0;

void push_lineno(int x) {
	if(lineno_stk_index == LINENO_STK_SIZE) {
		error(RuntimeError, "lineno stack overflow");
	}
	lineno_stk[lineno_stk_index++] = x;
}

int pop_lineno(void) {
	if(lineno_stk_index == 0) {
		error(Bug, "lineno stack is empty when poping");
	}
	return lineno_stk[--lineno_stk_index];
}

/******************************/

Ast new_ast_node(void) {
	Ast node = (Ast) malloc(sizeof(struct AstNode));
	if(node == NULL) {
		error(RuntimeError, "memory allocation fail in new_ast_node()");
	}
	
	node->sibling = NULL;
	int i;
	for(i = 0; i < MAX_CHILDREN; ++i) {
		node->children[i] = NULL;
	}
	
	node->number = 0;
	node->name = NULL;
	node->operator = 0;
	
	node->kind = -1;
	node->type = -1;
	node->has_else = FALSE;
	
	node->lineno = -1;
	
	node->symtab = NULL;
	
	return node;
}

/******************************/

static int indentno = -1;

#define INDENT indentno+=1
#define UNINDENT indentno-=1

static void print_spaces(void) {
	int i;
	for(i=0; i<indentno; i++) {
		printf("\t");
	}
}

/******************************/

static void print_ast(Ast node) {
	INDENT;
	while(node != NULL) {
		print_spaces();

		switch(node->kind) {
		case Decl_Var:
			printf("Decl_Var: name = %s, lineno = %d, type = %s",
					node->name, node->lineno, type_to_str(node->type));
			if(node->type == IntT) {
				printf("\n");
			} else {
				assert(node->type == IntArrayT);
				printf(", size = %d\n", node->number);
			}
			break;
		case Decl_Fun:
			printf("Decl_Fun: name = %s, lineno = %d, type = %s\n",
					node->name, node->lineno, type_to_str(node->type));
			print_spaces(); printf("--list of parameters:\n");
			print_ast(node->children[0]);
			print_spaces(); printf("--body of function:\n");
			print_ast(node->children[1]);
			break;
		case Decl_Param:
			printf("Decl_Param: name = %s, lineno = %d, type = %s\n",
					node->name, node->lineno, type_to_str(node->type));
			break;
		case Stat_Compound:
			printf("Stat_Compound: \n");
			print_ast(node->children[0]);
			break;
		case Stat_Select:
			printf("Stat_Select: has_else = %d\n", node->has_else);
			print_spaces(); printf("--expression of condition:\n");
			print_ast(node->children[0]);
			print_spaces(); printf("--statements of if:\n");
			print_ast(node->children[1]);
			if(node->has_else == TRUE) {
				print_spaces(); printf("--statements of else:\n");
				print_ast(node->children[2]);
			}
			break;
		case Stat_Iterate:
			printf("Stat_Iterate:\n");
			print_spaces(); printf("--expression of condition:\n");
			print_ast(node->children[0]);
			print_spaces(); printf("--statements of body:\n");
			print_ast(node->children[1]);
			break;
		case Stat_Return:
			printf("Stat_Return: type = %s\n", type_to_str(node->type));
			if(node->type == IntT) {
				print_ast(node->children[0]);
			} else {
				assert(node->type == VoidT);
			}
			break;
		case Stat_Empty:
			printf("Stat_Empty:\n");
			break;
		case Stat_Expression:
			printf("Stat_Expression:\n");
			print_ast(node->children[0]);
			break;
		case Expr_Assign:
			printf("Expr_Assign: lineno = %d\n", node->lineno);
			print_spaces(); printf("--variable:\n");
			print_ast(node->children[0]);
			print_spaces(); printf("--expression:\n");
			print_ast(node->children[1]);
			break;
		case Expr_Binary:
			printf("Expr_Binary: oper = \"%s\", lineno = %d\n",
					operator_to_str(node->operator), node->lineno);
			break;
			print_spaces(); printf("--left expression:\n");
			print_ast(node->children[0]);
			print_spaces(); printf("--right expression:\n");
			print_ast(node->children[1]);
			break;
		case Expr_Fun:
			printf("Expr_Fun: name = %s, lineno = %d\n", node->name, node->lineno);
			print_ast(node->children[0]);
			break;
		case Expr_Var:
			printf("Expr_Var: name = %s, lineno = %d, type = %s\n",
					node->name, node->lineno, type_to_str(node->type));
			if(node->type == IntArrayT) {
				print_ast(node->children[0]);
			} else {
				assert(node->type == IntT);
			}
			break;
		case Expr_Const:
			printf("Expr_Const: value = %d, lineno = %d\n", node->number, node->lineno);
			break;
		default:
			error(Bug, "unknown ast node kind");
		}

		node = node->sibling;
	}
	UNINDENT;
}

void print_ast_root(Ast root) {
	print_ast(root);
}

