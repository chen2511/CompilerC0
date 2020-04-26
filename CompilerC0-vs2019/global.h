#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
#pragma warning(disable:4996)

#include <string>
#include <iostream>
#include <string.h>

#define RESERVEDWORD_NUM 14

#define PRINT_AST_TOFILE true

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
    CALL, ARRAYAT, NEGATIVE                     // �ڱ��ʽ�ж���Ĳ������������á����顢ȡ��ֵ
}TokenType;

//Token:ö������TokenType��ֵstring value
typedef struct {
    TokenType   opType;
    char value[4096];
}_tToken;

//Դ�ļ�
extern FILE* sourceFile;
extern FILE* AST_File;
//�к�
extern int g_lineNumber;
//��buf�е�ָ�룻
extern int g_lexBegin;
//forward ָ����һ����Ҫ��ȡ���ַ���lexBegin��ʾ��token��ʼ��λ��
extern int g_forward;

//token
extern _tToken g_token;
//�Ƿ񵽴��ļ�ĩβ
extern bool EOF_flag;

/////////////////////////// FOR  AST
#define MAX_TREENODE_CHILD_NUM  3                       // ����ӽڵ���
#define MAX_PARAMETERE_NUM      8                       // ����������

typedef enum {
    DecK, StmtK, ExpK, BoolExpK                         //��������䣬���ʽ���������ʽ
}NodeKind;

typedef enum {
    Const_DecK, Var_DecK, Func_DecK, MainFunc_DecK,         //����˵��������˵���� ��������˵���� ������˵��
    Const_DefK, Var_DefK                                    //�������壬��������
}DecKind;

typedef enum {
    If_StmtK, While_StmtK, For_StmtK, Assign_StmtK,         //������ ѭ���� ��ֵ ���
    Call_StmtK, Seq_StmtK, Read_StmtK, Write_StmtK,         //���ã� ����У� ����д���
    Ret_StmtK,                                              //�������
    // Ϊ�˱��ڱ���AST�� ���ӽڵ����ͣ������ÿ���ɷֶ���һ������
    Write_StmtK_Str,
    Read_StmtK_Idlist

}StmtKind;

typedef enum {
    Op_ExpK, Num_ExpK, Iden_ExpK, Letter_ExpK               //�������� �� ���֣� ��ʶ��
}ExpKind;

typedef enum {
    Op_BoolEK,                                              //�������ʽ���������ǣ� 
    Const_BoolEK, ConOp_BoolEK                              //���������� ���������

    //ConFac_BoolEK                                           // �������ӣ� ��ʵ���Ǽ���һ��ı��ʽ
    // �������� �õ��Ǳ��ʽ�ڵ㣬����ʽ���췽����ͬ�� ��������һ��������ʵ�֣��﷨�����в��Ǳ��ʽ����ʵ���ǣ�
}BoolExpKind;

// ������Ϣ�� �����Ƕ���ʱ �����������Ϣ��Ҳ�������� ������ʽ�������Ƿ�ƥ��
typedef enum {
    T_VOID, T_INTEGER, T_CHAR
}Type;

// ������Ϣ���������ͺͲ�����Ҳ�������ӵ����ű���
typedef struct {
    Type rettype;
    struct {
        Type ptype;
        char* pname;
    }paratable[MAX_PARAMETERE_NUM];
    int paranum;
}FuncInfo;

// AST �Ľڵ㣺�������ֵܵ����νṹ�������ʽ���� ���Ƕ������ṹ
typedef struct TreeNode {
    struct TreeNode* child[MAX_TREENODE_CHILD_NUM];     // ���ӣ����������ͨ��ֻ��һ�����ض�����ж��
    struct TreeNode* sibling;                           // ���ֵ�
    int lineno;                                     // ���󱨸��к�
    
    NodeKind nodekind;                              // �ڵ�����
    union {
        DecKind dec;
        StmtKind stmt;
        ExpKind exp;
        BoolExpKind bexp;
    }kind;                                          // �ڵ��������

    union {
        TokenType op;                                   // �������ͣ�ͨ���Ǳ��ʽ��
        int val;                                        // NUM��ֵ
        char cval;                                      // Char �� ֵ
        char* name;                                     // Id ��ֵ��Ҳ�����Ǻ�������Str��ֵ
        bool bval;                                      // bool ����
        char* str;                                      // String ����
    }attr;                                          // �ڵ�����
    int vec;                                        // ��������׶����ã����鳤�ȣ������������-1 �� 
    Type type;                                      // ������������ ������˵�� �� ���ʽ���ͼ��
                                                    // ������Ϣ���������ͺͲ�����Ҳ�������ӵ����ű���
    FuncInfo* pfinfo;                               // ��������׶����ã�������Ϣ�� �����Ǻ������ý׶εĲ�����

}TreeNode;



#endif

