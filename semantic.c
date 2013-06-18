#include "globals.h"

static int ignore_array_without_index = FALSE;

// for expression, check symtab
static Type check_symtab(Ast node) {
	assert(node != NULL);

	Entry entry = NULL;
	Type type = -1;

	switch(node->kind) {

	case Decl_Var:
	case Decl_Param:
	case Decl_Fun:
		error(Bug, "declaration ast node should not attend in check_symtab()");

	case Stat_Compound:
	case Stat_Select:
	case Stat_Iterate:
	case Stat_Return:
	case Stat_Empty:
	case Stat_Expression:
		error(Bug, "statement ast node should not attend in check_symtab()");

	case Expr_Assign:
		type = check_symtab(node->children[0]);
		if(type != IntT) {
			yylineno = node->lineno;
			error(TypeError, "left value must be INT");
		}
		
		type = check_symtab(node->children[1]);
		if(type != IntT) {
			yylineno = node->lineno;
			error(TypeError, "right value must be INT");
		}

		return IntT;

	case Expr_Binary:
		type = check_symtab(node->children[0]);
		if(type != IntT) {
			yylineno = node->lineno;
			error(TypeError, "left expression of operator \"%s\" must be INT",
					operator_to_str(node->operator));
		}

		type = check_symtab(node->children[1]);
		if(type != IntT) {
			yylineno = node->lineno;
			error(TypeError, "right expression of operator \"%s\" must be INT",
					operator_to_str(node->operator));
		}

		switch(node->operator) {
			case '+':
			case '-':
			case '*':
			case '/':
				return IntT;
			case EQ:
			case NE:
			case LT:
			case LE:
			case GT:
			case GE:
				return BoolT;
			default:
				error(Bug, "unknown operator");
		}

	case Expr_Fun:
		; // a label can only be part of a statement and a declaration is not a statement
		FunInfo funinfo = lookup_funinfo(node->name);
		if(funinfo == NULL) {
			yylineno = node->lineno;
			error(SemanticError, "undefined function \"%s\"", node->name);
		}
		
		entry = funinfo->symtab->head;
		Ast param = node->children[0];

		while(entry != NULL && param != NULL) { // parameter order: form right to left
			if(param->kind == Expr_Var) { // special case (very restricted)
				ignore_array_without_index = TRUE;
			}
			type = check_symtab(param);
			ignore_array_without_index = FALSE;

			if(type != entry->type) {
				yylineno = node->lineno;
				error(SemanticError,
					"type mismatch between actual parameter \"%s\" and formal parameter \"%s\" of function \"%s\"",
					param->name, entry->name, node->name);
			}
			
			entry = entry->next;
			param = param->sibling; // list of expression, only here
		}
		
		if(entry != NULL || param != NULL) {
			yylineno = node->lineno;
			error(SemanticError, "unmatched parameter number of function \"%s\"", node->name);
		}
		
		return funinfo->type;

	case Expr_Var:
		entry = lookup_entry(node->name);
		if(entry == NULL) {
			yylineno = node->lineno;
			error(SemanticError, "undefined variable \"%s\"", node->name);
		}

		if(entry->type == IntArrayT && node->type == IntArrayT) {
			type = check_symtab(node->children[0]);
			if(type != IntT) {
				yylineno = node->lineno;
				error(TypeError, "index of array \"%s\" must be INT", node->name);
			}
			return IntT;
		} else if(entry->type == IntArrayT && node->type == IntT) {
			if(ignore_array_without_index) { // only usage of ignore_array_without_index
				return IntArrayT;
			} else {
				yylineno = node->lineno;
				error(TypeError, "array variable \"%s\" should followed by []", node->name);
			}
		} else if(entry->type == IntT && node->type == IntArrayT) {
			yylineno = node->lineno;
			error(TypeError, "single variable \"%s\" cannot followed by []", node->name);
		} else {
			assert(entry->type == IntT && node->type == IntT);
			return IntT;
		}

	case Expr_Const:
		return IntT;

	default:
		error(Bug, "unknown ast node kind");
	}
}

// for declaration, build symtab
// for statement, walk through
static void build_symtab(Ast node) {
	FunInfo funinfo = NULL;
	Type type = -1;
	
	while(node != NULL) {
		switch(node->kind) {

		case Decl_Var:
			insert_entry(node->name, node->type, node->number, node->lineno);
			break;

		case Decl_Param: // similar to Decl_Var
			insert_entry(node->name, node->type, node->number, node->lineno);
			break;

		case Decl_Fun:
			funinfo = lookup_funinfo(node->name);
			if(funinfo != NULL) {
				yylineno = node->lineno;
				error(SemanticError, "duplicated declaration of function \"%s\"", node->name);
			}
			
			node->symtab = new_symtab(Fun_Param);
			push_symtab(node->symtab);
			
			funinfo = new_funinfo(node->name, node->type, node->symtab);
			push_funinfo(funinfo);
			
			build_symtab(node->children[0]); // recursion
			build_symtab(node->children[1]); // recursion
			
			pop_symtab();
			break;

		case Stat_Compound: // similar to Decl_Fun, except without funinfo
			node->symtab = new_symtab(Compound_Var);
			push_symtab(node->symtab);
			
			build_symtab(node->children[0]); // recursion
			
			pop_symtab();
			break;

		case Stat_Select:
			type = check_symtab(node->children[0]);
			if(type != BoolT) {
				yylineno = node->lineno;
				error(TypeError, "condiction of if-statement must be BOOL");
			}
			
			build_symtab(node->children[1]); // recursion
			if(node->has_else == TRUE) {
				build_symtab(node->children[2]); // recursion
			}
			break;

		case Stat_Iterate:
			type = check_symtab(node->children[0]);
			if(type != BoolT) {
				yylineno = node->lineno;
				error(TypeError, "condiction of while-loop must be BOOL");
			}
			
			build_symtab(node->children[1]); // recursion
			break;

		case Stat_Return:
			funinfo = top_funinfo();
			
			if(funinfo->type == IntT && node->type == IntT) {
				type = check_symtab(node->children[0]);
				if(type != IntT) {
					yylineno = node->lineno;
					error(TypeError, "return value must be INT");
				}
			} else if (funinfo->type == VoidT && node->type == VoidT) {
				; // do nothing;
			} else {
				assert((funinfo->type == IntT && node->type == VoidT)
					|| (funinfo->type == VoidT && node->type == IntT));
				yylineno = node->lineno;
				error(TypeError, "type mismatch between return statement and function prototype");
			}
			break;

		case Stat_Empty:
			break;

		case Stat_Expression:
			check_symtab(node->children[0]);
			break;

		case Expr_Assign:
		case Expr_Binary:
		case Expr_Fun:
		case Expr_Var:
		case Expr_Const:
			error(Bug, "expression ast node should not attend in build_symtab()");

		default:
			error(Bug, "unknown ast node kind");
		}

		node = node->sibling;
	}
}

void build_symtab_root(Ast root) {
	// prelude symtab: global variable
	push_symtab(new_symtab(Global_Var));
	
	// prelude functions: input() & output()
	push_funinfo(prelude_input_funinfo());
	push_funinfo(prelude_output_funinfo());
	
	// build symtab and check type recursively
	build_symtab(root);
	
	// make sure the correctness of symtab stack
	if(top_symtab()->kind != Global_Var) {
		error(Bug, "symtab stack not clean after semantic analysis");
	}
	
	// check the position of main()
	if(strcmp(top_funinfo()->name, "main") != 0) {
		yylineno = -1;
		error(SemanticError, "the last function must be main()");
	}
	
	// reverse the fun_list
	reverse_fun_list();
}

