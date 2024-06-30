#include <stdlib.h>
#include <string.h>

#ifndef COMPILER_PARSETREE
#define COMPILER_PARSETREE

typedef enum { INT, STRING, BINOP, UNOP, INLINE_ASSEMBLY } ParseTreeType;
typedef enum { ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION } ParseTreeBinOp;
typedef enum { LOGICALNEGATION, ASSIGN, COUT } ParseTreeUnOp;

typedef struct parseTree ParseTree;

typedef struct BinOpExpr {
    ParseTreeBinOp BinOpType;
    ParseTree *lint;
    ParseTree *rint;
} BinOpExpr;

typedef struct UnOpExpr {
    ParseTreeUnOp UnOpType;
    ParseTree *rint;
} UnOpExpr;

struct parseTree {
    ParseTreeType type;
    union {
        int constantValue;
        char *string;
        BinOpExpr *binExpr;
        UnOpExpr *unExpr;
    };
};


ParseTree *intType(int constantValue) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    parseTree->type = INT;
    parseTree->constantValue = constantValue;
    return parseTree;
}

ParseTree *stringType(char *string) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    parseTree->type = STRING;
    parseTree->string = string;
    return parseTree;
}

ParseTree *inlineAssembly(char *string) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    parseTree->type = INLINE_ASSEMBLY;
    parseTree->string = string;
    return parseTree;
}

ParseTree *add(ParseTree *lint, ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    BinOpExpr *binOpExpr = malloc(sizeof(binOpExpr));
    binOpExpr->BinOpType = ADDITION;
    binOpExpr->lint = lint;
    binOpExpr->rint = rint;
    parseTree->type = BINOP;
    parseTree->binExpr = binOpExpr;
    return parseTree;
}

ParseTree *subtract(ParseTree *lint, ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    BinOpExpr *binOpExpr = malloc(sizeof(binOpExpr));
    binOpExpr->BinOpType = SUBTRACTION;
    binOpExpr->lint = lint;
    binOpExpr->rint = rint;
    parseTree->type = BINOP;
    parseTree->binExpr = binOpExpr;
    return parseTree;
}

ParseTree *multiply(ParseTree *lint, ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    BinOpExpr *binOpExpr = malloc(sizeof(binOpExpr));
    binOpExpr->BinOpType = MULTIPLICATION;
    binOpExpr->lint = lint;
    binOpExpr->rint = rint;
    parseTree->type = BINOP;
    parseTree->binExpr = binOpExpr;
    return parseTree;
}

ParseTree *divide(ParseTree *lint, ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    BinOpExpr *binOpExpr = malloc(sizeof(binOpExpr));
    binOpExpr->BinOpType = DIVISION;
    binOpExpr->lint = lint;
    binOpExpr->rint = rint;
    parseTree->type = BINOP;
    parseTree->binExpr = binOpExpr;
    return parseTree;
}

ParseTree *logicalNegation(ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    UnOpExpr *unOpExpr = malloc(sizeof(unOpExpr));
    unOpExpr->UnOpType = LOGICALNEGATION;
    unOpExpr->rint = rint;
    parseTree->type = UNOP;
    parseTree->unExpr = unOpExpr;
    return parseTree;
}

ParseTree *assign(ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    UnOpExpr *unOpExpr = malloc(sizeof(unOpExpr));
    unOpExpr->UnOpType = ASSIGN;
    unOpExpr->rint = rint;
    parseTree->type = UNOP;
    parseTree->unExpr = unOpExpr;
    return parseTree;
}

ParseTree *cout(ParseTree *rint) {
    ParseTree *parseTree = malloc(sizeof(parseTree));
    UnOpExpr *unOpExpr = malloc(sizeof(unOpExpr));
    unOpExpr->UnOpType = COUT;
    unOpExpr->rint = rint;
    parseTree->type = UNOP;
    parseTree->unExpr = unOpExpr;
    return parseTree;
}


#endif