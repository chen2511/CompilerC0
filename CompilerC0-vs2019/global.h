#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
#pragma warning(disable:4996)

#include <string>
#include <iostream>
#include <string.h>


#define RESERVEDWORD_NUM 14

#define PRINT_AST_TOFILE true

#define OPTIMIZE_SWITCH true
//#define OPTIMIZE_SWITCH false

//#define DEBUG_SWITCH
#ifdef  DEBUG_SWITCH
#define INFO(format, ...)           printf(format, ##__VA_ARGS__)
#define INFO_ASM(format, ...)       fprintf(ASM_FILE, format, ##__VA_ARGS__)
#else
#define INFO(info)              /* do nothing */
#define INFO_ASM(format, ...)    /* do nothing */
#endif

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
    CALL, ARRAYAT,                              // �ڱ��ʽ�ж���Ĳ������������á����飬���������﷨�����׶γ���
    END                                         // �ļ�����                                               
}TokenType;

//Token:ö������TokenType��ֵstring value
typedef struct {
    TokenType   opType;
    char value[4096];
}_tToken;

//Դ�ļ�
extern FILE* sourceFile;
extern FILE* AST_File;
extern FILE* IR_FILE;
extern FILE* ASM_FILE;

//�к�
extern int g_lineNumber;
//��buf�е�ָ�룻
extern int g_lexBegin;
//forward ָ����һ����Ҫ��ȡ���ַ���lexBegin��ʾ��token��ʼ��λ��
extern int g_forward;

extern int g_errorNum;

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
    Op_ExpK, Num_ExpK, Iden_ExpK                           //�������� �� ���֣���ĸ���������ֱ��棩�� ��ʶ��
}ExpKind;

typedef enum {
    Op_BoolEK,                                              //�������ʽ���������ǣ� 
    Const_BoolEK, ConOp_BoolEK                              //���������� ���������

    //ConFac_BoolEK                                           // �������ӣ� ��ʵ���Ǽ���һ��ı��ʽ
    // �������� �õ��Ǳ��ʽ�ڵ㣬����ʽ���췽����ͬ�� ��������һ��������ʵ�֣��﷨�����в��Ǳ��ʽ����ʵ���ǣ�
}BoolExpKind;

// ������Ϣ�� �����Ƕ���ʱ �����������Ϣ��Ҳ�������� ������ʽ�������Ƿ�ƥ��
typedef enum {
    T_VOID, T_INTEGER, T_CHAR, T_ERROR
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
        int val;                                        // NUM��ֵ��exp�� charҲ�Ǵ����
        unsigned char cval;                             // Char �� ֵ����������
        char* name;                                     // Id ��ֵ��Ҳ�����Ǻ�������Str��ֵ
        bool bval;                                      // bool ����
        char* str;                                      // String ����
    }attr;                                          // �ڵ�����
    int vec;                                        // ��������׶����ã����鳤�ȣ������������-1 �� 
    Type type;                                      // ������������ ������˵�� �� ���ʽ���ͼ��
                                                    // ������Ϣ���������ͺͲ�����Ҳ�������ӵ����ű���
    FuncInfo* pfinfo;                               // ��������׶����ã�������Ϣ�� �����Ǻ������ý׶εĲ�����

    // ת����IR���õ�
    char* place;
    int TC, FC;

    // ������
    bool error;

}TreeNode;


/////////////////// �������
#define SYMBOL_TABLE_SIZE 211

typedef enum IDType {
    Const_ID, Var_ID, Para_ID, Func_ID
}IDType;

// ���ű��е�ÿһ��
typedef struct Symbol {
    char* name;					// ��ʶ������
    IDType type;				// ID���ͣ� const��var��para��function
    Type valueType;				// ID������ֵ�������������͡��������͡�������������
    int value;					// ��������ֵ						ֻ�г�������Żᴫ��
    int adress;					// �ڴ��ַ
    int vec;					// �����С����������Ϊ-1��			ֻ�ж�������ʱ���Żᴫ��
    FuncInfo* pfinfo;			// ������Ϣ��AST�����У��������ɣ�  ֻ�к��������ǣ��Żᴫ��������NULL
    struct Symbol* next;		// ����ͬhashֵʱ����һ��

    bool isreg;                 // isreg ��������Ŀ�����׶Σ���¼�Ƿ񱣴��ڼĴ����У�Ĭ��Ϊfalse����Ĵ����������
    
}Symbol, * SymbolList;

typedef struct SymTab {
    SymTab* next;				// ���ű�ָ����һ�ű�
    char* fname;                // ���� = ������
    SymbolList hashTable[SYMBOL_TABLE_SIZE];        // ��ϣ��
    int varsize;                // �����ֲ���������ʱ�����Ŀռ��С
}SymTab;

extern SymTab* g_symtab;



////////////////////////// IR

#define MAX_QUADVAR_NUM 1000

typedef struct {
    char op[15];
    // �Ľ�5���ַ����Ľ�ö�٣����ܲ�
    char* var1;
    char* var2;
    char* var3;
}Quadvar;

extern Quadvar quadvarlist[MAX_QUADVAR_NUM];
extern int NXQ;

#define MAX_STRINGLIST_SIZE 100
extern char* stringlist[MAX_STRINGLIST_SIZE];
extern int str_index;
/////////// FOR genASM
typedef struct {
    int regindex;
    char* varname;
}RegInfo;


typedef enum {
    LACK_SEMI_CST,              // ��������û�зֺţ�������ǰ��ֱ���ٴ������������塢�������塢���
    LACK_TYPE_CST,              // û������
    LACK_ID_CST,                // û�б�ʶ��
    LACK_ASSIGN_CST,            // û�и�ֵ����

    LACK_XXX_VARDEF,            // �����������ֱ��������ǰ���

    LACK_TYPE_FUN,              // ��������δ˵��
    LACK_IDEN_FUN,              // ������δ˵��   �� 
    LACK_KUOHAO_FUN,            // �������Ŷ�ʧ   �� ������ǰ����

    SENTENCE_ERROR
}ErrorType;












#endif

