#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <iostream>
#include <string.h>

#define RESERVEDWORD_NUM 14

//ö������TokenType��
typedef enum {
    //�ؼ��֣�
    CHAR, CONST, ELSE, FALSE, FOR,              //0-4
    IF, INT, MAIN, PRINTF, RETURN,              //5-9
    SCANF, TRUE, VOID, WHILE,                   //10-13
    NUM, IDEN, LETTER, STRING,                  //14-17:���֡���ʶ�����ַ����ַ���
    PLUS, MINU, MULT, DIV,                      //18-21:+ - * /
    AND, OR, NOT,                               //22-24:&& || !
    LSS, LEQ, GRE, GEQ, NEQ, EQL,               //25-30��ϵ�����: < <= > >= != ==
    ASSIGN, COMMA, SEMICOLON,                   //31-33: =  ,  ;
    LBRACE, RBRACE, LBRACKET, RBRACKET,         //34-37:{ } [ ]
    LPARENTHES, RPARENTHES,                     //38-39:( )                                         
}TokenType;

//Token:ö������TokenType��ֵstring value
typedef struct {
    TokenType   opType;
    char value[4096];
}Token;

//Դ�ļ�
extern FILE* sourceFile;
//�к�
extern int g_lineNumber;
//��buf�е�ָ�룻
extern int g_lexBegin;
//forward ָ����һ����Ҫ��ȡ���ַ���lexBegin��ʾ��token��ʼ��λ��
extern int g_forward;

//token
extern Token g_token;
//�Ƿ񵽴��ļ�ĩβ
extern bool EOF_flag;


#endif

