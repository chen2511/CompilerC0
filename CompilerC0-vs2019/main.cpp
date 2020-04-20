#include "global.h"
#include "scan.h"

using namespace std;
#pragma warning(disable:4996)

FILE* sourceFile;

int g_lineNumber = 0;
Token token;


int main(int argc, char* argv[])
{
    sourceFile = fopen("Text.txt", "r");


    do {
        getNextToken();
        if (!EOF_flag)
            cout << " token type: " << token.opType << " value: " << token.value << endl;
    } while (!EOF_flag);

    return 0;
}


