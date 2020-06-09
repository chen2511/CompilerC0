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
    }
    else {
        cout << "source file open successfully!\n";
    }

    // �﷨���������ɳ����﷨��
    TreeNode* synaxtree = parser();

    //// �д��󣬲��������룬�˳�
    //if (g_errorNum) {
    //    exit(-1);
    //}

    // ���������������ű�
    g_symtab = initSimpleSymTable((char*)("Global"));
    semanticAnalyze(synaxtree);

    // �д��󣬲��������룬�˳�
    if (g_errorNum) {
        printf("һ���� %d ������\n", g_errorNum);
        exit(-1);
    }

    // ������Ԫʽ
    IR_Analyze(synaxtree);
    IR_FILE = fopen(irfilename.c_str(), "w+");

    // �Ż�
    if (OPTIMIZE_SWITCH) {
        optimize();
    }

    // �����Ԫʽ
    printIR();
    fclose(IR_FILE);

    // ���ɻ�����
    ASM_FILE = fopen(asmfilename.c_str(), "w+");
    genasm();

    


    return 0;
}


