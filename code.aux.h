#ifndef CODE_AUX_H
#define CODE_AUX_H

#define zero 0
#define ignore 0

#define ax 1
#define bx 2
#define cx 3

#define bp 4
#define sp 5
#define gp 6
#define pc 7

void emitComment(char* c);
void emitRO(char* op, int r, int s, int t, char* c);
void emitRM(char* op, int r, int d, int s, char* c);
void emitRM_Abs(char* op, int r, int a, char* c);

int emitSkip(int howMany);
void emitBackup(int loc);
void emitRestore();

void pseudo_mov_reg(int dst, int src, int x, char* c);
void pseudo_mov_const(int dst, int x, char* c);
void pseudo_push(int reg, char* c);
void pseudo_pop(int reg, char* c);

void pseudo_call(char* name);
void pseudo_fun_head(FunInfo funinfo);
void pseudo_return();

#endif /* CODE_AUX_H */
