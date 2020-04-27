#include "global.h"
#include "scan.h"
#include "parser.h"
#include "semantic.h"
#include "symtab.h"

using namespace std;
#pragma warning(disable:4996)

FILE* sourceFile;
FILE* AST_File;

int g_lineNumber = 0;
_tToken g_token;

SymTab* g_symtab;

int main(int argc, char* argv[])
{
    sourceFile = fopen("Text.txt", "r");
    if (NULL == sourceFile) {
        cout << "source file open failed!\n";
    }
    else {
        cout << "source file open successfully!\n";
    }

    TreeNode* synaxtree = parser();

    g_symtab = initSimpleSymTable((char*)("Global"));
    semanticAnalyze(synaxtree);


    return 0;
}


