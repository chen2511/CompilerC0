#pragma once
#ifndef SYMTAB_H
#define SYMTAB_H

#include "global.h"

// 初始化一张 单表；每个函数一张表、全局一张表
SymTab* initSimpleSymTable(char* name);

// 返回是否插入成功，如空间不足、重复定义
bool insert_SymTab(bool isGlobal, int lineno, char* name, IDType type, Type valuetype, int value, int vec = -1, FuncInfo* p = NULL);

// 返回符号节点指针、未定义返回空指针；
Symbol* lookUp_SymTab(char* name);

// 目标代码生成时使用，是否全局变量
Symbol* lookUp_SymTab(char* name, bool& isGlobal);

// 语义分析


#endif // !SYMTAB_H