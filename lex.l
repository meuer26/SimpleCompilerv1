/* https://www.quut.com/c/ANSI-C-grammar-l-1999.html */
/* DMO no further mods needed */
%{
#include <stdio.h>
#include "y.tab.h"
%}

%%
"//"[^\n]*    { /* consume //-comment */ }
[ \t\n]+      { /* consume whitespace characters */ }


"return"      { return TOK_RETURN; }
"int"         { yylval.string=strdup(yytext); return TOK_TYPE; }
"main"        { return TOK_IDENTIFIER; }
"asm"		  { return TOK_ASM;}
"volatile"    { return TOK_VOLATILE;}
"cout"		  { return TOK_COUT;}
"<<"		  { return TOK_STREAM_OUT; }
"{"           { return TOK_LBRACE; }
"}"           { return TOK_RBRACE; }
"("           { return TOK_LPAREN; }
")"           { return TOK_RPAREN; }
"+"           { return TOK_ADD; }
"-"           { return TOK_SUB; }
"*"           { return TOK_MUL; }
"/"           { return TOK_DIV; }
"!"		      { return TOK_LOGICAL_NEGATION; }
";"           { return TOK_SEMI; }
"="			  { return TOK_EQUAL; }
[0-9]+        { yylval.number=atoi(yytext); return TOK_UINT; }
[a-zA-Z_]+	  { yylval.string=strdup(yytext); return TOK_IDENTIFIER; }
["]([^"]|\\(.|\n))*["] 	{ yylval.string=strdup(yytext); return TOK_STRING; }

%%

int yywrap() {
	return 1;
}