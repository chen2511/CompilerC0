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
int g_errorNum = 0;
_tToken g_token;

SymTab* g_symtab;


string sourcefilename = "./Input/Test";
string asmfilename = "./Output/ASM";
string astfilename = "./Output/AST";
string irfilename = "./Output/IR";

void processFileName(int index) {
    char num[5];
    itoa(index, num, 10);

    sourcefilename += num;
    sourcefilename += ".c0";

    asmfilename += num;
    asmfilename += ".txt";

    astfilename += num;
    astfilename += ".txt";

    irfilename += num;
    irfilename += ".txt";
}

int main(int argc, char* argv[])
{
    processFileName(11);
        
    sourceFile = fopen(sourcefilename.c_str(), "r");
    AST_File = fopen(astfilename.c_str(), "w+");
    if (NULL == sourceFile) {
        cout << "source file open failed!\n";
        exit(-1);
    }
    else {
        cout << "source file open successfully!\n";
    }

    // 语法分析，生成抽象语法树
    TreeNode* synaxtree = parser();

    //// 有错误，不继续编译，退出
    //if (g_errorNum) {
    //    exit(-1);
    //}

    // 语义分析、构造符号表
    g_symtab = initSimpleSymTable((char*)("Global"));
    semanticAnalyze(synaxtree);

    // 有错误，不继续编译，退出
    if (g_errorNum) {
        printf("一共有 %d 个错误\n", g_errorNum);
        exit(-1);
    }

    // 生成四元式
    IR_Analyze(synaxtree);
    IR_FILE = fopen(irfilename.c_str(), "w+");

    // 优化
    if (OPTIMIZE_SWITCH) {
        optimize();
    }

    // 输出四元式
    printIR();
    fclose(IR_FILE);

    // 生成汇编代码
    ASM_FILE = fopen(asmfilename.c_str(), "w+");
    genasm();

    


    return 0;
}


