#include "global.h"
#include "scan.h"
#include "parser.h"

using namespace std;
#pragma warning(disable:4996)

FILE* sourceFile;

int g_lineNumber = 0;
Token g_token;


int main(int argc, char* argv[])
{
    sourceFile = fopen("Text.txt", "r");
    if (NULL == sourceFile) {
        cout << "source file open failed!\n";
    }
    else {
        cout << "source file open successfully!\n";
    }

    // �ʷ���������
    //do {
    //    getNextToken();
    //    if (!EOF_flag)
    //        cout << " token type: " << token.opType << " value: " << token.value << endl;
    //} while (!EOF_flag);

    parser();

    

    return 0;
}


