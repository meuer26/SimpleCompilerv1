%{
// https://www.quut.com/c/ANSI-C-grammar-y-1999.html

#include "stdio.h"
#include "parsetree.c"
#include "x64systemv.c"
#include "parserstack.h"


int yylex();
int yyparse();

void yyerror(const char *str) {
    //https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
    fprintf (stderr, "%s\n", str);
}


extern FILE *yyin;
ParserStack *parserStack;
ParserStack *parserStackReversed;
ParseTree *parseTree;

int parserStackSize = 0;
int symbolEntryIndex = 0;
char symbolName[50][20];
char symbolType[50][10];
int symbolLocation[50];

void insertSymbol(char *name, char *type, int location)
{
    strcpy(symbolName[symbolEntryIndex], name);
    strcpy(symbolType[symbolEntryIndex], type);
    symbolLocation[symbolEntryIndex] = location;
    symbolEntryIndex = symbolEntryIndex + 1;
}



int main(int argc, char *argv[])
{
    #if YYDEBUG == 1
    extern int yydebug;
    yydebug = 1;
    #endif
    
    yyin = fopen(argv[1], "r");
    parserStack = parserStackCreate();
    parserStackReversed = parserStackCreate();

    yyparse();

    FILE *prog = fopen("prog.s", "wb");

    dataSectionHeader(prog);
    textSectionHeader(prog);
    funcPrologue(prog, parserStackSize);

    while (parserStack->depth > 0)
    {
        printf("parserstack depth: %d\n", parserStack->depth);
        parseTree = parserStackPop(parserStack);
        parserStackPush(parserStackReversed, parseTree);

    }

    // I have to reverse the parsetree stack to get the codegen in the correct order
    while (parserStackReversed->depth > 0)
    {
        //parserStackSize = parserStackSize + 1;
        printf("parserstackReversed depth: %d\n", parserStackReversed->depth);
        parseTree = parserStackPop(parserStackReversed);
        funcCode(prog, parseTree, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
        parserStackSize = parserStackSize + 1;
        
    }

    printf("***** Symbol Table Dump *****\n");
    int x;
    for (x = 0; x < symbolEntryIndex; x++)
    {
        printf("--> ID: %d \tNAME: %s \tTYPE: %s \tLOCATION: %d\n", x, (char *) symbolName[x], (char *) symbolType[x], symbolLocation[x]);
    }

    sysExit(prog);
    fclose(prog);

}

%}

%token TOK_LBRACE TOK_ASM TOK_VOLATILE
%token TOK_RBRACE TOK_RETURN TOK_ADD TOK_EQUAL
%token TOK_LPAREN TOK_RPAREN TOK_SEMI
%token TOK_MUL TOK_SUB TOK_DIV TOK_LOGICAL_NEGATION
%token TOK_COUT TOK_STREAM_OUT

%union
{
    int number;
    char *string;
}

%token <number> TOK_UINT
%token <string> TOK_TYPE 
%token <string> TOK_IDENTIFIER
%token <string> TOK_STRING

%%

// grammar rules or productions

program: 
	function
        ;
function:
        function stmt
        |
        function TOK_TYPE TOK_IDENTIFIER TOK_LPAREN TOK_RPAREN TOK_LBRACE stmt_list TOK_RBRACE
        | /*support null */
        ;
stmt:
        TOK_SEMI
        |
        expr TOK_SEMI
        |
        TOK_RETURN expr TOK_SEMI
        |
        TOK_TYPE TOK_IDENTIFIER TOK_EQUAL expr TOK_SEMI
        {
            ParseTree *rint = parserStackPop(parserStack);
            insertSymbol((char *)$2, (char *)$1, parserStackSize);
            parserStackPush(parserStack, assign(rint));
            parserStackSize = parserStackSize + 1;
        }
        |
        TOK_COUT TOK_STREAM_OUT expr TOK_SEMI
        {
            ParseTree *rint = parserStackPop(parserStack);
            parserStackPush(parserStack, cout(rint));
            parserStackSize = parserStackSize + 1;
        }
        |
        TOK_ASM TOK_VOLATILE TOK_LPAREN TOK_STRING TOK_RPAREN TOK_SEMI
        {
            parserStackPush(parserStack, inlineAssembly($4));
            parserStackSize = parserStackSize + 1;
        }
        |
        TOK_LBRACE stmt_list TOK_RBRACE
        ;
stmt_list:
        stmt
        |
        stmt_list stmt
        ;
expr:
        value TOK_ADD expr
        {
            ParseTree *lint = parserStackPop(parserStack);
            ParseTree *rint = parserStackPop(parserStack);
            parserStackPush(parserStack, add(lint, rint));
            parserStackSize = parserStackSize + 1;
        }
        |
        value TOK_SUB expr
        {
            ParseTree *lint = parserStackPop(parserStack);
            ParseTree *rint = parserStackPop(parserStack);
            parserStackPush(parserStack, subtract(lint, rint));
            parserStackSize = parserStackSize + 1;
        }
        |
        value TOK_MUL expr
        {
            ParseTree *lint = parserStackPop(parserStack);
            ParseTree *rint = parserStackPop(parserStack);
            parserStackPush(parserStack, multiply(lint, rint));
            parserStackSize = parserStackSize + 1;
        }
        |
        value TOK_DIV expr
        {
            ParseTree *lint = parserStackPop(parserStack);
            ParseTree *rint = parserStackPop(parserStack);
            parserStackPush(parserStack, divide(lint, rint));
            parserStackSize = parserStackSize + 1;
        }
        |
        TOK_LOGICAL_NEGATION expr
        {
            ParseTree *rint = parserStackPop(parserStack);
            parserStackPush(parserStack, logicalNegation(rint));
            parserStackSize = parserStackSize + 1;

        }
        |
        value
        |
        TOK_LPAREN expr TOK_LPAREN
        ;
value:
        TOK_IDENTIFIER
        {
            parserStackPush(parserStack, stringType($1));
        }
        |
        number
        ;
number:
        TOK_UINT
        {
            printf("*******parser.y uint push value: %d\n", $1);
            parserStackPush(parserStack, intType($1));
        }
        ;