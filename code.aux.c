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
	if(loc > highEmitLoc) error(Bug, "loc > highEmitLoc in emit_backup()");
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
void pseudo_call(FunInfo funinfo) {
	char* name = funinfo->name;
	int address = funinfo->address;
	int size = funinfo->param_size;
	
	if(address < 0) {
		error(Bug, "address of funtion \"%s\" < 0", name);
	}

	emit_comment("-> [pseudo] call");
	emit_comment(name);
	pseudo_mov_reg(ax, pc, 3, "ac = pc + 3 (next pc)"); // why 4? "push ax" capture 2 positions
	pseudo_push(ax, "push ax (return address)");
	pseudo_mov_const(pc, address, "pc = address (jmp)");

	pseudo_mov_reg(sp, sp, size, "sp = sp + param_size"); // next pc: clean stack (parameter)
	emit_comment("<- [pseudo] call");
}

// return: ax
void pseudo_get_var_addr(Ast node) {
	Entry entry = lookup_entry(node->name);
	int offset = entry->offset;

	emit_comment("-> [pseudo] get var addr");
	switch(entry->kind) {
		case Global_Var:
			emit_RM("LDA", ax, -offset, gp, "ax = gp-offset");
			break;
		case Compound_Var:
			emit_RM("LDA", ax, -offset-1, bp, "ax = bp-offset-1");
			break;
		case Fun_Param:
			if(entry->type == IntT) {
				emit_RM("LDA", ax, offset+2, bp, "ax = bp+offset+2");
			} else {
				assert(entry->type == IntArrayT); // the value here is an address
				emit_RM("LD", ax, offset+2, bp, "ax = data[bp+offset+2]");
			}
			break;
		default: error(Bug, "unknown entry kind");
	}
	emit_comment("<- [pseudo] get var addr");
}

// input: bx, ax
// output: ax
void pseudo_compare(char* command) {
	emit_RO("SUB", ax, bx, ax, "ax = bx - ax");
	emit_RM(command, ax, 2, pc, "conditional jmp: if true");
	pseudo_mov_const(ax, 0, "ax = 0 (false case)");
	pseudo_mov_reg(pc, pc, 1, "unconditional jmp");
	pseudo_mov_const(ax, 1, "ax = 1 (true case)");
}

