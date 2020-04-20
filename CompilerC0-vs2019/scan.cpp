#include "global.h"
#include "scan.h"
#pragma warning(disable:4996)

using namespace std;

//在识别出标识符之后，验证是否关键字
const char* reservedWords[] = {
    "case", "char", "const", "default", "else",
    "false", "for", "if", "int", "main",
    "printf", "return", "scanf", "switch", "true",
    "void", "while"
};

//缓冲数组
char g_lineBuf[4097];
//在buf中的指针；
int g_lexBegin = 0;
//forward 指向下一个将要读取的字符；lexBegin表示该token开始的位置
int g_forward = 0;
//数组大小
int g_bufSize = 0;
//是否文件末尾
bool EOF_flag = false;

//状态机的所有状态
typedef enum {
    STATE_START, STATE_NUM, STATE_ID, STATE_CHAR, STATE_STRING, STATE_DONE
}StateType;

char getNextChar();

void backSpace()
{
    g_forward--;
}

//将值拷贝到token的value中
void copyValue(string tmpstr)
{
    int i;
    int len = tmpstr.length();
    for (i = 0; i < tmpstr.length(); i++)
        token.value[i] = tmpstr.at(i);
    token.value[i] = '\0';
}

//查看是否为关键字，若是，改变token Type
void compareWithKeyWord()
{
    for (int i = 0; i < RESERVEDWORD_NUM; i++) {
        if (0 == strcmp(token.value, reservedWords[i])) {
            token.opType = (TokenType)i;
            break;
        }
    }
}

//从文件中逐步读入字符，输出token
void getNextToken() {
    string tmpstr;
    char ch;
    StateType state = STATE_START;

    while (STATE_DONE != state) {
        ch = getNextChar();
        switch (state)
        {
        case STATE_START: {      /////////////////////开始状态
            if (' ' == ch || '\t' == ch || '\n' == ch) {

            }
            else if (isdigit(ch)) {                  //数字
                if ('0' == ch) {
                    state = STATE_DONE;
                    tmpstr += ch;
                    token.opType = TokenType::NUM;
                }
                else {
                    state = STATE_NUM;
                    tmpstr += ch;
                }
            }
            else if (isalpha(ch) || '_' == ch) {
                state = STATE_ID;    //标识符
                tmpstr += ch;
            }
            else if ('\'' == ch) {              //字符
                state = STATE_CHAR;
            }
            else if ('"' == ch) {               //字符串
                state = STATE_STRING;
            }
            else {                              //一个或两个字符就到终态
                state = STATE_DONE;
                tmpstr += ch;
                switch (ch) {
                case '+':                       // +
                    token.opType = TokenType::PLUS;
                    break;
                case '-':                       // -
                    token.opType = TokenType::MINU;
                    break;
                case '*':                       // *
                    token.opType = TokenType::MULT;
                    break;
                case '/':                       // /
                    token.opType = TokenType::DIV;
                    break;
                case '&':                       // &&
                    ch = getNextChar();
                    if ('&' == ch) {
                        token.opType = TokenType::AND;
                        tmpstr += ch;
                    }
                    else {
                        cout << "lex error: &&\n";
                    }
                    break;
                case '|':                       //||
                    ch = getNextChar();
                    if ('|' == ch) {
                        token.opType = TokenType::OR;
                        tmpstr += ch;
                    }
                    else {
                        cout << "lex error: ||\n";
                    }
                    break;
                case '!':                       // ! !=
                    ch = getNextChar();
                    if ('=' == ch) {
                        token.opType = TokenType::NEQ;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        token.opType = TokenType::NOT;
                    }
                    break;
                case '<':                       // < <=
                    ch = getNextChar();
                    if ('=' == ch) {
                        token.opType = TokenType::LEQ;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        token.opType = TokenType::LSS;
                    }
                    break;
                case '>':                       // > >=
                    ch = getNextChar();
                    if ('=' == ch) {
                        token.opType = TokenType::GEQ;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        token.opType = TokenType::GRE;
                    }
                    break;
                case '=':                       // = ==
                    ch = getNextChar();
                    if ('=' == ch) {
                        token.opType = TokenType::EQL;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        token.opType = TokenType::ASSIGN;
                    }
                    break;
                case ':':                       // :
                    token.opType = TokenType::COLON;
                    break;
                case ',':
                    token.opType = TokenType::COMMA;
                    break;
                case ';':
                    token.opType = TokenType::SEMICOLON;
                    break;
                case '{':
                    token.opType = TokenType::LBRACE;
                    break;
                case '}':
                    token.opType = TokenType::RBRACE;
                    break;
                case '[':
                    token.opType = TokenType::LBRACKET;
                    break;
                case ']':
                    token.opType = TokenType::RBRACKET;
                    break;
                case '(':
                    token.opType = TokenType::LPARENTHES;
                    break;
                case ')':
                    token.opType = TokenType::RPARENTHES;
                    break;
                }
            }
            break;
        }
        case STATE_CHAR:
            if (isdigit(ch) || isalpha(ch) || '+' == ch || '-' == ch || '*' == ch || '/' == ch) {
                tmpstr += ch;
            }
            else {
                cout << "lex error:illegal character\n";
            }

            ch = getNextChar();
            state = STATE_DONE;
            if ('\'' != ch) {
                backSpace();
                cout << "lex error:not '\n";
            }

            break;

        case STATE_ID:
            if (isdigit(ch) || isalpha(ch) || '_' == ch) {
                tmpstr += ch;
            }
            else {
                state = STATE_DONE;
                backSpace();
                token.opType = TokenType::IDEN;

            }
            break;

        case STATE_NUM:
            if (isdigit(ch)) {
                tmpstr += ch;
            }
            else {
                state = STATE_DONE;
                backSpace();
                token.opType = TokenType::NUM;
            }
            break;

        case STATE_STRING:
            if ('"' == ch) {
                state = STATE_DONE;
                token.opType = TokenType::STRING;
            }
            else if (32 <= ch && 126 >= ch) {
                tmpstr += ch;
            }
            else {
                cout << "lex error: illegal character in string\n";
            }
            break;

        default:
            break;
        }
    }
    if (TokenType::IDEN == token.opType) {             //查询关键字表

    }

    copyValue(tmpstr);

    compareWithKeyWord();
}



//返回下一个字符
char getNextChar() {
    if (g_bufSize <= g_forward) {
        //如果该buf读取完，读取下一行
        g_lineNumber++;
        if (fgets(g_lineBuf, 4096, sourceFile)) {
            //成功读取
            g_bufSize = strlen(g_lineBuf);
            g_forward = 0;
        }
        else {
            //文件结尾
            EOF_flag = true;
            return EOF;
        }
    }
    //返回下一个字符
    return g_lineBuf[g_forward++];
}





