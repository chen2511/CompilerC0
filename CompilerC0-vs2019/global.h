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
    ASSIGN, COMMA, SEMICOLON,                   //31-33: =  ,  ;
    LBRACE, RBRACE, LBRACKET, RBRACKET,         //34-37:{ } [ ]
    LPARENTHES, RPARENTHES,                     //38-39:( )      
    CALL, ARRAYAT, NEGATIVE                     // 在表达式中额外的操作：函数调用、数组、取负值
}TokenType;

//Token:枚举类型TokenType、值string value
typedef struct {
    TokenType   opType;
    char value[4096];
}_tToken;

//源文件
extern FILE* sourceFile;
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
    Ret_StmtK                                              //返回语句
}StmtKind;

typedef enum {
    Op_ExpK, Num_ExpK, Iden_ExpK, Letter_ExpK               //操作类型 ， 数字， 标识符
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
        int val;                                        // NUM的值
        char cval;                                      // Char 型 值
        char* name;                                     // Id 的值
        bool bval;                                      // bool 常量
        char* str;                                      // String 类型
    }attr;                                          // 节点属性
    int vec;                                        // 变量定义阶段设置：数组长度，不是数组就是-1 ； 
    Type type;                                      // 常、变量定义 ，类型说明 和 表达式类型检查
                                                    // 函数信息：返回类型和参数表；也可以链接到符号表中
    FuncInfo* pfinfo;                               // 函数定义阶段设置：函数信息； 或者是函数调用阶段的参数表

}TreeNode;



#endif

