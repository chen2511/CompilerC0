#pragma once
#ifndef AST_H
#define AST_H

#include "global.h"

// 创建声明节点：包括常量、变量、函数等等说明定义，传入具体声明类型，返回节点指针
TreeNode* newDecNode(DecKind kind);

// 创建语句节点：传入具体声明类型，返回节点指针
TreeNode* newStmtNode(StmtKind kind);

// 创建表达式节点：传入具体声明类型，返回节点指针
TreeNode* newExpNode(ExpKind kind);

// 创建布尔表达式节点：传入具体声明类型，返回节点指针
TreeNode* newBoolExpNode(BoolExpKind kind);

// 定义函数时，保存函数信息：返回类型和参数表
// FuncInfo* newFuncInfo(Type t);

// 拷贝token中的字符串到动态分配的空间
char* copyString(char* s);

// 输出抽象语法树到文件
void printAST(TreeNode* tree);

#endif // !AST_H


