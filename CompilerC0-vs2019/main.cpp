#include "global.h"
#include "scan.h"
#include "parser.h"
#include "semantic.h"
#include "symtab.h"
#include "ir.h"
#include "asm.h"

using namespace std;
#pragma warning(disable:4996)

FILE* sourceFile;
FILE* AST_File;
FILE* IR_FILE;
FILE* ASM_FILE;

int g_lineNumber = 0;
_tToken g_token;

SymTab* g_symtab;

int main(int argc, char* argv[])
{
    sourceFile = fopen("Text01.txt", "r");
    if (NULL == sourceFile) {
        cout << "source file open failed!\n";
    }
    else {
        cout << "source file open successfully!\n";
    }

    TreeNode* synaxtree = parser();

    g_symtab = initSimpleSymTable((char*)("Global"));
    semanticAnalyze(synaxtree);

    
    IR_Analyze(synaxtree);
    IR_FILE = fopen("IR.txt", "w+");
    printIR();
    fclose(IR_FILE);

    ASM_FILE = fopen("asm.txt", "w+");
    genasm();


    return 0;
}


