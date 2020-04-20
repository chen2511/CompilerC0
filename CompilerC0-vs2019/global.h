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
    ASSIGN, COLON, COMMA, SEMICOLON,            //31-34: =  :  ,  ;
    LBRACE, RBRACE, LBRACKET, RBRACKET,         //35-38:{ } [ ]
    LPARENTHES, RPARENTHES,                     //39-40:( )                                         
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
//token
extern Token token;
//�Ƿ񵽴��ļ�ĩβ
extern bool EOF_flag;


#endif

