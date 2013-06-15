%option noyywrap nodefault yylineno

%x COMMENT

%{

#include "globals.h"

%}

%%

"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>([^*]|\en)+|.
<COMMENT><<EOF>> { error(LexicalError, "unterminated comment"); return 0; }

"if"		{ return IF; }
"else"		{ return ELSE; }
"while"		{ return WHILE; }
"return"	{ return RETURN; }
"int"		{ return INT; }
"void"		{ return VOID; }

"+" |
"-" |
"*" |
"/" |
"=" |
";" |
"," |
"(" |
")" |
"[" |
"]" |
"{" |
"}" { return yytext[0]; }

"<"  { return LT; }
">"  { return GT; }
"<=" { return LE; }
">=" { return GE; }
"==" { return EQ; }
"!=" { return NE; }

[0-9]+					{ yylval.int_value = atoi(yytext);   return NUM; }
[_a-zA-Z][_a-zA-Z0-9]*	{ yylval.str_value = strdup(yytext); return ID; /* extension */ }

[ \t\n]		{ }

.			{ error(LexicalError, "illegal character \"%s\"", yytext); }

%%

