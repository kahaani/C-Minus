#ifndef CODE_AUX_H
#define CODE_AUX_H

#define ignore 0
#define zero 0

#define ax 1
#define bx 2
#define cx 3

#define bp 4
#define sp 5
#define gp 6
#define pc 7

void emit_comment(char* c);
void emit_RO(char* op, int r, int s, int t, char* c);
void emit_RM(char* op, int r, int d, int s, char* c);

int emit_skip(int howMany);
void emit_backup(int loc);
void emit_restore(void);

void pseudo_mov_reg(int dst, int src, int x, char* c);
void pseudo_mov_const(int dst, int x, char* c);
void pseudo_push(int reg, char* c);
void pseudo_pop(int reg, char* c);

void pseudo_return(void);
void pseudo_fun_head(char* name);
void pseudo_call(FunInfo funinfo);
void pseudo_get_var_addr(Ast node);
void pseudo_compare(char* command);

//void pseudo_get_var(Ast node);
//void pseudo_set_var(Ast node);

#endif /* CODE_AUX_H */
