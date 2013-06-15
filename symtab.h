#ifndef SYMTAB_H
#define SYMTAB_H

struct EntryDec;
typedef struct EntryDec * Entry;

struct EntryDec {
	char* name;
	Type type;
	int no;
	
	Entry next;
};

/******************************/

typedef enum { Global_Var, Fun_Param, Compound_Var } StKind;

struct SymtabDec;
typedef struct SymtabDec * Symtab;

struct SymtabDec {
	StKind kind;
	int size;
	Entry head;
	
	Symtab next;
};

extern Symtab global_var;

/******************************/

struct FunInfoDec;
typedef struct FunInfoDec * FunInfo;

struct FunInfoDec {
	char* name;
	Type type;
	
	int param_size;
	int var_size;
	Symtab symtab;
	
	int address;
	
	FunInfo next;
};

extern FunInfo fun_list;

/******************************/

void build_symtab_root(Ast root);
void print_symtab_root(Ast root);
void print_funlist();

#endif /* SYMTAB_H */
