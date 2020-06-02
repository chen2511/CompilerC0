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

//#define DEBUG_SWITCH
#ifdef  DEBUG_SWITCH
#define INFO(format, ...)           printf(format, ##__VA_ARGS__)
#define INFO_ASM(format, ...)       fprintf(ASM_FILE, format, ##__VA_ARGS__)
#else
#define INFO(info)              /* do nothing */
#define INFO_ASM(format, ...)    /* do nothing */
#endif

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
    ASSIGN, COMMA, SEMICOLON,                   //31-33: =  ,  ;
    LBRACE, RBRACE, LBRACKET, RBRACKET,         //34-37:{ } [ ]
    LPARENTHES, RPARENTHES,                     //38-39:( )      
    CALL, ARRAYAT                               // 在表达式中额外的操作：函数调用、数组，但不会在语法分析阶段出现
}TokenType;

//Token:枚举类型TokenType、值string value
typedef struct {
    TokenType   opType;
    char value[4096];
}_tToken;

//源文件
extern FILE* sourceFile;
extern FILE* AST_File;
extern FILE* IR_FILE;
extern FILE* ASM_FILE;
//行号
extern int g_lineNumber;
//在buf中的指针；
extern int g_lexBegin;
//forward 指向下一个将要读取的字符；lexBegin表示该token开始的位置
extern int g_forward;

//token
extern _tToken g_token;
//是否到达文件末尾
extern bool EOF_flag;

/////////////////////////// FOR  AST
#define MAX_TREENODE_CHILD_NUM  3                       // 最大孩子节点数
#define MAX_PARAMETERE_NUM      8                       // 最大参数个数

typedef enum {
    DecK, StmtK, ExpK, BoolExpK                         //声明，语句，表达式，布尔表达式
}NodeKind;

typedef enum {
    Const_DecK, Var_DecK, Func_DecK, MainFunc_DecK,         //常量说明，变量说明， 非主函数说明， 主函数说明
    Const_DefK, Var_DefK                                    //常量定义，变量定义
}DecKind;

typedef enum {
    If_StmtK, While_StmtK, For_StmtK, Assign_StmtK,         //条件， 循环， 赋值 语句
    Call_StmtK, Seq_StmtK, Read_StmtK, Write_StmtK,         //调用， 语句列， 读，写语句
    Ret_StmtK,                                              //返回语句
    // 为了便于遍历AST， 增加节点类型：即语句每个成分都有一个类型
    Write_StmtK_Str,
    Read_StmtK_Idlist

}StmtKind;



typedef enum {
    Op_ExpK, Num_ExpK, Iden_ExpK                           //操作类型 ， 数字（字母常量按数字保存）， 标识符
}ExpKind;

typedef enum {
    Op_BoolEK,                                              //布尔表达式操作：与或非； 
    Const_BoolEK, ConOp_BoolEK                              //布尔常量， 条件运算符

    //ConFac_BoolEK                                           // 条件因子， 其实就是简化了一点的表达式
    // 条件因子 用的是表达式节点，与表达式构造方法相同， 但是在另一个过程里实现（语法定义中不是表达式，但实际是）
}BoolExpKind;

// 类型信息： 可以是定义时 保存的类型信息；也可以用于 检验表达式中类型是否匹配
typedef enum {
    T_VOID, T_INTEGER, T_CHAR
}Type;

// 函数信息：返回类型和参数表；也可以链接到符号表中
typedef struct {
    Type rettype;
    struct {
        Type ptype;
        char* pname;
    }paratable[MAX_PARAMETERE_NUM];
    int paranum;
}FuncInfo;

// AST 的节点：左孩子右兄弟的树形结构；但表达式部分 又是二叉树结构
typedef struct TreeNode {
    struct TreeNode* child[MAX_TREENODE_CHILD_NUM];     // 左孩子，最多三个，通常只有一个，特定语句有多个
    struct TreeNode* sibling;                           // 右兄弟
    int lineno;                                     // 错误报告行号
    
    NodeKind nodekind;                              // 节点类型
    union {
        DecKind dec;
        StmtKind stmt;
        ExpKind exp;
        BoolExpKind bexp;
    }kind;                                          // 节点具体类型

    union {
        TokenType op;                                   // 操作类型：通常是表达式中
        int val;                                        // NUM的值：exp中 char也是存这个
        unsigned char cval;                             // Char 型 值：常量定义
        char* name;                                     // Id 的值，也可以是函数名，Str的值
        bool bval;                                      // bool 常量
        char* str;                                      // String 类型
    }attr;                                          // 节点属性
    int vec;                                        // 变量定义阶段设置：数组长度，不是数组就是-1 ； 
    Type type;                                      // 常、变量定义 ，类型说明 和 表达式类型检查
                                                    // 函数信息：返回类型和参数表；也可以链接到符号表中
    FuncInfo* pfinfo;                               // 函数定义阶段设置：函数信息； 或者是函数调用阶段的参数表

    // 转化成IR中用到
    char* place;
    int TC, FC;

}TreeNode;


/////////////////// 语义分析
#define SYMBOL_TABLE_SIZE 211

typedef enum IDType {
    Const_ID, Var_ID, Para_ID, Func_ID
}IDType;

// 符号表中的每一项
typedef struct Symbol {
    char* name;					// 标识符名字
    IDType type;				// ID类型： const，var，para，function
    Type valueType;				// ID的类型值：常变量的类型、参数类型、函数返回类型
    int value;					// 常量定义值						只有常数定义才会传入
    int adress;					// 内存地址
    int vec;					// 数组大小，不是数组为-1；			只有定义数组时，才会传入
    FuncInfo* pfinfo;			// 函数信息，AST中已有，拷贝即可；  只有函数定义是，才会传进，否则NULL
    struct Symbol* next;		// 有相同hash值时，下一条

    bool isreg;                 // isreg 用于生成目标代码阶段，记录是否保存在寄存器中；默认为false；与寄存器分配相关
    
}Symbol, * SymbolList;

typedef struct SymTab {
    SymTab* next;				// 多张表；指向下一张表
    char* fname;                // 表名 = 函数名
    SymbolList hashTable[SYMBOL_TABLE_SIZE];        // 哈希表
    int varsize;                // 函数局部变量和临时变量的空间大小
}SymTab;

extern SymTab* g_symtab;



////////////////////////// IR

#define MAX_QUADVAR_NUM 1000

typedef struct {
    char op[15];
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



#endif

