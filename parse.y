%{

#define YYPARSER /* distinguishes Yacc output from other code files */
#include "globals.h"

static Ast final_tree;

%}

%union {
	int   int_value;
	char* str_value;
	int   token;
	Ast   astree;
	Type  type;
}

%token IF ELSE WHILE RETURN INT VOID
%token LT GT LE GE EQ NE
%token <int_value> NUM
%token <str_value> ID

%type <astree> declaration_list
%type <astree> declaration
%type <astree> var_declaration
%type <astree> fun_declaration
%type <astree> params
%type <astree> param_list
%type <astree> param
%type <astree> compound_stmt
%type <astree> local_declarations
%type <astree> statement_list
%type <astree> statement
%type <astree> args
%type <astree> arg_list

%type <astree> expression_stmt
%type <astree> selection_stmt
%type <astree> iteration_stmt
%type <astree> return_stmt

%type <astree> expression
%type <astree> simple_expression
%type <astree> additive_expression
%type <astree> term
%type <astree> factor
%type <astree> var
%type <astree> call

%type <token> relop
%type <token> addop
%type <token> mulop

%type <type> type_specifier

%start program

%%

program:
	  declaration_list { final_tree = $1; }
	;

/* 改成右递归 */
declaration_list:
	  declaration declaration_list {
	    Ast node = $1;
	    node->sibling = $2;
	    $$ = node;
	  }
	| declaration { $$ = $1; }
	;

declaration:
	  var_declaration { $$ = $1; }
	| fun_declaration { $$ = $1; }
	;

var_declaration:
	  type_specifier ID { push_lineno(yylineno); } ';' {
	    if($1 == VoidT) {
	      error(SemanticError, "declare variable \"%s\" as void", $2); // 提前检查
	    } else {
	      assert($1 == IntT);
	      Ast node = new_ast_node();
	      node->kind = Decl_Var;
	      node->type = IntT;
	      node->name = $2;
	      node->lineno = pop_lineno();
	      $$ = node;
	    }
	  }
	| type_specifier ID { push_lineno(yylineno); } '[' NUM ']' ';' {
	    if($1 == VoidT) {
	      error(SemanticError, "declare variable \"%s\" as void", $2); // 提前检查
	    } else {
	      assert($1 == IntT);
	      Ast node = new_ast_node();
	      node->kind = Decl_Var;
	      node->type = IntArrayT;
	      node->name = $2;
	      node->number = $5;
	      node->lineno = pop_lineno();
	      $$ = node;
	    }
	  }
	;

type_specifier:
	  INT  { $$ = IntT;  }
	| VOID { $$ = VoidT; }
	;

fun_declaration:
	  type_specifier ID { push_lineno(yylineno); } '(' params ')' compound_stmt {
	    Ast node = new_ast_node();
	    node->kind = Decl_Fun;
	    node->type = $1;
	    node->name = $2;
	    node->children[0] = $5;
	    node->children[1] = $7;
	    node->lineno = pop_lineno();
	    $$ = node;
	  }
	;

params:
	  param_list { $$ = $1; }
	| VOID { $$ = NULL; }
	;

/* 改成右递归 */
param_list:
	  param ',' param_list {
	    Ast node = $1;
	    node->sibling = $3;
	    $$ = node;
	  }
	| param { $$ = $1; }
	;

param:
	  type_specifier ID {
	    if($1 == VoidT) {
	      error(SemanticError, "declare parameter \"%s\" as void", $2); // 提前检查
	    } else {
	      assert($1 == IntT);
	      Ast node = new_ast_node();
	      node->kind = Decl_Param;
	      node->type = IntT;
	      node->name = $2;
	      node->lineno = yylineno;
	      $$ = node;
	    }
	  }
	| type_specifier ID { push_lineno(yylineno); } '[' ']' {
	    if($1 == VoidT) {
	      error(SemanticError, "declare parameter \"%s\" as void", $2); // 提前检查
	    } else {
	      assert($1 == IntT);
	      Ast node = new_ast_node();
	      node->kind = Decl_Param;
	      node->type = IntArrayT;
	      node->name = $2;
	      node->lineno = pop_lineno();
	      $$ = node;
	    }
	}
	;

compound_stmt:
	  '{' local_declarations statement_list '}' {
	    Ast root = new_ast_node();
	    root->kind = Stat_Compound;
	    
	    Ast node = $2;
	    if(node != NULL) {
	      while(node->sibling != NULL) {
	        node = node->sibling;
	      }
	      node->sibling = $3;
	      root->children[0] = $2;
	    } else {
	      root->children[0] = $3;
	    }
	    
	    $$ = root;
	  }
	;

/* 改成右递归 */
local_declarations:
	  var_declaration local_declarations {
	    Ast node = $1;
	    node->sibling = $2;
	    $$ = node;
	  }
	| { $$ = NULL; }
	;

/* 改成右递归 */
statement_list:
	  statement statement_list {
	    Ast node = $1;
	    node->sibling = $2;
	    $$ = node;
	  }
	| { $$ = NULL; }
	;

statement:
	  expression_stmt { $$ = $1; }
	| compound_stmt   { $$ = $1; }
	| selection_stmt  { $$ = $1; }
	| iteration_stmt  { $$ = $1; }
	| return_stmt     { $$ = $1; }
	;

expression_stmt:
	  expression ';' { 
	    Ast node = new_ast_node();
	    node->kind = Stat_Expression;
	    node->children[0] = $1;
	    $$ = node;
	  }
	| ';' { 
	    Ast node = new_ast_node();
	    node->kind = Stat_Empty;
	    $$ = node;
	  }
	;

selection_stmt:
	  IF '(' expression ')' statement {
	    Ast node = new_ast_node();
	    node->kind = Stat_Select;
	    node->has_else = FALSE;
	    node->children[0] = $3;
	    node->children[1] = $5;
	    $$ = node;
	  }
	| IF '(' expression ')' statement ELSE statement {
	    Ast node = new_ast_node();
	    node->kind = Stat_Select;
	    node->has_else = TRUE;
	    node->children[0] = $3;
	    node->children[1] = $5;
	    node->children[2] = $7;
	    $$ = node;
	  }
	;

iteration_stmt:
	  WHILE '(' expression ')' statement {
	    Ast node = new_ast_node();
	    node->kind = Stat_Iterate;
	    node->children[0] = $3;
	    node->children[1] = $5;
	    $$ = node;
	  }
	;

return_stmt:
	  RETURN ';' {
	    Ast node = new_ast_node();
	    node->kind = Stat_Return;
	    node->type = VoidT;
	    $$ = node;
	  }
	| RETURN expression ';' {
	    Ast node = new_ast_node();
	    node->kind = Stat_Return;
	    node->type = IntT;
	    node->children[0] = $2;
	    $$ = node;
	  }
	;

expression:
	  var '=' { push_lineno(yylineno); } expression {
	    Ast node = new_ast_node();
	    node->kind = Expr_Assign;
	    node->children[0] = $1;
	    node->children[1] = $4;
	    node->lineno = pop_lineno();
	    $$ = node;
	  }
	| simple_expression { $$ = $1; }
	;

var:
	  ID {
	    Ast node = new_ast_node();
	    node->kind = Expr_Var;
	    node->name = $1;
	    node->type = IntT;
	    node->lineno = yylineno;
	    $$ = node;
	  }
	| ID '[' expression ']' { 
	    Ast node = new_ast_node();
	    node->kind = Expr_Var;
	    node->name = $1;
	    node->type = IntArrayT;
	    node->children[0] = $3;
	    node->lineno = yylineno;
	    $$ = node;
	  }
	;

simple_expression:
	  additive_expression relop { push_lineno(yylineno); } additive_expression {
	    Ast node = new_ast_node();
	    node->kind = Expr_Binary;
	    node->operator = $2;
	    node->type = BoolT; // 提前锁定类型
	    node->children[0] = $1;
	    node->children[1] = $4;
	    node->lineno = pop_lineno();
	    $$ = node;
	  }
	| additive_expression { $$ = $1; }
	;

relop:
	  LE { $$ = LE; }
	| LT { $$ = LT; }
	| GT { $$ = GT; }
	| GE { $$ = GE; }
	| EQ { $$ = EQ; }
	| NE { $$ = NE; }
	;

additive_expression:
	  additive_expression addop { push_lineno(yylineno); } term {
	    Ast node = new_ast_node();
	    node->kind = Expr_Binary;
	    node->operator = $2;
	    node->type = IntT; // 提前锁定类型
	    node->children[0] = $1;
	    node->children[1] = $4;
	    node->lineno = pop_lineno();
	    $$ = node;
	  }
	| term { $$ = $1; }
	;

addop:
	  '+' { $$ = '+'; }
	| '-' { $$ = '-'; }
	;

term:
	  term mulop { push_lineno(yylineno); } factor {
	    Ast node = new_ast_node();
	    node->kind = Expr_Binary;
	    node->operator = $2;
	    node->type = IntT; // 提前锁定类型
	    node->children[0] = $1;
	    node->children[1] = $4;
	    node->lineno = pop_lineno();
	    $$ = node;
	  }
	| factor { $$ = $1; }
	;

mulop:
	  '*' { $$ = '*'; }
	| '/' { $$ = '/'; }
	;

factor:
	  '(' expression ')' { $$ = $2; }
	| var { $$ = $1; }
	| call { $$ = $1; }
	| NUM {
	    Ast node = new_ast_node();
	    node->kind = Expr_Const;
	    node->number = $1;
	    node->type = IntT; // 提前锁定类型
	    node->lineno = yylineno;
	    $$ = node;
	  }
	;

call:
	  ID { push_lineno(yylineno); } '(' args ')' {
	    Ast node = new_ast_node();
	    node->kind = Expr_Fun;
	    node->name = $1;
	    node->children[0] = $4;
	    node->lineno = pop_lineno();
	    $$ = node;
	  }
	;

args:
	  arg_list { $$ = $1; }
	| { $$ = NULL; }
	;

/* 左递归，从右往左逆序排列 */
arg_list:
	  arg_list ',' expression {
	    Ast node = $3;
	    node->sibling = $1;
	    $$ = node;
	  }
	| expression { $$ = $1; }
	;

%%

Ast parse() {
	yyparse();
	return final_tree;
}

