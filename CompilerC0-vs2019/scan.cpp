#include "global.h"
#include "scan.h"
#pragma warning(disable:4996)

using namespace std;

//��ʶ�����ʶ��֮����֤�Ƿ�ؼ���
const char* reservedWords[] = {
    "char", "const",  "else", "false", "for", 
    "if", "int", "main", "printf", "return", 
    "scanf", "true", "void", "while"
};

//��������
char g_lineBuf[4097];
//��buf�е�ָ�룻
int g_lexBegin = 0;
//forward ָ����һ����Ҫ��ȡ���ַ���lexBegin��ʾ��token��ʼ��λ��
int g_forward = 0;
//�����С
int g_bufSize = 0;
//�Ƿ��ļ�ĩβ
bool EOF_flag = false;

//״̬��������״̬
typedef enum {
    STATE_START, STATE_NUM, STATE_ID, STATE_CHAR, STATE_STRING, STATE_DONE
}StateType;

char getNextChar();

void backSpace()
{
    g_forward--;
}

//��ֵ������token��value��
void copyValue(string tmpstr)
{
    int i;
    int len = tmpstr.length();
    for (i = 0; i < tmpstr.length(); i++)
        g_token.value[i] = tmpstr.at(i);
    g_token.value[i] = '\0';
}

//�鿴�Ƿ�Ϊ�ؼ��֣����ǣ��ı�token Type
void compareWithKeyWord()
{
    for (int i = 0; i < RESERVEDWORD_NUM; i++) {
        if (0 == strcmp(g_token.value, reservedWords[i])) {
            g_token.opType = (TokenType)i;
            break;
        }
    }
}

//���ļ����𲽶����ַ������token
void getNextToken() {
    string tmpstr;
    char ch;
    StateType state = STATE_START;

    g_lexBegin = g_forward;

    while (STATE_DONE != state) {
        ch = getNextChar();
        if (EOF == ch) {
            g_token.opType = END;
            return;
        }

        switch (state)
        {
        case STATE_START: {      /////////////////////��ʼ״̬
            if (' ' == ch || '\t' == ch || '\n' == ch) {

            }
            else if (isdigit(ch)) {                  //����
                if ('0' == ch) {
                    state = STATE_DONE;
                    tmpstr += ch;
                    g_token.opType = TokenType::NUM;
                }
                else {
                    state = STATE_NUM;
                    tmpstr += ch;
                }
            }
            else if (isalpha(ch) || '_' == ch) {
                state = STATE_ID;    //��ʶ��
                tmpstr += ch;
            }
            else if ('\'' == ch) {              //�ַ�
                state = STATE_CHAR;
            }
            else if ('"' == ch) {               //�ַ���
                state = STATE_STRING;
            }
            else {                              //һ���������ַ��͵���̬
                state = STATE_DONE;
                tmpstr += ch;
                switch (ch) {
                case '+':                       // +
                    g_token.opType = TokenType::PLUS;
                    break;
                case '-':                       // -
                    g_token.opType = TokenType::MINU;
                    break;
                case '*':                       // *
                    g_token.opType = TokenType::MULT;
                    break;
                case '/':                       // /
                    g_token.opType = TokenType::DIV;
                    break;
                case '&':                       // &&
                    ch = getNextChar();
                    if ('&' == ch) {
                        g_token.opType = TokenType::AND;
                        tmpstr += ch;
                    }
                    else {
                        cout << "lex error: &&\n";
                    }
                    break;
                case '|':                       //||
                    ch = getNextChar();
                    if ('|' == ch) {
                        g_token.opType = TokenType::OR;
                        tmpstr += ch;
                    }
                    else {
                        cout << "lex error: ||\n";
                    }
                    break;
                case '!':                       // ! !=
                    ch = getNextChar();
                    if ('=' == ch) {
                        g_token.opType = TokenType::NEQ;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        g_token.opType = TokenType::NOT;
                    }
                    break;
                case '<':                       // < <=
                    ch = getNextChar();
                    if ('=' == ch) {
                        g_token.opType = TokenType::LEQ;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        g_token.opType = TokenType::LSS;
                    }
                    break;
                case '>':                       // > >=
                    ch = getNextChar();
                    if ('=' == ch) {
                        g_token.opType = TokenType::GEQ;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        g_token.opType = TokenType::GRE;
                    }
                    break;
                case '=':                       // = ==
                    ch = getNextChar();
                    if ('=' == ch) {
                        g_token.opType = TokenType::EQL;
                        tmpstr += ch;
                    }
                    else {
                        backSpace();
                        g_token.opType = TokenType::ASSIGN;
                    }
                    break;
                case ',':
                    g_token.opType = TokenType::COMMA;
                    break;
                case ';':
                    g_token.opType = TokenType::SEMICOLON;
                    break;
                case '{':
                    g_token.opType = TokenType::LBRACE;
                    break;
                case '}':
                    g_token.opType = TokenType::RBRACE;
                    break;
                case '[':
                    g_token.opType = TokenType::LBRACKET;
                    break;
                case ']':
                    g_token.opType = TokenType::RBRACKET;
                    break;
                case '(':
                    g_token.opType = TokenType::LPARENTHES;
                    break;
                case ')':
                    g_token.opType = TokenType::RPARENTHES;
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
            // ��һ��bug������������Ǽ��ˡ�����
            g_token.opType = TokenType::LETTER;
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
                g_token.opType = TokenType::IDEN;

            }
            break;

        case STATE_NUM:
            if (isdigit(ch)) {
                tmpstr += ch;
            }
            else {
                state = STATE_DONE;
                backSpace();
                g_token.opType = TokenType::NUM;
            }
            break;

        case STATE_STRING:
            if ('"' == ch) {
                state = STATE_DONE;
                g_token.opType = TokenType::STRING;
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
    if (TokenType::IDEN == g_token.opType) {             //��ѯ�ؼ��ֱ�

    }

    copyValue(tmpstr);
    if (g_token.opType == TokenType::IDEN) {
        compareWithKeyWord();
    }
    
}

//������һ���ַ�
char getNextChar() {
    if (g_bufSize <= g_forward) {
        //�����buf��ȡ�꣬��ȡ��һ��
        g_lineNumber++;
        if (fgets(g_lineBuf, 4096, sourceFile)) {
            //�ɹ���ȡ
            g_bufSize = strlen(g_lineBuf);
            g_forward = 0;

            //֮ǰ����������Ϊ0��
            g_lexBegin = 0;
        }
        else {
            //�ļ���β
            EOF_flag = true;
            return EOF;
        }
    }
    //������һ���ַ�
    return g_lineBuf[g_forward++];
}





