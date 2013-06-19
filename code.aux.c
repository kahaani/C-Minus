#include "globals.h"

static int emitLoc = 0 ;
static int highEmitLoc = 0;

void emit_comment(char* c) {
	printf("* %s\n", c);
}

void emit_RO(char* op, int r, int s, int t, char* c) {
	printf("%4d: %5s %d, %d, %d \t %s \n", emitLoc++, op, r, s, t, c);
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

void emit_RM(char* op, int r, int d, int s, char* c) {
	printf("%4d: %5s %d, %d(%d) \t %s \n", emitLoc++, op, r, d, s, c);
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

int emit_skip(int howMany) {
	int i = emitLoc;
	emitLoc += howMany;
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
	return i;
}

void emit_backup(int loc) {
	if(loc > highEmitLoc) error(Bug, "loc > highEmitLoc"); //emit_comment("BUG in emit_backup");
	emitLoc = loc;
}

void emit_restore(void) {
	emitLoc = highEmitLoc;
}

/******************************/

void pseudo_mov_reg(int dst, int src, int x, char* c) {
	emit_RM("LDA", dst, x, src, c);
}

void pseudo_mov_const(int dst, int x, char* c) {
	emit_RM("LDC", dst, x, ignore, c);
}

void pseudo_push(int reg, char* c) {
	if(reg == sp || reg == pc) {
		warning("push sp/pc");
	}
	pseudo_mov_reg(sp, sp, -1, c);
	emit_RM("ST", reg, 0, sp, "");
}

void pseudo_pop(int reg, char* c) {
	if(reg == sp) {
		warning("pop sp");
	}
	// why this order? think about "pop pc"
	pseudo_mov_reg(sp, sp, 1, c);
	emit_RM("LD", reg, -1, sp, "");
}

/******************************/

void pseudo_return(void) {
	emit_comment("-> [pseudo] return");
	pseudo_mov_reg(sp, bp, 0, "sp = bp");
	pseudo_pop(bp, "pop bp");
	pseudo_pop(pc, "pop pc");
	emit_comment("<- [pseudo] return");
}

void pseudo_fun_head(char* name) {
	FunInfo funinfo = lookup_funinfo(name);
	funinfo->address = emit_skip(0); // IMPORTANT: register address
	int size = funinfo->var_size;

	emit_comment("-> [pseudo] function head");
	pseudo_push(bp, "push bp");
	pseudo_mov_reg(bp, sp, 0, "bp = sp");
	pseudo_mov_reg(sp, sp, -size, "sp = sp - param_size");
	emit_comment("<- [pseudo] function head");
}

// note: unsafe ax
void pseudo_call(char* name) {
	FunInfo funinfo = lookup_funinfo(name);
	int address = funinfo->address;
	int size = funinfo->param_size;
	
	if(address < 0) {
		error(Bug, "address of funtion \"%s\" < 0", name);
	}

	emit_comment("-> [pseudo] call:");
	emit_comment(name);
	pseudo_mov_reg(ax, pc, 3, "ac = pc + 3 (next pc)");
	pseudo_push(ax, "push ax (return address)");
	pseudo_mov_const(pc, address, "pc = address (jmp)");

	//清理堆栈
	//pseudo_mov_reg(sp, sp, size, "sp = sp + param_size");
	emit_comment("<- [pseudo] call");
}

// return: ax
void pseudo_get_var_addr(Ast node) {
	Entry entry = lookup_entry(node->name);
	int offset = entry->offset;

	emit_comment("-> [pseudo] get var addr");
	switch(entry->kind) {
		case Global_Var:   emit_RM("LDA", ax, -offset,   gp, "ax = gp-offset"); break;
		case Compound_Var: emit_RM("LDA", ax, -offset-1, bp, "ax = bp-offset-1"); break;
		case Fun_Param:    emit_RM("LDA", ax, offset+2,  bp, "ax = bp+offset+2"); break;
		default: error(Bug, "unknown entry kind");
	}
	emit_comment("<- [pseudo] get var addr");
}

/*
// return: ax
void pseudo_get_var(Ast node) {
	Entry entry = lookup_entry(node->name);
	int offset = entry->offset;
	
	emit_comment("-> [pseudo] get var");
	emit_comment(node->name);
	
	if((entry->type == IntArrayT && node->type == IntArrayT) // common case: return address
		|| (entry->type == IntArrayT && node->type == IntT)
		// special case: pass array name as function argument, return address
	) {
		switch(entry->kind) {
			case Global_Var:   emit_RM("LDA", ax, -offset,   gp, "ax = gp-offset"); break;
			case Compound_Var: emit_RM("LDA", ax, -offset-1, bp, "ax = bp-offset-1"); break;
			case Fun_Param:    emit_RM("LDA", ax, offset+2,  bp, "ax = bp+offset+2"); break;
			default: error(Bug, "unknown entry kind");
		}
	} else if (entry->type == IntT && node->type == IntT) { // common case: retrun value
		switch(entry->kind) {
			case Global_Var:   emit_RM("LD", ax, -offset,   gp, "ax = data[gp-offset]"); break;
			case Compound_Var: emit_RM("LD", ax, -offset-1, bp, "ax = data[bp-offset-1]"); break;
			case Fun_Param:    emit_RM("LD", ax, offset+2,  bp, "ax = data[bp+offset+2]"); break;
			default: error(Bug, "unknown entry kind");
		}
	} else { 
		error(Bug, "type error of variable \"%s\" discoverd in pseudo_get_var()", node->name);
	}
	
	emit_comment("<- [pseudo] get var");
}
*/
/*
// parameter: ax
void pseudo_set_var(Ast node) {
	Entry entry = lookup_entry(node->name);
	int offset = entry->offset;
	assert(entry->type == IntT && node->type == IntT);

	emit_comment("-> [pseudo] set var");
	emit_comment(node->name);
	switch(entry->kind) {
		case Global_Var:   emit_RM("ST", ax, -offset,   gp, "data[gp-offset]"); break;
		case Compound_Var: emit_RM("ST", ax, -offset-1, bp, "data[bp-offset-1]"); break;
		case Fun_Param:    emit_RM("ST", ax, offset+2,  bp, "data[bp+offset+2]"); break;
		default: error(Bug, "unknown entry kind");
	}
	emit_comment("<- [pseudo] set var");
}
*/
