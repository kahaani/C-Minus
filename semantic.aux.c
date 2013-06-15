#include "globals.h"

Symtab  global_var = NULL;

/******************************/

FunInfo fun_list = NULL;

void push_funinfo(FunInfo funinfo) {
	funinfo->next = fun_list;
	fun_list = funinfo;
}

FunInfo top_funinfo() {
	if(fun_list == NULL) {
		error(Bug, "fun list is empty when toping");
	}
	return fun_list;
}

void reverse_fun_list() {
	FunInfo funinfo = fun_list;
	fun_list = NULL;
	
	while(funinfo != NULL) {
		FunInfo tmp = funinfo;
		funinfo = funinfo->next;
		push_funinfo(tmp);
	}
}

/******************************/

static Symtab st_stk = NULL;

void push_symtab(Symtab symtab) {
	symtab->next = st_stk;
	st_stk = symtab;
}

void pop_symtab() {
	if(st_stk == NULL) {
		error(Bug, "symtab stack is empty when poping");
	}
	st_stk = st_stk->next;
}

Symtab top_symtab() {
	if(st_stk == NULL) {
		error(Bug, "symtab stack is empty when toping");
	}
	return st_stk;
}

/******************************/

Entry new_entry(char* name, Type type, int offset, StKind kind, Entry next) {
	Entry entry = (Entry) malloc(sizeof(struct EntryDec));
	if(entry == NULL) {
		error(RuntimeError, "memory allocation fail in new_entry()");
	}
	
	entry->name = name;
	entry->type = type;
	entry->offset = offset;
	entry->kind = kind;
	entry->next = next;
	
	return entry;
}

Symtab new_symtab(StKind kind) {
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

FunInfo new_funinfo(char* name, Type type, Symtab symtab) {
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

int insert_entry(char* name, Type type, int array_size, int lineno) {
	Symtab symtab = top_symtab();

	// Step 1: check if duplicated
	// only search the top symtab, cannot be replaced by lookup_entry()

	Entry p = symtab->head;
	while(p != NULL && strcmp(p->name, name) != 0) {
		p = p->next;
	}
	if(p != NULL) {
		yylineno = lineno;
		error(SemanticError, "duplicated declaration of variable \"%s\"", name);
	}

	// Step 2: calculate position, new an entry and add it into symtab

	int offset = -1;
	switch(symtab->kind) {
		case Global_Var:
		case Fun_Param:
			offset = symtab->size;
			break;
		case Compound_Var:
			offset = top_funinfo()->var_size;
			break;
		default:
			error(Bug, "unknown symtab kind");
	}

	Entry entry = new_entry(name, type, offset, symtab->kind, symtab->head);
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

Entry lookup_entry(char* name) {
	Symtab symtab = top_symtab();
	while(symtab != NULL) {
		Entry entry = symtab->head;
		while(entry != NULL) {
			if(strcmp(entry->name, name) == 0) {
				return entry;
			}
			entry = entry->next;
		}
		symtab = symtab->next;
	}
	
	return NULL;
}

FunInfo lookup_funinfo(char* name) {
	FunInfo funinfo = top_funinfo();
	while(funinfo != NULL) {
		if(strcmp(funinfo->name, name) == 0) {
			return funinfo;
		}
		funinfo = funinfo->next;
	}
	return NULL;
}

/******************************/

FunInfo prelude_input() {
	Symtab symtab = new_symtab(Fun_Param);

	FunInfo funinfo = new_funinfo("input", IntT, symtab);
	return funinfo;
}

FunInfo prelude_output() {
	Entry entry = new_entry("x", IntT, 0, Fun_Param, NULL);

	Symtab symtab = new_symtab(Fun_Param);
	symtab->head = entry;
	symtab->size = 1;

	FunInfo funinfo = new_funinfo("output", VoidT, symtab);
	funinfo->param_size = 1;
	return funinfo;
}

/******************************/

static void print_entry(Entry entry) {
	printf("--Entry: name = %s, type = %s, kind = %s, offset = %d\n",
			entry->name, type_to_str(entry->type), stkind_to_str(entry->kind), entry->offset);
}

static void print_symtab(Symtab symtab) {
	printf("Symtab: kind = %s, size = %d\n", stkind_to_str(symtab->kind), symtab->size);
	Entry entry = symtab->head;
	while(entry != NULL) {
		print_entry(entry);
		entry = entry->next;
	}
}

static void print_symtab_node(Ast node) {
	while(node != NULL) {
		switch(node->kind) { // only four kinds possible
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
	// prelude symtab: global variable
	print_symtab(global_var);
	
	// two prelude functions: input() & output()
	print_symtab(top_funinfo()->symtab);
	print_symtab(top_funinfo()->next->symtab);
	
	// print ast recursively
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

