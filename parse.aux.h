#ifndef PARSE_AUX_H
#define PARSE_AUX_H

Ast new_ast_node();
void print_ast_root(Ast root);

void push_lineno(int x);
int pop_lineno();

#endif /* PARSE_AUX_H */
