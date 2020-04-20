#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <iostream>
#include <string.h>

#define RESERVEDWORD_NUM 14

//枚举类型TokenType：
typedef enum {
    //关键字：
    CHAR, CONST, ELSE, FALSE, FOR,              //0-4
    IF, INT, MAIN, PRINTF, RETURN,              //5-9
    SCANF, TRUE, VOID, WHILE,                   //10-13
    NUM, IDEN, LETTER, STRING,                  //14-17:数字、标识符、字符、字符串
    PLUS, MINU, MULT, DIV,                      //18-21:+ - * /
    AND, OR, NOT,                               //22-24:&& || !
    LSS, LEQ, GRE, GEQ, NEQ, EQL,               //25-30关系运算符: < <= > >= != ==
    ASSIGN, COLON, COMMA, SEMICOLON,            //31-34: =  :  ,  ;
    LBRACE, RBRACE, LBRACKET, RBRACKET,         //35-38:{ } [ ]
    LPARENTHES, RPARENTHES,                     //39-40:( )                                         
}TokenType;

//Token:枚举类型TokenType、值string value
typedef struct {
    TokenType   opType;
    char value[4096];
}Token;

//源文件
extern FILE* sourceFile;
//行号
extern int g_lineNumber;
//token
extern Token token;
//是否到达文件末尾
extern bool EOF_flag;


#endif

