#include "globals.h"

static int emitLoc = 0 ;
static int highEmitLoc = 0;

void emitComment(char* c) {
	printf("* %s\n", c);
}

void emitRO(char* op, int r, int s, int t, char* c) {
	printf("%4d: %5s %d, %d, %d \t %s \n", emitLoc++, op, r, s, t, c);
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

void emitRM(char* op, int r, int d, int s, char* c) {
	printf("%4d: %5s %d, %d(%d) \t %s \n", emitLoc++, op, r, d, s, c);
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

int emitSkip(int howMany) {
	int i = emitLoc;
	emitLoc += howMany ;
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
	return i;
}

void emitBackup(int loc) {
	if(loc > highEmitLoc) emitComment("BUG in emitBackup");
	emitLoc = loc;
}

void emitRestore(void) {
	emitLoc = highEmitLoc;
}

void emitRM_Abs(char* op, int r, int a, char* c) {
	printf("%4d: %5s %d, %d(%d) \t %s \n", emitLoc, op, r, a-(emitLoc+1), pc, c);
	++emitLoc;
	if(highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

/******************************/

void pseudo_mov_reg(int dst, int src, int x, char* c) {
	emitRM("LDA", dst, x, src, c);
}

void pseudo_mov_const(int dst, int x, char* c) {
	emitRM("LDC", dst, x, ignore, c);
}

void pseudo_push(int reg, char* c) {
	if(reg == sp || reg == pc) {
		warning("push sp/pc");
	}
	pseudo_mov_reg(sp, sp, -1, c);
	emitRM("ST", reg, 0, sp, "");
}

void pseudo_pop(int reg, char* c) {
	if(reg == sp) {
		warning("pop sp");
	}
	// why this order? think about "pop pc"
	pseudo_mov_reg(sp, sp, 1, c);
	emitRM("LD", reg, -1, sp, "");
}

/******************************/

// note: unsafe ax
void pseudo_call(char* name) {
	FunInfo funinfo = lookup_funinfo(name);

	emitComment("-> call");
	pseudo_mov_reg(ax, pc, 3, "ac = pc + 3");
	pseudo_push(ax, "push ax");
	pseudo_mov_const(pc, funinfo->address, "pc = address");
	//清理堆栈
	emitComment("<- call");
}

void pseudo_fun_head(FunInfo funinfo) {
	funinfo->address = emitSkip(0); // register address
	int size = funinfo->var_size;

	emitComment("-> function head");
	pseudo_push(bp, "push bp");
	pseudo_mov_reg(bp, sp, 0, "bp = sp");
	pseudo_mov_reg(sp, sp, -size, "sp = sp - param_size");
	emitComment("<- function head");
}

void pseudo_return(void) {
	emitComment("-> return");
	pseudo_mov_reg(sp, bp, 0, "sp = bp");
	pseudo_pop(bp, "pop bp");
	pseudo_pop(pc, "pop pc");
	emitComment("-> return");
}

void pseudo_get_var(void) {

}

void pseudo_set_var(void) {

}

