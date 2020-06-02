#include "global.h"
#include "scan.h"
#include "parser.h"
#include "semantic.h"
#include "symtab.h"
#include "ir.h"
#include "asm.h"
#include "opt.h"

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
    sourceFile = fopen("./Input/Text08.txt", "r");
    if (NULL == sourceFile) {
        cout << "source file open failed!\n";
    }
    else {
        cout << "source file open successfully!\n";
    }

    TreeNode* synaxtree = parser();

    g_symtab = initSimpleSymTable((char*)("Global"));
    semanticAnalyze(synaxtree);

    // 生成四元式
    IR_Analyze(synaxtree);
    IR_FILE = fopen("./Output/IR.txt", "w+");

    // 优化
    if (OPTIMIZE_SWITCH) {
        optimize();
    }

    // 输出四元式
    printIR();
    fclose(IR_FILE);

    // 生成汇编代码
    ASM_FILE = fopen("./Output/asm.txt", "w+");
    genasm();

    


    return 0;
}


