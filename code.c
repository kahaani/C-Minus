#include "globals.h"

void codegen_exp(Ast node) {
	assert(node != NULL);

	switch(node->kind) {

	case Decl_Var:
	case Decl_Param:
	case Decl_Fun:
		error(Bug, "declaration ast node should not attend in codegen_exp()");

	case Stat_Compound:
	case Stat_Select:
	case Stat_Iterate:
	case Stat_Return:
	case Stat_Empty:
	case Stat_Expression:
		error(Bug, "statement ast node should not attend in codegen_exp()");

	case Expr_Assign:

	case Expr_Binary:

	case Expr_Fun:

	case Expr_Var:

	case Expr_Const:

	default:
		error(Bug, "unknown ast node kind");
	}
}

void codegen_stmt(Ast node) {
	while(node != NULL) {
		switch(node->kind) {

		case Decl_Var:
			break;

		case Decl_Param: // similar to Decl_Var
			break;

		case Decl_Fun:
			break;

		case Stat_Compound: // similar to Decl_Fun, except without funinfo
			break;

		case Stat_Select:
			break;

		case Stat_Iterate:
			break;

		case Stat_Return:
			break;

		case Stat_Empty:
			break;

		case Stat_Expression:
			break;

		case Expr_Assign:
		case Expr_Binary:
		case Expr_Fun:
		case Expr_Var:
		case Expr_Const:
			error(Bug, "expression ast node should not attend in codegen_stmt()");

		default:
			error(Bug, "unknown ast node kind");
		}

		node = node->sibling;
	}
}

void codegen_prelude_input() {
	FunInfo funinfo = lookup_funinfo("input");

	emitComment("-> prelude function: input");
	pseudo_fun_head(funinfo);
	emitRO("IN", ax, ignore, ignore, "input ax");
	pseudo_fun_return();
	emitComment("<- prelude function: input");
}

void codegen_prelude_output() {
	FunInfo funinfo = lookup_funinfo("output");

	emitComment("-> prelude function: output");
	pseudo_fun_head(funinfo);
	emitRM("LD", ax, 2, bp, "ax = data[bp+2] (param 1)");
	emitRO("OUT", ax, ignore, ignore, "output ax");
	pseudo_fun_return();
	emitComment("<- prelude function: output");
}

void codegen_root(Ast root) {
	emitComment("-> beginning of TM code");

	emitComment("-> init sp");
	// suppose: ax = 0
	emitRM("LD", sp, 0, ax, "sp = data[0] (maxaddress)");
	emitRM("ST", ax, 0, ax, "data[0] = 0");
	emitComment("<- init sp");

	// jmp to main
	emitComment("skip: address of main()");
	int jmp_to_main = emitSkip(1);

	codegen_prelude_input();
	codegen_prelude_output();

	codegen_stmt(root);

	// backpatch add of main
	int current_loc = emitSkip(0);
	emitBackup(jmp_to_main);
	pseudo_mov_const(pc, current_loc, "backpatch: pc = main()");
	emitRestore();
	
	pseudo_mov_const(ax, 1234, "ax = 1234");
	pseudo_push(ax, "push ax");
	pseudo_call(lookup_funinfo("output"));

	emitComment("<- end of TM code");
}

