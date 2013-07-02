#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

typedef int Bool;

/******************************/

// declaration in symtab.h
struct SymtabDec;
typedef struct SymtabDec * Symtab;

/******************************/

typedef enum {
	Decl_Var, Decl_Fun, Decl_Param,
	Stat_Compound, Stat_Select, Stat_Iterate, Stat_Return, Stat_Empty, Stat_Expression,
	Expr_Assign, Expr_Binary, Expr_Fun, Expr_Var, Expr_Const
} Kind;

typedef enum { IntT, IntArrayT, VoidT, BoolT } Type;

#define MAX_CHILDREN 3

struct AstNode;
typedef struct AstNode * Ast;

struct AstNode {
	Ast sibling;
	Ast children[MAX_CHILDREN];
	
	int   number;
	char* name;
	int   operator; // not char
	
	Kind kind;
	Type type;
	int has_else;

	int lineno;
	
	Symtab symtab; // for semantic
};

/******************************/

#ifndef YYPARSER
#include "parse.tab.h"
#endif /* YYPARSER */

extern int yylineno;
int yylex(void);
Ast parse(void);

/******************************/

#include "scan.aux.h"
#include "parse.aux.h"
#include "semantic.aux.h"
#include "semantic.h"
#include "code.aux.h"
#include "code.h"
#include "util.h"

#define TRACE_SCAN     FALSE
#define YYDEBUG        FALSE
#define TRACE_PARSE    FALSE
#define TRACE_SEMANTIC FALSE

#endif /* GLOBALS_H */
