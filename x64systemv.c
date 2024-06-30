//https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf
//sticking to Intel syntax and will use NASM to assemble

#include "stdio.h"
#include "parsetree.c"
#include <string.h>

int funcCodeSymbolIndex = 0;

void dataSectionHeader(FILE *prog) {
    fprintf(prog, "section .data\n\n");
}

void textSectionHeader(FILE *prog) {
    fprintf(prog, "section .text\n\n");
    fprintf(prog, "    global _start\n\n");
    fprintf(prog, "_start:\n");
}

void funcPrologue(FILE *prog, int parserStackSize) {
    //prepare a generic 64-byte stack frame
    fprintf(prog, "    push rbp\n");
    fprintf(prog, "    mov rbp, rsp\n");
    fprintf(prog, "    sub rsp, %d\n\n", (parserStackSize) * 12 ); //prepare a generic 64-byte stack frame
}

void sysExit(FILE *prog) {
    fprintf(prog, "\n    mov rax, 60\n");
    fprintf(prog, "    syscall\n");
}

void returnValue(FILE *prog) {
    fprintf(prog, "    mov eax, edi\n");
}

void funcCode(FILE *prog, ParseTree *parseTree, int parserStackSize, char symbolName[][20], char symbolType[][10], int symbolLocation[], int symbolEntryIndex) {

    if (parseTree->type == INT) {
        printf("x64systemv.c constantValue: %d\n", parseTree->constantValue);
        fprintf(prog, "    mov edi, %d\n", parseTree->constantValue);

    
    }  
    else if (parseTree->type == STRING) {
        printf("x64systemv.c stringValue: %s\n", parseTree->string);

        for(int x = 0; x < symbolEntryIndex; ++x)
        {
            if(!strcmp(symbolName[x], parseTree->string))
            {
                printf("x64systemv.c found variable in symbol table: %d\n", x);
                fprintf(prog, "    mov edi, [rbp-%d]\n", symbolLocation[x]);
            }
        }

    }  
    else if (parseTree->type == INLINE_ASSEMBLY) {
        
        printf("x64systemv.c Inline Assembly Value: %s\n", parseTree->string);
        char *inlineAssemblyString = parseTree->string;
        inlineAssemblyString[strlen(inlineAssemblyString)-1] = 0; //Remove the last character
        inlineAssemblyString++; // Move the pointer forward to remove first character

        fprintf(prog, "%s\n", inlineAssemblyString);

    }  
    else if (parseTree->type == BINOP) {
        BinOpExpr *binOpExpr = parseTree->binExpr;

        if (binOpExpr->BinOpType == ADDITION) {

            funcCode(prog, binOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov [rbp-%d], rdi\n", (parserStackSize) * 4 );

            funcCode(prog, binOpExpr->lint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    adc edi, [rbp-%d]\n", (parserStackSize) * 4 );
            fprintf(prog, "    mov [rbp-%d], edi\n\n", ((parserStackSize) * 4 ) + 4);

            returnValue(prog);
        }
        else if (binOpExpr->BinOpType == SUBTRACTION) {
            
            funcCode(prog, binOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov [rbp-%d], edi\n", (parserStackSize) * 4 );

            funcCode(prog, binOpExpr->lint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    sbb edi, [rbp-%d]\n", (parserStackSize) * 4 );
            fprintf(prog, "    mov [rbp-%d], edi\n\n", ((parserStackSize) * 4 ) + 4);

            returnValue(prog);
        }
        else if (binOpExpr->BinOpType == MULTIPLICATION) {
            
            funcCode(prog, binOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov [rbp-%d], edi\n", (parserStackSize) * 4 );

            funcCode(prog, binOpExpr->lint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    imul edi, [rbp-%d]\n", (parserStackSize) * 4 );   //signed mul for future
            fprintf(prog, "    mov [rbp-%d], edi\n\n", ((parserStackSize) * 4 ) + 4);

            returnValue(prog);
        }
        else if (binOpExpr->BinOpType == DIVISION) {
            
            funcCode(prog, binOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov [rbp-%d], edi\n", (parserStackSize) * 4 );

            funcCode(prog, binOpExpr->lint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov eax, edi\n");
            fprintf(prog, "    mov ecx, [rbp-%d]\n", (parserStackSize) * 4 );
            fprintf(prog, "    idiv ecx\n");    // signed div for future
            fprintf(prog, "    mov [rbp-%d], ecx\n\n", ((parserStackSize) * 4 ) + 4);

            returnValue(prog);
        }

    } else if (parseTree->type == UNOP) {
        UnOpExpr *unOpExpr = parseTree->unExpr;

        if (unOpExpr->UnOpType == LOGICALNEGATION) {
            
            funcCode(prog, unOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov [rbp-%d], edi\n", (parserStackSize) * 4 );
            fprintf(prog, "    cmp edi, 0x0\n");
            fprintf(prog, "    jne done\n");
            fprintf(prog, "    mov [rbp-%d], dword 0x1\n",  ((parserStackSize) * 4 ) + 4);
            fprintf(prog, "done:\n");
            fprintf(prog, "    mov rax, [rbp-%d]\n",  ((parserStackSize) * 4 ) + 4);

        }

        else if (unOpExpr->UnOpType == ASSIGN) {
            
            funcCode(prog, unOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "%s:\n", symbolName[funcCodeSymbolIndex]);
            fprintf(prog, "    mov [rbp-%d], edi\n\n", (parserStackSize) * 4 );
            symbolLocation[funcCodeSymbolIndex] = (parserStackSize * 4);
            printf("CodeGen --> symbol location for ID:%d = %d\n", funcCodeSymbolIndex, (parserStackSize * 4));
            funcCodeSymbolIndex = funcCodeSymbolIndex + 1;
        }

        else if (unOpExpr->UnOpType == COUT) {
            
            // this will only print out an answer of three digits or less 
            // because I run out of stack space since each digit is pushed using
            // 8 bytes of stack space for each digit
            funcCode(prog, unOpExpr->rint, parserStackSize, symbolName, symbolType, symbolLocation, symbolEntryIndex);
            fprintf(prog, "    mov edi, [rbp-%d]\n", ((parserStackSize) * 4) + 4);
            fprintf(prog, "    mov eax, edi\n");
            fprintf(prog, "    xor rcx, rcx\n");
            fprintf(prog, "    xor r9, r9\n");
            fprintf(prog, "    lea r9, [rbp-%d]\n", ((parserStackSize) * 4) + 4);
            fprintf(prog, "uToString:    \n");
            fprintf(prog, "    xor rdx, rdx\n");
            fprintf(prog, "    mov ebx, 10\n");
            fprintf(prog, "    idiv ebx\n");
            fprintf(prog, "    push rdx\n");
            fprintf(prog, "    inc rcx\n");
            fprintf(prog, "    test eax, eax\n");
            fprintf(prog, "    je uToStringComplete\n");
            fprintf(prog, "    jmp uToString\n");
            fprintf(prog, "uToStringComplete:    \n");
            fprintf(prog, "    pop rax\n");
            fprintf(prog, "    lea rax, [rax+'0']\n");
            fprintf(prog, "    mov [r9], rax\n"); 
            fprintf(prog, "    dec rcx\n");
            fprintf(prog, "    inc r9\n");
            fprintf(prog, "    cmp rcx, 0\n");
            fprintf(prog, "    jg uToStringComplete\n");
            fprintf(prog, "    lea rsi, [rbp-%d]\n", ((parserStackSize) * 4) + 4);           
            fprintf(prog, "    mov rax, 1\n");
            fprintf(prog, "    mov rdi, 1\n");
            fprintf(prog, "    mov rdx, 3\n");
            fprintf(prog, "    syscall\n");

        }

    }
}
