#pragma once
#ifndef SYMTAB_H
#define SYMTAB_H

#include "global.h"

// ��ʼ��һ�� ����ÿ������һ�ű�ȫ��һ�ű�
SymTab* initSimpleSymTable(char* name);

// �����Ƿ����ɹ�����ռ䲻�㡢�ظ�����
bool insert_SymTab(bool isGlobal, int lineno, char* name, IDType type, Type valuetype, int value, int vec = -1, FuncInfo* p = NULL);

// ���ط��Žڵ�ָ�롢δ���巵�ؿ�ָ�룻
Symbol* lookUp_SymTab(char* name);

// Ŀ���������ʱʹ�ã��Ƿ�ȫ�ֱ���
Symbol* lookUp_SymTab(char* name, bool& isGlobal);

// �������


#endif // !SYMTAB_H