#include "globals.h"

static int donot_fetch_var_value = FALSE;

// return: ax
static void codegen_exp(Ast node) {
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

	case Expr_Assign: //TODO
		emit_comment("-> assign");

		donot_fetch_var_value = TRUE;
		codegen_exp(node->children[0]);
		pseudo_push(ax, "push ax");
		donot_fetch_var_value = FALSE;
		
		codegen_exp(node->children[1]);
		
		pseudo_pop(bx, "push bx");
		emit_RM("ST", ax);
		
		emit_comment("<- assign");
		break;

	case Expr_Binary: //TODO
		break;

	case Expr_Fun: //TODO
		break;

	case Expr_Var:
		emit_comment("-> var");
		pseudo_get_var_addr(node);
		if(node->type == IntArrayT) {
			pseudo_push(ax, "push ax");
			codegen_exp(node->children[0]);
			pseudo_pop(bx, "pop bx");
			emit_RO("SUB", ax, bx, ax, "ax = bx - ax (array offset)");
		} else {
			assert(node->type == IntT);
		}
		if(! donot_fetch_var_value) {
			emit_RM("LD", ax, 0, ax, "ax = data[ax]");
		}
		emit_comment("<- var");
/*		pseudo_get_var(node);
		if(node->type == IntArrayT) {
			pseudo_push(ax, "push ax");
			codegen_exp(node->children[0]);
			pseudo_pop(bx, "pop bx");
			emit_RO("SUB", ax, bx, ax, "ax = bx - ax (array offset)");
			emit_RM("LD", ax, 0, ax, "ax = data[ax]");
		} else {
			assert(node->type == IntT);
		}
*/		break;

	case Expr_Const:
		emit_comment("-> const");
		pseudo_mov_const(ax, node->number, "ax = const value");
		emit_comment("-> const");
		break;

	default:
		error(Bug, "unknown ast node kind");
	}
}

static void codegen_stmt(Ast node) {
	int saved_loc_1 = -1;
	int saved_loc_2 = -1;
	int current_loc = -1;

	while(node != NULL) {
		switch(node->kind) {

		case Decl_Var: // skip
			break;

		case Decl_Param: // skip
			break;

		case Decl_Fun:
			push_symtab(node->symtab);
			emit_comment("-> function:");
			emit_comment(node->name);

			pseudo_fun_head(node->name);
			codegen_stmt(node->children[1]); // recursion: function body
			pseudo_return();

			emit_comment("<- function");
			pop_symtab();
			break;

		case Stat_Compound:
			push_symtab(node->symtab);
			emit_comment("-> compound");
			codegen_stmt(node->children[0]); // recursion
			emit_comment("<- compound");
			pop_symtab();
			break;

		case Stat_Select:
			emit_comment("-> if");
			
			codegen_exp(node->children[0]); // value in ax

			saved_loc_1 = emit_skip(1);
			emit_comment("skip: jmp to else");
			
			codegen_stmt(node->children[1]); // recursion
			
			saved_loc_2 = emit_skip(1);
			emit_comment("skip: jmp to end");
			
			current_loc = emit_skip(0);
			emit_backup(saved_loc_1);
			emit_RM("JEQ", ax, current_loc, zero, "if ax == 0, pc = addr of else");
			emit_restore();
			
			codegen_stmt(node->children[2]); // recursion
			
			current_loc = emit_skip(0);
			emit_backup(saved_loc_2);
			pseudo_mov_const(pc, current_loc, "pc = addr of endif");
			emit_restore();
			emit_comment("<- if");
			break;

		case Stat_Iterate:
			emit_comment("-> while");
			
			saved_loc_1 = emit_skip(0);
			
			codegen_exp(node->children[0]);
			
			saved_loc_2 = emit_skip(1);
			emit_comment("skip: jmp to end");
			
			codegen_stmt(node->children[1]); // recursion
			
			pseudo_mov_const(pc, saved_loc_1, "pc = addr of while");
			
			current_loc = emit_skip(0);
			emit_backup(saved_loc_2);
			emit_RM("JNE", ax, current_loc, zero, "if ax != 0, pc = add of endwhile");
			emit_restore();
			
			emit_comment("<- while");
			break;

		case Stat_Return:
			emit_comment("-> return");
			if(node->type == IntT) {
				codegen_exp(node->children[0]); // return value now in ax
			} else {
				assert(node->type == VoidT);
			}
			pseudo_return();
			emit_comment("<- return");
			break;

		case Stat_Empty: // skip
			break;

		case Stat_Expression:
			emit_comment("-> expression statement");
			codegen_exp(node->children[0]);
			emit_comment("<- expression statement");
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

static void codegen_init(void) {
	int size = top_symtab()->size;

	emit_comment("-> init zero, gp, sp");
	pseudo_mov_const(zero, 0, "zero = 0 (noneed)");
	emit_RM("LD", gp, 0, zero, "gp = data[0] (maxaddress)");
	emit_RM("ST", zero, 0, zero, "data[0] = 0");
	pseudo_mov_reg(sp, gp, -size+1, "sp = gp - global_var_size + 1");
	//pseudo_mov_reg(sp, sp, 1, "sp = sp + 1");
	emit_comment("<- init zero, gp, sp");
}

static void codegen_prelude_input() {
	emit_comment("-> prelude function: input");
	pseudo_fun_head("input");
	emit_RO("IN", ax, ignore, ignore, "input ax");
	pseudo_return();
	emit_comment("<- prelude function: input");
}

static void codegen_prelude_output() {
	emit_comment("-> prelude function: output");
	pseudo_fun_head("output");
	emit_RM("LD", ax, 2, bp, "ax = data[bp+2] (param 0)");
	emit_RO("OUT", ax, ignore, ignore, "output ax");
	pseudo_return();
	emit_comment("<- prelude function: output");
}

void codegen_root(Ast root) {
	emit_comment("-> beginning of TM code");

	codegen_init();
	
	// jmp to main 改成 call main 后面跟 HALT
	emit_comment("skip: address of main()");
	int jmp_to_main = emit_skip(1);

	codegen_prelude_input();
	codegen_prelude_output();

	codegen_stmt(root);

	// backpatch add of main
	int current_loc = emit_skip(0);
	emit_backup(jmp_to_main);
	pseudo_mov_const(pc, current_loc, "backpatch: pc = main()");
	emit_restore();
	
	// for test
	pseudo_mov_const(ax, 1234, "ax = 1234");
	pseudo_push(ax, "push ax");
	pseudo_call("output");

	emit_comment("<- end of TM code");
}

