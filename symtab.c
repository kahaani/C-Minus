#include "globals.h"

Symtab  global_var = NULL;

/******************************/

FunInfo fun_list = NULL;

static void push_funinfo(FunInfo funinfo) {
	funinfo->next = fun_list;
	fun_list = funinfo;
}

static FunInfo top_funinfo() {
	if(fun_list == NULL) {
		error(Bug, "fun list is empty when toping");
	}
	return fun_list;
}

/******************************/

static Symtab st_stk = NULL;

static void push_symtab(Symtab symtab) {
	symtab->next = st_stk;
	st_stk = symtab;
}

static void pop_symtab() {
	if(st_stk == NULL) {
		error(Bug, "symtab stack is empty when poping");
	}
	st_stk = st_stk->next;
}

static Symtab top_symtab() {
	if(st_stk == NULL) {
		error(Bug, "symtab stack is empty when toping");
	}
	return st_stk;
}

/******************************/

static Entry new_entry(char* name, Type type, int no, Entry next) {
	Entry entry = (Entry) malloc(sizeof(struct EntryDec));
	if(entry == NULL) {
		error(RuntimeError, "memory allocation fail in new_entry()");
	}
	
	entry->name = name;
	entry->type = type;
	entry->no   = no;
	entry->next = next;
	
	return entry;
}

static Symtab new_symtab(StKind kind) {
	Symtab symtab = (Symtab) malloc(sizeof(struct SymtabDec));
	if(symtab == NULL) {
		error(RuntimeError, "memory allocation fail in new_symtab()");
	}
	
	symtab->kind = kind;
	symtab->size = 0;
	symtab->head = NULL;
	symtab->next = NULL;
	
	return symtab;
}

static FunInfo new_funinfo(char* name, Type type, Symtab symtab) {
	FunInfo funinfo = (FunInfo) malloc(sizeof(struct FunInfoDec));
	if(funinfo == NULL) {
		error(RuntimeError, "memory allocation fail in new_funinfo()");
	}
	
	funinfo->name = name;
	funinfo->type = type;

	funinfo->param_size = 0;
	funinfo->var_size = 0;
	funinfo->symtab = symtab;
	
	funinfo->address = -1;
	funinfo->next = NULL;
	
	return funinfo;
}

/******************************/

static int insert_entry(char* name, Type type, int array_size, int lineno) {
	Symtab symtab = top_symtab();

	// Step 1: check if duplicated

	Entry p = symtab->head;
	while(p != NULL && strcmp(p->name, name) != 0) {
		p = p->next;
	}
	if(p != NULL) {
		yylineno = lineno;
		error(SemanticError, "duplicated declaration of \"%s\"", name);
	}

	// Step 2: calculate position, new an entry and add it into symtab

	int no = -1;
	switch(symtab->kind) {
		case Global_Var:
		case Fun_Param:
			no = symtab->size;
			break;
		case Compound_Var:
			no = top_funinfo()->var_size;
			break;
		default:
			error(Bug, "unknown symtab kind");
	}

	Entry entry = new_entry(name, type, no, symtab->head);
	symtab->head = entry;

	// Step 3: adjust size of both symtab and funinfo

	if(symtab->kind != Fun_Param && type == IntArrayT) {
		symtab->size += array_size;
	} else {
		symtab->size ++;
	}
	
	switch(symtab->kind) {
		case Global_Var:
			; // do nothing
			break;
		case Fun_Param:
			top_funinfo()->param_size ++;
			break;
		case Compound_Var:
			if(type == IntArrayT) {
				top_funinfo()->var_size += array_size;
			} else {
				assert(type == IntT);
				top_funinfo()->var_size ++;
			}
			break;
		default:
			error(Bug, "unknown symtab kind");
	}
}

/******************************/

static void build_symtab(Ast node) {
	while(node != NULL) {
		switch(node->kind) {

		case Decl_Var:
			insert_entry(node->name, node->type, node->number, node->lineno);
			break;

		case Decl_Fun:
			node->symtab = new_symtab(Fun_Param);
			push_symtab(node->symtab);
			
			FunInfo funinfo = new_funinfo(node->name, node->type, node->symtab);
			push_funinfo(funinfo);
			
			build_symtab(node->children[0]);
			build_symtab(node->children[1]);
			
			pop_symtab();
			break;

		case Decl_Param: // similar to Decl_Var
			insert_entry(node->name, node->type, node->number, node->lineno);
			break;

		case Stat_Compound: // similar to Decl_Fun
			node->symtab = new_symtab(Compound_Var);
			push_symtab(node->symtab);
			
			build_symtab(node->children[0]);
			
			pop_symtab();
			break;

		case Stat_Select:
			build_symtab(node->children[1]);
			if(node->has_else == TRUE) {
				build_symtab(node->children[2]);
			}
			break;

		case Stat_Iterate:
			build_symtab(node->children[1]);
			break;

		case Stat_Return:
			break;
		case Stat_Empty:
			break;
		case Stat_Expression:
			break;
		case Expr_Assign:
			break;
		case Expr_Binary:
			break;
		case Expr_Fun:
			break;
		case Expr_Var:
			break;
		case Expr_Const:
			break;
		default:
			error(Bug, "unknown ast node kind");
		}

		node = node->sibling;
	}
}

void build_symtab_root(Ast root) {
	global_var = new_symtab(Global_Var);
	push_symtab(global_var);
	
	build_symtab(root);
}

/******************************/

static void print_symtab(Symtab symtab) {
	printf("Symtab: kind = %s, size = %d\n", stkind_to_str(symtab->kind), symtab->size);
	Entry entry = symtab->head;
	while(entry != NULL) {
		printf("--Entry: name = %s, type = %s, no = %d\n", entry->name, type_to_str(entry->type), entry->no);
		entry = entry->next;
	}
}

static void print_symtab_node(Ast node) {
	while(node != NULL) {
		switch(node->kind) {
		case Decl_Fun:
			print_symtab(node->symtab);
			print_symtab_node(node->children[0]);
			print_symtab_node(node->children[1]);
			break;
		case Stat_Compound:
			print_symtab(node->symtab);
			print_symtab_node(node->children[0]);
			break;
		case Stat_Select:
			print_symtab_node(node->children[1]);
			if(node->has_else == TRUE) {
				print_symtab_node(node->children[2]);
			}
			break;
		case Stat_Iterate:
			print_symtab_node(node->children[1]);
			break;
		default:
			; // do nothing
		}
		node = node->sibling;
	}
}

void print_symtab_root(Ast root) {
	print_symtab(global_var);
	print_symtab_node(root);
}

void print_funlist() {
	FunInfo funinfo = top_funinfo();

	while(funinfo != NULL) {
		printf("FunInfo: name = %s, type = %s, param_size = %d, var_size = %d\n",
				funinfo->name, type_to_str(funinfo->type), funinfo->param_size, funinfo->var_size);
		funinfo = funinfo->next;
	}
}

