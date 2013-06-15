#include "globals.h"

int main() {
#if TRACE_SCAN
	trace_scan();
#else /* TRACE_SCAN */
	
	#if YYDEBUG
	extern int yydebug;
	yydebug = TRUE;
	#endif

	Ast ast = parse();

	#if TRACE_PARSE
	print_ast_root(ast);
	#endif

	build_symtab_root(ast);
	#if TRACE_SYMTAB
	print_funlist();
	print_symtab_root(ast);
	#endif

#endif /* TRACE_SCAN */

	return 0;
}

